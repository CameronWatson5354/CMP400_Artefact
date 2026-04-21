// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcSpinBox.h"

void UProcSpinBox::SetValueNoNotify(float InValue)
{
	if (MySpinBox.IsValid())
	{
		MySpinBox->SetValue(InValue);
	}

}
