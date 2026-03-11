// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/AZBagShapeDataAsset.h"

//UBagShapeData::UBagShapeData()
//{
//	Rows = 5;
//	Columns = 5;
//
//	GridFlags.Init(false, Rows * Columns);
//}


//void UBagShapeData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
//{
//	Super::PostEditChangeProperty(PropertyChangedEvent);
//
//	// Rows나 Columns가 변경되었을 때 배열 크기 재조정
//	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
//	if (PropertyName == GET_MEMBER_NAME_CHECKED(UBagShapeData, Rows) ||
//		PropertyName == GET_MEMBER_NAME_CHECKED(UBagShapeData, Columns))
//	{
//		if (Rows < 1) Rows = 1;
//		if (Columns < 1) Columns = 1;
//
//		int32 TargetSize = Rows * Columns;
//		if (TargetSize < 0) TargetSize = 0;
//
//		// 크기가 다를 때만 조절 (기존 데이터 보존 노력)
//		if (GridFlags.Num() != TargetSize)
//		{
//			// 데이터가 날아가지 않게 보존하며 크기 조절 (줄어들면 뒤쪽 데이터는 삭제됨)
//			GridFlags.SetNum(TargetSize);
//		}
//	}
//
//}

//FBagDefinition UAZBagShapeDataAsset::FindBagDataByName(const FName& BagName) const
//{
//
//    return FBagDefinition();
//}
