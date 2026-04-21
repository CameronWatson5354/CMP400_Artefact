// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcInGameGeneratorWidget.h"

#include "ProcGenAttributeWidget.h"
#include "ProcAttributeSlider.h"
#include "ProcAttributeTickBox.h"
#include "ProcSwordData.h"
#include "ProcSwordGeneratorB.h"
#include "ProcSwordInterface.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CheckBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Spacer.h"
#include "Components/SpinBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "UObject/UnrealTypePrivate.h"

void UProcInGameGeneratorWidget::SpawnGenerator()
{
	FVector SpawnOffset{GetOwningPlayerPawn()->GetActorForwardVector() * 200.0f};
	FVector SpawnLocation{GetOwningPlayerPawn()->GetActorLocation() + SpawnOffset};

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	SwordGenerator = GetWorld()->SpawnActor<AProcSwordGeneratorB>(GeneratorClass.LoadSynchronous(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);

	FAttachmentTransformRules TransformRules{EAttachmentRule::KeepWorld, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, true};
	SwordGenerator->AttachToActor(GetOwningPlayerPawn(), TransformRules);
	SwordGenerator->SetUseDefaultParams(false);

	UProcSwordData* SwordData{IProcSwordInterface::Execute_GetSwordData(GetOwningPlayerPawn())};
	if (IsValid(SwordData))
	{
		SwordGenerator->SetCurrentGenParams(SwordData->GetGenerationParameters());
	}
	else
	{
		SwordGenerator->ResetParameters();
	}
}

void UProcInGameGeneratorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SpawnGenerator();

	FindObjectsToGenerate(SwordGenerator.Get());

	OnGeneratorChanged();
}

void UProcInGameGeneratorWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (SwordGenerator.IsValid())
	{
		SwordGenerator->Destroy();
	}
}

void UProcInGameGeneratorWidget::GenerateSlidersRecursive(FProperty* Property, void* Container, UPanelWidget* ParentPanel)
{
	if (Property && VariablesToHide.Contains(Property->GetName()))
	{
		return;
	}
	
	UPanelWidget* NewParentPanel{ParentPanel};

	if (FStructProperty* StructProperty{CastField<FStructProperty>(Property)})
	{
		void* StructPtr{StructProperty->ContainerPtrToValuePtr<void>(Container)};
		UPanelWidget* ChildPanel{nullptr};

		//is sword blade params
		if (StructProperty->Struct->IsChildOf(FGeneralGenParams::StaticStruct()))
		{
			//make a widget that has the title of the struct on its own line
			UTextBlock* TitleText{NewObject<UTextBlock>(GetOwningPlayer())};
			TitleText->SetText(FText::FromString(StructProperty->GetName()));
			VerticalBoxList->AddChildToVerticalBox(TitleText)->SetPadding(FMargin{0.0f, 20.0f, 0.0f, 0.0f});
		}
		if (StructProperty->Struct->IsChildOf(FProcGenerationParameter::StaticStruct()))
		{
			//create parent attribute widget on new line
			UProcGenAttributeWidget* AttributeWidget{CreateWidget<UProcGenAttributeWidget>(GetOwningPlayer(), AttributeWidgetClass)};
			AttributeWidget->GetAttributeText()->SetText(FText::FromString(StructProperty->GetName()));
			AttributeWidget->SetPadding(FMargin{5.0f});
			VerticalBoxList->AddChildToVerticalBox(AttributeWidget);

			ChildPanel = AttributeWidget->GetAttributeListBox();
		}

		for (TFieldIterator<FProperty> StructIt{StructProperty->Struct}; StructIt; ++StructIt)
		{
			FProperty* PropertyB{*StructIt};
			GenerateSlidersRecursive(PropertyB, StructPtr, ChildPanel);
		}
	}
	else if (FFloatProperty* FloatProp{CastField<FFloatProperty>(Property)})
	{
		float* FloatValue{FloatProp->ContainerPtrToValuePtr<float>(Container)};

		if (!IsValid(ParentPanel))
		{
			//create parent attribute widget on new line
			UProcGenAttributeWidget* AttributeWidget{CreateWidget<UProcGenAttributeWidget>(GetOwningPlayer(), AttributeWidgetClass)};
			AttributeWidget->GetAttributeText()->SetText(FText::FromString(Property->GetName()));
			AttributeWidget->SetPadding(FMargin{5.0f});
			VerticalBoxList->AddChildToVerticalBox(AttributeWidget);

			NewParentPanel = AttributeWidget->GetAttributeListBox();
		}

		UProcAttributeSlider* Slider{CreateWidget<UProcAttributeSlider>(GetOwningPlayer(), SliderClass)};
		
		Slider->GetTitleText()->SetText(FText::FromString(FloatProp->GetName()));
		
		if (UStruct* OwnerStruct{Property->GetOwnerStruct()})
		{
			//hide text if not a param with 3 values
			if (OwnerStruct->GetName() != "ProcGenerationParameter")
			{
				Slider->GetTitleText()->SetVisibility(ESlateVisibility::Collapsed);
				Slider->GetAttributeSlider()->Font.Size = 12.0f;
			}
			
		}

		if (FloatProp->GetName() == "MainValue")
		{
			Slider->ModifyMainValueAppearance();
		}
		
		Slider->SetPadding(FMargin{5.0f, 0.0f});
		Slider->SetAttributeFloatValue(FloatValue);
		Slider->OnAnyValueChanged.AddDynamic(this, &UProcInGameGeneratorWidget::SliderChangeCallback);
		Slider->GetAttributeSlider()->SetValue(*FloatValue);
		UPanelSlot* PanelSlot{NewParentPanel->AddChild(Slider)};

		
		
		if (UHorizontalBoxSlot* HorizontalSlot{Cast<UHorizontalBoxSlot>(PanelSlot)})
		{
			HorizontalSlot->SetVerticalAlignment(VAlign_Center);
		}

		AttributeSliders.Add(Slider);
	}
	else if (FIntProperty* IntProp{CastField<FIntProperty>(Property)})
	{
		int* IntValue{IntProp->ContainerPtrToValuePtr<int>(Container)};

		//create parent attribute widget on new line
		UProcGenAttributeWidget* AttributeWidget{CreateWidget<UProcGenAttributeWidget>(GetOwningPlayer(), AttributeWidgetClass)};
		AttributeWidget->GetAttributeText()->SetText(FText::FromString(Property->GetName()));
		AttributeWidget->SetPadding(FMargin{5.0f});
		VerticalBoxList->AddChildToVerticalBox(AttributeWidget);

		UProcAttributeSlider* Slider{CreateWidget<UProcAttributeSlider>(GetOwningPlayer(), SliderClass)};
		Slider->GetTitleText()->SetText(FText::FromString(Property->GetName()));
		Slider->GetTitleText()->SetVisibility(ESlateVisibility::Collapsed);
		Slider->SetPadding(FMargin{5.0f, 0.0f});
		Slider->SetAttributeIntValue(IntValue);
		Slider->OnAnyValueChanged.AddDynamic(this, &UProcInGameGeneratorWidget::SliderChangeCallback);
		Slider->GetAttributeSlider()->SetValue(*IntValue);

		USpinBox* SpinBox{Slider->GetAttributeSlider()};
		SpinBox->SetMaxFractionalDigits(0);
		SpinBox->SetDelta(1.0f);


		AttributeWidget->GetAttributeListBox()->AddChildToHorizontalBox(Slider);
		AttributeSliders.Add(Slider);
	}
	//if value is a boolean
	else if (FBoolProperty* BoolProp{CastField<FBoolProperty>(Property)})
	{
		bool* BoolValue{BoolProp->ContainerPtrToValuePtr<bool>(Container)};

		//create parent attribute widget on new line
		UProcGenAttributeWidget* AttributeWidget{CreateWidget<UProcGenAttributeWidget>(GetOwningPlayer(), AttributeWidgetClass)};
		AttributeWidget->GetAttributeText()->SetText(FText::FromString(Property->GetName()));
		AttributeWidget->SetPadding(FMargin{5.0f});
		VerticalBoxList->AddChildToVerticalBox(AttributeWidget);

		//create tickbox
		UProcAttributeTickBox* TickBox{CreateWidget<UProcAttributeTickBox>(GetOwningPlayer(), TickBoxClass)};
		TickBox->SetAttributeBoolValue(BoolValue);
		TickBox->GetAttributeCheckBox()->OnCheckStateChanged.AddDynamic(this, &UProcInGameGeneratorWidget::CheckBoxChangeCallback);

		AttributeWidget->GetAttributeListBox()->AddChildToHorizontalBox(TickBox);
		AttributeTickBoxes.Add(TickBox);
	}
}

