"""시민 구조 프로토타입 콘텐츠 구성 전용. 테스트용 임시 표현이며 기존 HUD·입력 설정은 보존한다."""
import json
from pathlib import Path
import unreal

root = Path(unreal.Paths.project_dir())
editor = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
worlds = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
table = unreal.load_asset('/Game/Mission/Data/DT_MissionDefinition')
assert unreal.DataTableFunctionLibrary.fill_data_table_from_json_file(
    table, str(root / 'Content/Mission/Data/DT_MissionDefinition.json'))
assert unreal.EditorAssetLibrary.save_loaded_asset(table)

unreal.EditorLoadingAndSavingUtils.load_map('/Game/Level/L_Prototype_Mission')
target_class = unreal.load_class(None, '/Script/ProjectW.WRescueTarget')
objective_class = unreal.load_class(None, '/Script/ProjectW.WMissionObjectiveComponent')
assert target_class and objective_class
# 이전 왕복 전용 큐브가 남아 있으면 시민과 중복 배치되지 않게 교체한다.
for actor in editor.get_all_level_actors():
    if actor.get_actor_label() == 'DEV_CompleteMission':
        assert editor.destroy_actor(actor)
targets = [a for a in editor.get_all_level_actors() if a.get_class() == target_class]
assert len(targets) <= 1, '사용자 배치를 임의로 삭제하지 않도록 중복이면 중단'
target = targets[0] if targets else editor.spawn_actor_from_class(target_class, unreal.Vector(250, 0, 60))
target.set_actor_label('Prototype_RescueCitizen_TemporaryVisual')
target.get_component_by_class(objective_class).set_editor_property('objective_id', 'RescueCitizen')
world = worlds.get_editor_world()
unreal.EditorAssetLibrary.set_metadata_tag(world, 'PrototypePurpose',
    '테스트용 임시 시민 도형과 SOS/OK: 1회 구조·목표 보고 검증. 정식 시민 모델과 피드백으로 교체.')
assert unreal.EditorLoadingAndSavingUtils.save_current_level()
audit = {
    'target': target.get_path_name(),
    'location': str(target.get_actor_location()),
    'objective_id': str(target.get_component_by_class(objective_class).get_editor_property('objective_id')),
    'target_count': len([a for a in editor.get_all_level_actors() if a.get_class() == target_class]),
    'definition': unreal.DataTableFunctionLibrary.export_data_table_to_json_string(table),
    'game_mode': world.get_world_settings().get_editor_property('default_game_mode').get_path_name(),
}
with open(root / 'Saved/RescueObjectiveConfiguredAssets.json', 'w', encoding='utf-8') as file:
    json.dump(audit, file, ensure_ascii=False, indent=2)
