# SkillBase: 첫 Bolt 실행 경로와 검증

검증일: 2026-11-21 · 브랜치: `feature/SkillBase` · 기준 엔진: UE 5.8.3 / Win64

이 변경은 싱글플레이의 **좌클릭 → 기본공격 슬롯 요청 → 조건 검증 → Ability 실행·커밋 → Bolt 발사 → 종료**를 연결한다. GAS를 도입하지 않는다. 특수공격은 슬롯 타입과 선택적 초기 설정만 마련하며, 실제 스킬·우클릭 입력·선택 UI는 후속 범위다.

## 실행과 상태 소유자

```text
IA_W_BasicAttack / LeftMouseButton / Started
→ AWPlayerCharacter::OnBasicAttackAction
→ UWSkillComponent::TryActivateSlot(BasicAttack, AimDirection)
→ UWAbility::TryActivate: 공통 조건과 자식 조건 검사
→ UWAbility_Bolt::OnActivate: 투사체 생성 준비
→ UWAbility::CommitAbility: 현재 조건 재검증과 쿨다운 확정
→ 투사체 FinishSpawning
→ UWAbility::EndAbility
```

| 담당 객체 | 책임 |
| --- | --- |
| `AWPlayerController` | 커서 광선과 캐릭터 높이 평면으로 조준 방향 계산, 입력 에셋 참조 |
| `AWPlayerCharacter` | Enhanced Input을 기본공격 슬롯 요청으로 전달, 입력 해제 시 수락받은 실행 ID 전달 |
| `UWSkillComponent` | 초기 슬롯 설정 검증, 캐릭터별 Ability 인스턴스 보유, Ability ID별 쿨다운, 요청 차단·전체 취소 |
| `UWAbility` | 준비·커밋·종료 상태, 실행 ID 검사, 공통 재검증, 중복 커밋·종료 방지, 객체에 바인딩한 타이머 정리 |
| `UWAbility_Bolt` | 투사체 설정 검증, 생성 준비 후 커밋, 한 발 발사와 즉시 정상 종료 |
| `AWBoltProjectile` | 확정된 피해량 보유, 시전자 무시, 이동·충돌·PointDamage, 충돌 또는 수명 만료 시 제거 |

클래스 기본값(CDO)은 정의로 사용하고 실행 상태는 컴포넌트가 소유한 인스턴스에 둔다. 태그는 분류 정보이며 실행은 자식 가상 함수로 연결한다. 컴포넌트에는 Bolt 종류를 선택하는 분기가 없다.

요청의 `Accepted`는 실행 시작이 수락됐다는 뜻이다. 이번 Bolt는 같은 호출 안에서 발사와 종료까지 완료하지만, 비동기 스킬의 최종 성공까지 의미하지 않는다. 종료는 `OnAbilityEnded`와 상태 조회로 확인한다. ID `0`은 무효이며, 오래된 실행 ID의 해제·커밋·종료·취소는 새 실행을 변경하지 않는다.

쿨다운은 커밋 시 시작한다. 반복 커밋은 기존 성공 상태를 반환하고 쿨다운을 다시 시작하지 않는다. 커밋 후 취소는 이미 시작된 쿨다운을 유지한다. 입력 `Completed`/`Canceled`, 캐릭터 `UnPossessed`, 컴포넌트 `EndPlay`, `CancelAllAbilities`에 정리 경로가 있다. 사망·미션 종료 담당 객체는 향후 `SetSkillsEnabled(false)`를 호출해 새 요청 차단과 현재 실행 취소를 연결할 수 있다.

이미 발사된 Bolt는 Ability 종료 후에도 독립적으로 이동한다. 충돌 또는 기본 2초 수명에 따라 제거되며 맵이 제거되면 월드와 함께 정리된다. 같은 맵 안의 미션 종료 처리와 사망 시스템은 이 브랜치에서 새로 구현하지 않는다.

현재는 고정 초기 장착만 제공한다. 장착 교체·해금 목록, 자원 소모, 룬, 채널링, 공통 비동기 작업 프레임워크는 구현하지 않는다. 마나 규칙은 기획에서 미정이다. C++ Bolt의 쿨다운 0.25초·피해량 10·속도 1000cm/s는 기존 초안의 임시 검증값이다.

## 충돌 채널 관리와 밀림 수정

