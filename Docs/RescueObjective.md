# 시민 구조 프로토타입

기준: `origin/develop`의 `2e079ea` · 브랜치: `feature/RescueObjective` · UE 5.8 / 싱글플레이

시민 1명에게 접근해 기존 C++ `IWInteractable`로 즉시 구조한다. 최초 구조만 미션에 수락되어 `0/1 → 1/1`이 되고, 구조 완료는 결과 확정이나 맵 이동을 호출하지 않는다.

## 책임과 교체 경계

| 소유자 | 책임과 수명 |
| --- | --- |
| `AWRescueTarget` | 같은 월드의 플레이어·거리·미구조 여부를 다시 검사하고 즉시 구조를 요청한다. 도형과 SOS/OK는 임시 표현이다. |
| 대상이 소유한 `UWMissionObjectiveComponent` | 목표 ID, 등록된 시도 ID와 대상의 1회 완료 상태. 미션이 보고를 수락한 뒤에만 완료 상태를 바꾼다. |
| `FWMissionDefinition.RequiredObjectiveIds` | 변경하지 않는 필수 목표 목록. 각 ID에 정확히 하나의 컴포넌트가 배치되어야 한다. |
| `AWMissionGameMode` | 초기화 때 대상 등록, 현재 시도·실제 컴포넌트·목표 ID·중복·진행 상태 검증, 보고 수락과 집계. 구조의 거리 규칙은 소유하지 않는다. |
| `AWMissionGameState` | 현재 시도의 완료 ID, 요구 수량, 진행 상태와 확정 결과를 조회하는 원본. |
| `AWMissionLoopHUD` | 상태 조회, 상호작용 안내, 테스트용 결과 확정·즉시 복귀 요청 전달. 별도의 진행 수량을 소유하지 않는다. |

추가 목표는 자신의 동작을 검증한 뒤 같은 컴포넌트로 완료를 보고한다. GameMode는 구조·처치·진압 종류를 선택하는 중앙 분기를 갖지 않는다. 목표 등록은 시작 때 고정하며 동적 목표 추가는 이번 범위가 아니다. 서로 다른 Actor 계층을 묶는 새 부모나 빈 확장 프레임워크도 추가하지 않는다. 기존 `Mission/` 경로와 Runtime 모듈을 유지한다.

정식 화면에서도 GameState의 `GetCompletedObjectives`, `GetRequiredObjectives`, `AreRequiredObjectivesComplete`, `GetAttemptId`, `HasResult`와 GameMode의 `CanReturnToStation` / `TryReturnToStation`을 재사용할 수 있다. 시민 상태는 `IsRescued`로 조회한다. 테스트용 `CanFinishDevelopmentMission` / `TryFinishDevelopmentMission`은 실제 탈출을 대신하는 정식 API가 아니다.

## 테스트용 임시 UI와 콘텐츠

화면 첫 줄의 **[테스트용 임시 UI]**, 완료 화면의 **[테스트 T] 결과 확정**, 결과의 **[테스트 R] 소방서 즉시 복귀**는 시민 구조·중복 방지·현재 시도 집계·왕복만 확인한다. `AWMissionLoopHUD`의 표시와 버튼은 정식 UI 작업에서 교체한다. 시민 도형과 SOS/OK 텍스트는 정식 시민 모델·상태 피드백으로 교체한다. 코드 주석과 미션 맵 메타데이터에도 임시 용도를 기록한다.

구조 후에도 `InProgress`이며 조작과 추가 상호작용은 유지된다. 모든 등록 목표가 완료되었을 때만 별도 테스트 요청으로 성공 결과를 확정할 수 있다. 이 요청은 Shipping에서 거부한다. 결과 이후 기존 MissionLoop의 입력 차단·결과 전달·복귀 검증을 사용한다. 실제 탈출 지점·보스 조건·진입/귀환 전 시퀀스, 실패/재시작 확장, 정식 UI·전투·룬은 후속 기능이다.

## 사용자 직접 확인 순서

1. 소방서 플레이 창에서 미션 패널을 선택하고 출동 포탈을 사용한다. 상호작용 키는 실제 입력 에셋에서 HUD가 조회한다.
2. 시민 구조 `0/1`, 새 시도 ID를 확인한다. 구조 전 T나 R을 눌러도 결과·복귀가 시작되지 않아야 한다.
3. 앞쪽 SOS 시민에게 접근한다. 구조 안내가 나타날 때 상호작용하면 `1/1`, 구조 완료와 OK 표시로 바뀌어야 한다. 결과 화면으로 자동 전환되면 안 된다.
4. 같은 위치에서 상호작용을 다시 눌러도 `1/1`을 유지해야 한다.
5. `[테스트 T] 결과 확정`을 누르거나 클릭한다. 결과에서 `[테스트 R] 소방서 즉시 복귀`를 사용한다.
6. 패널 선택·출동을 다시 수행한다. 이전과 다른 시도 ID, 시민 SOS/미구조, `0/1`, 빈 현재 결과를 확인한다. 다시 구조할 수 있어야 한다.

새 출동은 새 월드·GameState·시민 컴포넌트를 생성한다. 목표 집합·시도 연결·구조 완료 상태는 맵 간 전달하지 않는다. 전달 서비스는 기존처럼 확정 결과 값만 보관하고 소방서가 한 번 소비하면 비운다.

## 검증 기록

- 시작 상태: 작업 폴더 깨끗함. `git fetch origin` 후 `2e079ea`와 `de3605e` 확인, `origin/develop`에서 새 브랜치 생성.
- 에디터 `ProjectWEditor Win64 Development` 빌드: 통과. 최종 소스 기준 `Saved/Logs/RescueObjectiveEditorBuild.log`.
- 콘텐츠 구성: 통과. 저장된 맵의 시민 1명·목표 ID·DataTable 정의 일치 확인, commandlet 종료 코드 0, 오류/경고 0. `Saved/RescueObjectiveConfiguredAssets.json`, `Saved/Logs/RescueObjectiveConfigure.log`. 기록용 Python API 이름을 수정한 뒤 최종 스크립트를 재실행했다.
- 사용자 직접 플레이: 창 1개 실행. Computer Use에서 ProjectW 창이 정확히 1개이고 목표 1/1·테스트 결과·복귀 버튼이 표시되는 것을 관찰했다. 사용자 확인 결과는 대기 중이며 추가 창이나 에이전트 조작을 실행하지 않는다. 전체 왕복·중복 입력·재출동 초기화의 수동 통과로 아직 기록하지 않는다.
- 기존 MissionLoop 자동화 소스는 삭제된 개발용 큐브를 시민 구조와 별도 결과 요청으로 변경했다. 새 테스트 종류를 추가하거나 자동화를 실행하지 않았다.
- 쿠킹·패키징·패키지 플레이는 실행하지 않았다. 실제 구조·전투·진압 한 판 완료 기준은 이번 브랜치의 왕복으로 충족되지 않는다.

콘텐츠 재생성은 필요한 경우에만 `Scripts/ConfigureRescueObjective.py`를 UE Python commandlet에서 실행한다. 미션 DataTable과 미션 맵만 갱신하며 기존 HUD·플레이어·컨트롤러 설정은 유지한다. 전체 초기 구성용 `ConfigureMissionLoop.py`는 명시적으로 임시 HUD를 설정하므로 교체 HUD 설정 후 일반 실행하지 않는다.
