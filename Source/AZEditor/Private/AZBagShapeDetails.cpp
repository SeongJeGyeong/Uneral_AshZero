// Fill out your copyright notice in the Description page of Project Settings.


#include "AZBagShapeDetails.h"
#include "DataAsset/AZBagShapeDataAsset.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/STextBlock.h"
#include "PropertyCustomizationHelpers.h"

TSharedRef<IPropertyTypeCustomization> AZBagShapeDetails::MakeInstance()
{
	return MakeShared<AZBagShapeDetails>();
}

void AZBagShapeDetails::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	HeaderRow
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			PropertyHandle->CreatePropertyValueWidget()
		];
}

void AZBagShapeDetails::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
    //구조체 내부의 멤버변수 가져오기
    TSharedPtr<IPropertyHandle> IDHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FBagDefinition, ID));
	TSharedPtr<IPropertyHandle> RowsHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FBagDefinition, Rows));
	TSharedPtr<IPropertyHandle> ColsHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FBagDefinition, Columns));
	TSharedPtr<IPropertyHandle> NameHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FBagDefinition, BagName));

    //화면에 그리기
    ChildBuilder.AddProperty(IDHandle.ToSharedRef());
	ChildBuilder.AddProperty(NameHandle.ToSharedRef());
	ChildBuilder.AddProperty(RowsHandle.ToSharedRef());
	ChildBuilder.AddProperty(ColsHandle.ToSharedRef());

    //구조체의 주소를 가져오기
	void* StructData = nullptr;
	PropertyHandle->GetValueData(StructData);
	FBagDefinition* BagDef = static_cast<FBagDefinition*>(StructData); //만든 struct 타입으로 변환 

	if (!BagDef) return;

    //Rows랑 Colmuns값이 바뀌면 실행할 람다
	auto OnSizeChanged = [BagDef]()
		{
			if (BagDef)
			{
                if (BagDef->Rows < 1) BagDef->Rows = 1;
                if (BagDef->Columns < 1) BagDef->Columns = 1;

				int32 NewSize = BagDef->Rows * BagDef->Columns;
				if (NewSize > 0 && BagDef->GridFlags.Num() != NewSize)
				{
					BagDef->GridFlags.SetNum(NewSize);
				}
			}
		};


    //핸들에 연결해서 에디터에서 숫자를 바꿀 때마다 자동으로 실행
	RowsHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda(OnSizeChanged));
	ColsHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda(OnSizeChanged));

    //격자 모양 컨테이너 생성
	TSharedRef<SUniformGridPanel> GridPanel = SNew(SUniformGridPanel).SlotPadding(2.0f);

    int32 ExpectedSize = BagDef->Rows * BagDef->Columns;
    if (BagDef->GridFlags.Num() != ExpectedSize)
    {
        BagDef->GridFlags.SetNum(ExpectedSize);
    }

    for (int32 Row = 0; Row < BagDef->Rows; ++Row)
    {
        for (int32 Col = 0; Col < BagDef->Columns; ++Col)
        {
            int32 Index = Row * BagDef->Columns + Col;

            if (!BagDef->GridFlags.IsValidIndex(Index)) continue;

            //슬롯 추가
            GridPanel->AddSlot(Col, Row)
                [
                    SNew(SCheckBox)
                        .IsChecked_Lambda([BagDef, Index]() -> ECheckBoxState   //체크박스가 체크되어 있는지
                            {
                                if (BagDef && BagDef->GridFlags.IsValidIndex(Index))
                                {
                                    return BagDef->GridFlags[Index] ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                                }
                                return ECheckBoxState::Undetermined;
                            })
                        .OnCheckStateChanged_Lambda([BagDef, Index, PropertyHandle](ECheckBoxState NewState)    //체크박스를 클릭했을 때 실행
                            {
                                if (BagDef && BagDef->GridFlags.IsValidIndex(Index))
                                {
                                    PropertyHandle->NotifyPreChange();
                                    BagDef->GridFlags[Index] = (NewState == ECheckBoxState::Checked);
                                    PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
                                }
                            })
                ];
        }
    }

    //
    ChildBuilder.AddCustomRow(FText::FromString("Grid"))
        .NameContent()
        [
            SNew(STextBlock)
                .Text(FText::FromString("Inventory Shape"))
                .Font(IDetailLayoutBuilder::GetDetailFont())
        ]
        .ValueContent()
        [
            GridPanel
        ];
}

//void AZBagShapeDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
//{
//	TArray<TWeakObjectPtr<UObject>> Objects;
//	DetailBuilder.GetObjectsBeingCustomized(Objects);
//
//	if (Objects.Num() == 0) return;
//	UBagShapeData* MyAsset = Cast<UBagShapeData>(Objects[0].Get());
//	if (!MyAsset) return;
//
//	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("BagShapeSettings");
//	TSharedRef<SUniformGridPanel> GridPanel = SNew(SUniformGridPanel).SlotPadding(2.0f);
//	
//	for (int32 Row = 0; Row < MyAsset->Rows; ++Row)
//	{
//		for (int32 Col = 0; Col < MyAsset->Columns; Col++)
//		{
//			int32 Index = Row * MyAsset->Columns + Col;
//			if (!MyAsset->GridFlags.IsValidIndex(Index)) continue;
//			
//			GridPanel->AddSlot(Col, Row)
//				[
//					SNew(SCheckBox)
//						.IsChecked_Lambda([MyAsset, Index]()->ECheckBoxState 
//						{
//							if (MyAsset && MyAsset->GridFlags.IsValidIndex(Index))
//							{
//								return MyAsset->GridFlags[Index] ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
//							}
//							return ECheckBoxState::Undetermined;
//						})
//						.OnCheckStateChanged_Lambda([MyAsset, Index](ECheckBoxState NewState) 
//						{
//							if (MyAsset && MyAsset->GridFlags.IsValidIndex(Index))
//							{
//								MyAsset->Modify();
//
//								bool bIsChecked = (NewState == ECheckBoxState::Checked);
//								MyAsset->GridFlags[Index] = bIsChecked;
//							}
//						})
//						.ToolTipText(FText::Format(FText::FromString("Row: {0}, Col: {1}"), Row, Col))
//				];
//
//		}
//	}
//
//	Category.AddCustomRow(FText::FromString("GridVisualizer"))
//		.NameContent()
//		[
//			SNew(STextBlock)
//				.Text(FText::FromString("Inventory Pattern"))
//				.Font(IDetailLayoutBuilder::GetDetailFont())
//		]
//		.ValueContent()
//		[
//			GridPanel
//		];
//}
