# 소개
Kog 산학 협력을 통해 Return Alive 리소스를 활용한 언리얼 엔진 게임 프로젝트입니다.

+ 개발 환경 : UE 5.5, SVN, Jira
+ 플랫폼 : PC
+ 개발 인원 : 프로그래밍 3명, 기획 5명, 아트 6명

# 담당 업무

## 1. 랜덤 맵 생성 시스템
<details>
  <summary><b>보기</b></summary>
  <details>
    <summary><b>맵 생성기</b></summary>
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/014f91ee6dc12390553a37b7e792901cab6f8436/Source/AshZero/Levels/AZRandomMapGenerator.h#L1-L307
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/014f91ee6dc12390553a37b7e792901cab6f8436/Source/AshZero/Levels/AZRandomMapGenerator.cpp#L1-L953
  </details>
  <details>
    <summary><b>호스트 맵 생성 시작 함수</b></summary>
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/014f91ee6dc12390553a37b7e792901cab6f8436/Source/AshZero/System/Player/AZPlayerController.cpp#L662-L676
  </details>
  <details>
    <summary><b>룸 액터</b></summary>
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/014f91ee6dc12390553a37b7e792901cab6f8436/Source/AshZero/Levels/Rooms/AZBaseRoom.h#L3-L98
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/014f91ee6dc12390553a37b7e792901cab6f8436/Source/AshZero/Levels/Rooms/AZBaseRoom.cpp#L4-L295
  </details>
  <details>
    <summary><b>에셋 액션 유틸리티</b></summary>
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/014f91ee6dc12390553a37b7e792901cab6f8436/Source/AZEditor/Public/AZRoomDataActionUtility.h#L3-L23
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/014f91ee6dc12390553a37b7e792901cab6f8436/Source/AZEditor/Private/AZRoomDataActionUtility.cpp#L4-L126
  </details>
</details>

## 2. 퀘스트 시스템
<img width="640" height="480" alt="Image" src="https://github.com/user-attachments/assets/e76f9b30-ab9e-434c-b70d-310e9ef6cd11" />
<details>
  <summary><b>보기</b></summary>
  <details>
    <summary><b>퀘스트 매니저</b></summary>
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/5826111d01e6d65a40560dac376286c8e7cde1cf/Source/AshZero/System/Subsystems/AZQuestManagerSubsystem.h#L3-L50
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/5826111d01e6d65a40560dac376286c8e7cde1cf/Source/AshZero/System/Subsystems/AZQuestManagerSubsystem.cpp#L4-L212
  </details>
  <details>
    <summary><b>퀘스트 목표 컴포넌트</b></summary>
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/5826111d01e6d65a40560dac376286c8e7cde1cf/Source/AshZero/Components/AZQuestObjectiveComponent.h#L3-L37
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/5826111d01e6d65a40560dac376286c8e7cde1cf/Source/AshZero/Components/AZQuestObjectiveComponent.cpp#L3-L60
  </details>
  <details>
    <summary><b>퀘스트 에디터</b></summary>
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/03cf4da206d2cd4ca01c40e0fdd19ca2b701a49a/Source/AZEditor/Public/SAZQuestEditorPanel.h#L3-L83
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/03cf4da206d2cd4ca01c40e0fdd19ca2b701a49a/Source/AZEditor/Private/SAZQuestEditorPanel.cpp#L4-L448
  </details>
  <details>
    <summary><b>퀘스트 에디터용 오브젝트</b></summary>
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/03cf4da206d2cd4ca01c40e0fdd19ca2b701a49a/Source/AZEditor/Public/AZQuestCreationObject.h#L3-L69
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/03cf4da206d2cd4ca01c40e0fdd19ca2b701a49a/Source/AZEditor/Private/AZQuestCreationObject.cpp#L3-L53
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/03cf4da206d2cd4ca01c40e0fdd19ca2b701a49a/Source/AZEditor/Public/AZQuestTagEditorObject.h#L3-L19
  </details>
  <details>
    <summary><b>커스텀 에디터 모듈</b></summary>
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/03cf4da206d2cd4ca01c40e0fdd19ca2b701a49a/Source/AZEditor/Public/AZEditorModule.h#L1-L15
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/dd7b3f25f878c4d715f14399104183c8cfd45538/Source/AZEditor/Private/AZEditorModule.cpp#L1-L78
  </details>
</details>

