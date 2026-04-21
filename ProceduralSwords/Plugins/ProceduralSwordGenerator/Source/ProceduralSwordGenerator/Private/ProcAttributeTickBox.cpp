// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcAttributeTickBox.h"

#include "Components/CheckBox.h"

void UProcAttributeTickBox::NativeConstruct()
{
	Super::NativeConstruct();

	AttributeCheckBox->OnCheckStateChanged.AddDynamic(this, &UProcAttributeTickBox::CheckBoxChangeCallback);
}

UCheckBox* UProcAttributeTickBox::GetAttributeCheckBox()
{
	return AttributeCheckBox;
}

void UProcAttributeTickBox::SetAttributeBoolValue(bool* bNewAttributeBoolValue)
{
	bAttributeBoolValue = bNewAttributeBoolValue;

	AttributeCheckBox->SetIsChecked(*bAttributeBoolValue);
}

void UProcAttributeTickBox::UpdateValue()
{
	AttributeCheckBox->SetIsChecked(*bAttributeBoolValue);
}

void UProcAttributeTickBox::CheckBoxChangeCallback(bool bIsChecked)
{
	*bAttributeBoolValue = bIsChecked;
	
	OnTickboxValueChanged.Broadcast();
}