C++ 이름은 [`WCollisionChannels.h`](../Source/ProjectW/Actors/Collision/WCollisionChannels.h), 채널 등록은 [`DefaultEngine.ini`](../Config/DefaultEngine.ini)의 `Engine.CollisionProfile`에서 함께 관리한다.

| 이름 | 엔진 슬롯 | 현재 사용 |
| --- | --- | --- |
| `PlayerCharacter` | `ECC_GameTraceChannel1` | 플레이어 캡슐과 스켈레탈 메시 |
| `SkillProjectile` | `ECC_GameTraceChannel2` | 현재 플레이어 Bolt의 충돌 구와 비충돌 시각 메시 |

플레이어와 Bolt는 양쪽 모두 서로를 `Ignore`한다. Bolt끼리도 무시한다. Bolt는 `Pawn`, `WorldStatic`, `WorldDynamic`을 계속 차단하므로 적·월드 명중은 유지된다. 새 채널의 기본 응답은 `Block`으로 두어 기존 월드 오브젝트가 투사체를 통과시키지 않게 한다. 채널 번호는 저장된 에셋의 의미이므로 다른 용도로 재사용하지 않는다. 후속 적 투사체에는 피격 정책에 맞는 별도 응답을 설계해야 한다.

기존 에셋 감사에서 Bolt의 시각 메시는 이미 `NoCollision`이었다. 실제 차단 겹침은 `WorldDynamic`인 Bolt 충돌 구와 `Pawn`인 플레이어 캡슐 사이에 있었다. 투사체 이동에서 시전자를 무시하는 것만으로는 플레이어 쪽의 충돌 질의를 막지 못했다. 수정 전 테스트는 네 방향 모두 이 차단 겹침을 검출했고, 채널 분리 후 같은 검사를 통과했다.

## 검증한 에셋 연결

| 에셋 | 확인한 연결 |
| --- | --- |
| `BP_PlayerCharacter` | 네이티브 SkillComponent의 기본공격에 `AS_Test_Bolt` 장착 |
| `BP_WPlayerController` | `IA_W_BasicAttack`과 `IMC_MainIMC` 참조 |
| `IMC_MainIMC` | 좌클릭 → 기본공격 액션 매핑 정확히 1개 |
| `IA_W_BasicAttack` | Boolean 액션 |
| `AS_Test_Bolt` | `UWAbility_Bolt` 파생, 실행 ID 이름 `Test_Bolt`, `BP_Test_Bolt` 선택, 투사체 태그 |
| `BP_Test_Bolt` | `AWBoltProjectile` 파생, 전용 충돌 채널, 시각 메시 `NoCollision`, 명중·수명 종료 |

위 uasset 6개는 작업 시작 시 있던 변경이다. 실제 로딩·스폰·충돌 검사를 통과한 뒤 포함했으며, 작업 시작 스냅샷과 파일 해시가 같아 기존 바이트를 보존했다. `DefaultEngine.ini`의 기존 시작 맵 변경은 사용자 요청에 따라 커밋에서 제외했다. 충돌 채널 등록만 별도 hunk로 커밋했다.

## 검증 결과

| 검증 | 결과와 범위 |
| --- | --- |
| `ProjectWEditor Win64 Development` 빌드 | 통과, MSVC 14.44 / Windows SDK 10.0.22621.0 |
| `ProjectW Win64 Development` 빌드 | 통과, 런타임 타깃 컴파일·링크 |
| `ProjectW.Skills.AssetWiring` | 실제 BP·입력 연결, C++/설정 채널 일치, 발사 직후 BP 캐릭터 위치 유지, BP 명중·수명 종료 |
| `ProjectW.Skills.BoltDoesNotPushPlayer` | 네 방향의 플레이어 캡슐 차단 겹침 없음, 작은 이동 스윕에서 과도한 위치 보정 없음 |
| `ProjectW.Skills.BoltProjectile` | 한 요청당 한 발, 쿨다운, 시전자 무시, WorldDynamic·Pawn·WorldStatic 명중, 피해 1회, 빗나감 수명, 잘못된 설정 거부 |
| `ProjectW.Skills.CommitRevalidationAndCleanup` | 준비 후 조건 변경 거부, 취소 타이머 정리, 실행 차단, 소유자 제거와 종료 알림 중 재진입 정리 |
| `ProjectW.Skills.InstanceIsolationAndDefinitions` | 캐릭터별 상태·쿨다운 분리, CDO 보존, 슬롯 불일치·중복 ID 거부 |
| `ProjectW.Skills.LifecycleAndStaleExecution` | 잘못된 조준·빈 슬롯·중첩 실행 거부, 재커밋, 중복 종료, 오래된 콜백 무시, 일치하는 입력 해제로 종료 |
| 수정 후 실제 단독 플레이 | `L_Prototype_FireStation`에서 좌클릭 수락 6회 = 발사 6회 = 정상 종료 6회, 쿨다운 거부 3회 |