## 3. 멀티플레이 동기화
<details>
  <summary><b>보기</b></summary>
    <details>
      <summary><b>세션 시스템</b></summary>
      https://github.com/SeongJeGyeong/Uneral_AshZero/blob/a93c409c47cfaf8bdbb14e5ce7b2ec6dc3e2dc3f/Source/AshZero/System/AZSessionSubsystem.h#L3-L85
      https://github.com/SeongJeGyeong/Uneral_AshZero/blob/a93c409c47cfaf8bdbb14e5ce7b2ec6dc3e2dc3f/Source/AshZero/System/AZSessionSubsystem.cpp#L4-L407
    </details>
    <details>
      <summary><b>세션 로비</b></summary>
      https://github.com/SeongJeGyeong/Uneral_AshZero/blob/a93c409c47cfaf8bdbb14e5ce7b2ec6dc3e2dc3f/Source/AshZero/UI/Level/Lobby/AZLobbyUI.h#L3-L40
      https://github.com/SeongJeGyeong/Uneral_AshZero/blob/a93c409c47cfaf8bdbb14e5ce7b2ec6dc3e2dc3f/Source/AshZero/UI/Level/Lobby/AZLobbyUI.cpp#L3-L59
      https://github.com/SeongJeGyeong/Uneral_AshZero/blob/a93c409c47cfaf8bdbb14e5ce7b2ec6dc3e2dc3f/Source/AshZero/UI/Level/Lobby/AZSessionItem.h#L3-L43
      https://github.com/SeongJeGyeong/Uneral_AshZero/blob/a93c409c47cfaf8bdbb14e5ce7b2ec6dc3e2dc3f/Source/AshZero/UI/Level/Lobby/AZSessionItem.cpp#L4-L37
    </details>
    <details>
      <summary><b>파티 창</b></summary>
      https://github.com/SeongJeGyeong/Uneral_AshZero/blob/a93c409c47cfaf8bdbb14e5ce7b2ec6dc3e2dc3f/Source/AshZero/UI/Level/Lobby/AZPartyUI.h#L3-L105
      https://github.com/SeongJeGyeong/Uneral_AshZero/blob/a93c409c47cfaf8bdbb14e5ce7b2ec6dc3e2dc3f/Source/AshZero/UI/Level/Lobby/AZPartyUI.cpp#L4-L175
    </details>
    <details>
      <summary><b>탈출 워프게이트</b></summary>
      https://github.com/SeongJeGyeong/Uneral_AshZero/blob/fbb084d505fd236c8d8e5e6e6d7a277dee8bfeee/Source/AshZero/Levels/Props/AZWarpGate.h#L3-L88
      https://github.com/SeongJeGyeong/Uneral_AshZero/blob/fbb084d505fd236c8d8e5e6e6d7a277dee8bfeee/Source/AshZero/Levels/Props/AZWarpGate.cpp#L4-L198
    </details>
      <details>
      <summary><b>플레이어 사망 시 관전 모드</b></summary>
      https://github.com/SeongJeGyeong/Uneral_AshZero/blob/fbb084d505fd236c8d8e5e6e6d7a277dee8bfeee/Source/AshZero/System/GameMode/AZStageGameMode.cpp#L65-L104
      </details>
</details>

## 4. 대화창 시스템
<details>
  <summary><b>보기</b></summary>
    <details>
      <summary><b>대화창 위젯</b></summary>
      https://github.com/SeongJeGyeong/Uneral_AshZero/blob/315a8dfd795b3e99dcf7c9aa8a73e018f053a109/Source/AshZero/UI/DialogBox/AZDialogBoxUI.h#L3-L107
      https://github.com/SeongJeGyeong/Uneral_AshZero/blob/a430ec693e1a8115e5d370c49b01dabc1e840bbc/Source/AshZero/UI/DialogBox/AZDialogBoxUI.cpp#L2-L435
    </details>
    <details>
      <summary><b>버튼 위젯</b></summary>
      https://github.com/SeongJeGyeong/Uneral_AshZero/blob/315a8dfd795b3e99dcf7c9aa8a73e018f053a109/Source/AshZero/UI/DialogBox/AZDialogButtonWidget.h#L3-L38
      https://github.com/SeongJeGyeong/Uneral_AshZero/blob/315a8dfd795b3e99dcf7c9aa8a73e018f053a109/Source/AshZero/UI/DialogBox/AZDialogButtonWidget.cpp#L4-L18
    </details>
    <details>
      <summary><b>대화 시스템</b></summary>
      https://github.com/SeongJeGyeong/Uneral_AshZero/blob/a430ec693e1a8115e5d370c49b01dabc1e840bbc/Source/AshZero/System/Subsystems/AZDialogSubsystem.h#L3-L35
      https://github.com/SeongJeGyeong/Uneral_AshZero/blob/a430ec693e1a8115e5d370c49b01dabc1e840bbc/Source/AshZero/System/Subsystems/AZDialogSubsystem.cpp#L4-L76
    </details>
    <details>
      <summary><b>대화 UI 컴포넌트</b></summary>
      https://github.com/SeongJeGyeong/Uneral_AshZero/blob/c5dcec835124ee1bd861ec5418650b7e169f4b9f/Source/AshZero/System/Player/Components/AZDialogUIComponent.h#L3-L40
      https://github.com/SeongJeGyeong/Uneral_AshZero/blob/c5dcec835124ee1bd861ec5418650b7e169f4b9f/Source/AshZero/System/Player/Components/AZDialogUIComponent.cpp#L4-L69
    </details>
</details>

## 5. 시네마틱 & 로딩 스크린
<details>
  <summary><b>보기</b></summary>
  <details>
    <summary><b>로딩 스크린</b></summary>
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/5d5e1b267b3386fd58a00e5d8fb6f31e0edbb04d/Source/AshZero/System/Subsystems/AZSceneSubsystem.cpp#L221-L271
  </details>
  <details>
    <summary><b>시퀀스 재생</b></summary>
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/5d5e1b267b3386fd58a00e5d8fb6f31e0edbb04d/Source/AshZero/System/Subsystems/AZSceneSubsystem.cpp#L74-L190
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/5d5e1b267b3386fd58a00e5d8fb6f31e0edbb04d/Source/AshZero/Levels/Props/AZBossTriggerComponent.cpp#L4-L131
  </details>
</details>
