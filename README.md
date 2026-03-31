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

## 2. 대화 & 퀘스트 시스템
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
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/5826111d01e6d65a40560dac376286c8e7cde1cf/Source/AZEditor/Public/SAZQuestEditorPanel.h#L3-L74
    https://github.com/SeongJeGyeong/Uneral_AshZero/blob/661ee93c884fcae890ce007e20e912154c798a3f/Source/AZEditor/Private/SAZQuestEditorPanel.cpp#L1-L511
  </details>
</details>

## 3. 멀티플레이 동기화
<details>
<summary><b>보기</b></summary>
</details>

## 4. 시네마틱 & 로딩 스크린
<details>
<summary><b>보기</b></summary>
</details>

## 5. 최적화
<details>
<summary><b>보기</b></summary>
</details>
