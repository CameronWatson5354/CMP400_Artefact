// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcAttributeSlider.h"

#include "ProcSpinBox.h"
#include "Components/SpinBox.h"

void UProcAttributeSlider::NativeConstruct()
{
	Super::NativeConstruct();

	AttributeSlider->OnValueChanged.AddDynamic(this, &UProcAttributeSlider::SliderChangeCallback);
	AttributeSlider->OnValueCommitted.AddDynamic(this, &UProcAttributeSlider::CommitChangeCallback);
}

USpinBox* UProcAttributeSlider::GetAttributeSlider()
{
	return AttributeSlider;
}

UTextBlock* UProcAttributeSlider::GetTitleText()
{
	return TitleText;
}

void UProcAttributeSlider::SetAttributeFloatValue(float* NewAttributeFloatValue)
{
	AttributeFloatValue = NewAttributeFloatValue;

	ValueType = EValueType::Float;
}

void UProcAttributeSlider::SetAttributeIntValue(int* NewAttributeIntValue)
{
	AttributeIntValue = NewAttributeIntValue;

	ValueType = EValueType::Int;
}

void UProcAttributeSlider::UpdateValue(bool bSliderNotify)
{
	bNotifChanges = bSliderNotify;


	switch (ValueType)
	{
	case EValueType::Float:
		AttributeSlider->SetValue(*AttributeFloatValue);

		break;

	case EValueType::Int:
		AttributeSlider->SetValue(*AttributeIntValue);
		break;
	}

	bNotifChanges = NullOpt;
}

void UProcAttributeSlider::SliderChangeCallback(float InValue)
{
	switch (ValueType)
	{
	case EValueType::Float:
		*AttributeFloatValue = InValue;
		break;

	case EValueType::Int:
		*AttributeIntValue = InValue;
		break;
	}

	if (!bNotifChanges.IsSet() || (bNotifChanges.IsSet() && bNotifChanges.GetValue() == true))
	{
		OnAnyValueChanged.Broadcast();
	}
}

void UProcAttributeSlider::CommitChangeCallback(float InValue, ETextCommit::Type CommitMethod)
{
	SliderChangeCallback(InValue);
}
