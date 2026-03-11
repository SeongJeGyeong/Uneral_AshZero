// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/AZUISoundDataAsset.h"

TObjectPtr<USoundBase> UAZUISoundDataAsset::GetUISFX(EUISFXType SelectSFXType)
{
	return UISFXList.FindRef(SelectSFXType);
}
