// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(AshZero, Log, All);

#define CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))

#define PRINT_CALLINFO( ) UE_LOG(AshZero, Warning, TEXT("%s"), *CALLINFO)

#define PRINT_LOG(fmt, ...) UE_LOG(AshZero, Warning, TEXT("%s %s"), *CALLINFO, \
*FString::Printf(fmt, ##__VA_ARGS__))

void PrintLogWithRole(const AActor* WorldContextObject, FString Text, FLinearColor TextColor, float Duration);
void PrintScreenLog(int32 Key, float TimeToDisplay, FColor DisplayColor, FString Text);