자동화 최종 보고서는 **성공 6 / 실패 0 / 미실행 0**이다. 단독 플레이는 UE 에디터 실행 파일의 `-game` 모드에서 수행했고 정상 종료 로그를 확인했다. 화면 조작은 사용자의 Escape 중지 요청 이후 재개하지 않았다. 수정 후 밀림 방지는 엔진 충돌 질의와 실제 Blueprint 위치 검사로 확인했으며, 화면 조작 중지 이후 별도의 육안 판정을 추가했다고 간주하지 않는다.

쿠킹·패키징 및 패키징된 게임의 플레이는 이번 첫 스킬 경로 검증에 포함하지 않았다. 소방서·미션 왕복, 전투 체력·사망, 특수공격 완성도나 로드맵 M3 전체 완료를 주장하지 않는다.

로컬 상세 증거는 Git에서 제외된 다음 파일에 남긴다. 검토용 요약은 [`SkillBaseValidation.json`](SkillBaseValidation.json)에 보관한다.

- `Saved/Logs/SkillBaseBaseline20261121.log`: 최종 에디터 빌드
- `Saved/Logs/SkillBaseGameBuild.log`: 최종 게임 빌드
- `Saved/SkillBaseAutomation/index.json`: 최종 6개 테스트 결과
- `Saved/SkillCollisionBefore/index.json`: 수정 전 차단 겹침 검출
- `Saved/Logs/SkillCollisionAudit.log`: 기존 Blueprint 충돌 설정 감사
- `Saved/Logs/SkillBasePlayCollision.log`: 수정 후 실제 입력·발사·종료
- `Saved/SkillBaseBaseline20261121/`: 작업 시작 당시 staged/unstaged 패치와 변경 파일 사본

## 재현

저장소 루트의 PowerShell에서 실행한다. 설치 위치가 다르면 엔진 경로를 바꾼다.

```powershell
$taskEngine = 'C:\Program Files\Epic Games\UE_5.8'
$taskProject = Join-Path (Get-Location) 'ProjectW.uproject'
& "$taskEngine\Engine\Build\BatchFiles\Build.bat" ProjectWEditor Win64 Development "-Project=$taskProject" -WaitMutex -NoHotReloadFromIDE -NoXGE -NoUBA
& "$taskEngine\Engine\Build\BatchFiles\Build.bat" ProjectW Win64 Development "-Project=$taskProject" -WaitMutex -NoHotReloadFromIDE -NoXGE -NoUBA
$taskReportPath = Join-Path (Get-Location) 'Saved\SkillBaseAutomation'
& "$taskEngine\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" $taskProject -Unattended -NullRHI -NoSound -NoSplash -NoLiveCoding '-ExecCmds=Automation RunTests ProjectW.Skills' '-TestExit=Automation Test Queue Empty' "-ReportExportPath=$taskReportPath"
$taskResult = Get-Content -Raw "$taskReportPath\index.json" | ConvertFrom-Json
if ($taskResult.succeeded -ne 6 -or $taskResult.failed -ne 0 -or $taskResult.notRun -ne 0) { throw 'Skill automation failed or incomplete' }
& "$taskEngine\Engine\Binaries\Win64\UnrealEditor.exe" $taskProject /Game/Level/L_Prototype_FireStation -game -windowed '-LogCmds=LogWSkills Verbose'
```

테스트 월드는 엔진과 같은 프레임 카운터 진행을 모사하고 종료 때 원래 값으로 복원한다. 테스트 프로세스의 종료 코드만으로 성공을 판단하지 않고, `index.json`의 집계와 각 테스트 결과를 확인한다.
