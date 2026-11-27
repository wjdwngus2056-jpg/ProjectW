"""UE Python commandlet: 기존 프로토타입 콘텐츠에 미션 루프를 연결한다. 반복 실행 가능."""
import unreal
import json
from pathlib import Path

root = Path(unreal.Paths.project_dir())
editor = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
worlds = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
hud = unreal.load_class(None, '/Script/ProjectW.WMissionLoopHUD')
state = unreal.load_class(None, '/Script/ProjectW.WMissionGameState')
table = unreal.load_asset('/Game/Mission/Data/DT_MissionDefinition')
assert unreal.DataTableFunctionLibrary.fill_data_table_from_json_file(table, str(root / 'Content/Mission/Data/DT_MissionDefinition.json'))
unreal.EditorAssetLibrary.save_loaded_asset(table)

audit = {}
for domain in ['FireStation', 'Mission']:
    path = f'/Game/{domain}/Framework/BP_W{domain}GameMode'
    bp = unreal.load_asset(path)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    cls = unreal.load_class(None, path + f'.BP_W{domain}GameMode_C')
    assert not cls.get_path_name().startswith('/Engine/Transient')
    cdo = unreal.get_default_object(cls)
    bp.modify()
    cdo.set_editor_property('hud_class', hud)
    if domain == 'Mission':
        cdo.set_editor_property('game_state_class', state)
        cdo.set_editor_property('mission_data_table', table)
    unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
    map_path = f'/Game/Level/L_Prototype_{domain}'
    unreal.EditorLoadingAndSavingUtils.load_map(map_path)
    world = worlds.get_editor_world()
    # 기존 맵별 BP GameMode를 명시하여 글로벌 기본 GameMode에 의존하지 않는다.
    world.get_world_settings().set_editor_property('default_game_mode', cls)
    # 목표 배치는 최신 시민 구조 구성 스크립트가 담당한다.
    unreal.EditorLoadingAndSavingUtils.save_current_level()
    audit[domain] = {
        'game_mode': world.get_world_settings().get_editor_property('default_game_mode').get_path_name(),
        'game_state': cdo.get_editor_property('game_state_class').get_path_name(),
        'hud': cdo.get_editor_property('hud_class').get_path_name(),
        'actors': [{'name':a.get_name(), 'class':a.get_class().get_path_name(), 'location':str(a.get_actor_location())} for a in editor.get_all_level_actors()],
    }
with open(root / 'Saved/MissionLoopConfiguredAssets.json', 'w', encoding='utf-8') as f:
    json.dump(audit, f, ensure_ascii=False, indent=2)

# 초기 구성을 재생성할 때도 삭제된 개발용 큐브 대신 시민 1명을 배치한다.
exec(compile((root / 'Scripts/ConfigureRescueObjective.py').read_text(encoding='utf-8'),
             'ConfigureRescueObjective.py', 'exec'))