void UProcInGameGeneratorWidget::FindObjectsToGenerate(AProcSwordGeneratorB* Generator)
{
	for (TFieldIterator<FProperty> It(Generator->GetClass()); It; ++It)
	{
		FProperty* Property = *It;

		if (GeneratorVariableNames.Contains(Property->GetName()))
		{
			GenerateSlidersRecursive(Property, Generator);
		}
	}
}

void UProcInGameGeneratorWidget::UpdateGeneratorParams(const FProcCombinedGenParams& NewGenerationParameters)
{
	SwordGenerator->SetCurrentGenParams(NewGenerationParameters);
	FindObjectsToGenerate(SwordGenerator.Get());
	UpdateVariablesNextTick(true);
}

void UProcInGameGeneratorWidget::ResetSwordMesh()
{
	if (SwordGenerator.IsValid())
	{
		SwordGenerator->ResetParameters();
		FindObjectsToGenerate(SwordGenerator.Get());
		UpdateVariablesNextTick(true);
	}
}

void UProcInGameGeneratorWidget::TickFunction(bool bSliderNotify)
{
	SwordGenerator->UpdateMesh();

	for (auto& Slider : AttributeSliders)
	{
		Slider->UpdateValue(bSliderNotify);
	}

	for (auto& CheckBox : AttributeTickBoxes)
	{
		CheckBox->UpdateValue();
	}

	OnGeneratorChanged();
}

void UProcInGameGeneratorWidget::UpdateVariablesNextTick(bool bSliderNotify)
{
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &UProcInGameGeneratorWidget::TickFunction, bSliderNotify);
	
	GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegate);
}

void UProcInGameGeneratorWidget::SliderChangeCallback()
{
	UpdateVariablesNextTick(false);
}

void UProcInGameGeneratorWidget::CheckBoxChangeCallback(bool bIsChecked)
{
	UpdateVariablesNextTick(true);
}

void UProcInGameGeneratorWidget::SaveSword()
{
	UProcSwordData* SwordData{SwordGenerator->SaveSwordIntoData()};

	IProcSwordInterface::Execute_PickupSword(GetOwningPlayerPawn(), SwordData);
}
