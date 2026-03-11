// Copyright Epic Games, Inc. All Rights Reserved.

#include "AshZero.h"
#include "Modules/ModuleManager.h"
#include "Kismet/GameplayStatics.h"

class FMyGameModule : public FDefaultGameModuleImpl
{
public:
	// 게임(에디터)이 켜질 때 딱 한 번 실행되는 함수
	virtual void StartupModule() override
	{
		FDefaultGameModuleImpl::StartupModule();
	}

	// 게임(에디터)이 꺼질 때 실행되는 함수
	virtual void ShutdownModule() override
	{
		FDefaultGameModuleImpl::ShutdownModule();
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FMyGameModule, AshZero, "AshZero" );
DEFINE_LOG_CATEGORY(AshZero);


void PrintLogWithRole(const AActor* WorldContextObject, FString Text, FLinearColor TextColor, float Duration)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (World)
	{
		if (World->WorldType == EWorldType::PIE)
		{
		}
		else
		{
			FString ModeMsg;
			switch (World->GetNetMode())
			{
			case NM_Standalone:
			{
				if (WorldContextObject->HasAuthority())
				{
					ModeMsg = TEXT("Server");
				}
				else
				{
					ModeMsg = TEXT("Client");
				}
			}break;
			case NM_DedicatedServer:
			case NM_ListenServer:
			{
				ModeMsg = TEXT("Server");
			}break;
			case NM_Client:
			{
				ModeMsg = TEXT("Client");
			}break;
			}
			Text.InsertAt(0, FString::Printf(TEXT("%s : "), *ModeMsg));

			GEngine->AddOnScreenDebugMessage(-1, Duration, TextColor.ToFColor(true), Text);
		}
	}
}

void PrintScreenLog(int32 Key, float TimeToDisplay, FColor DisplayColor, FString Text)
{
	GEngine->AddOnScreenDebugMessage(Key, TimeToDisplay, DisplayColor, Text);
}
