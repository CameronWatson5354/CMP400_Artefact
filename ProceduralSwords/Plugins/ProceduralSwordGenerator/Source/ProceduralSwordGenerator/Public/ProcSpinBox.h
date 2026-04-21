// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SpinBox.h"
#include "ProcSpinBox.generated.h"

/**
 * 
 */
UCLASS()
class PROCEDURALSWORDGENERATOR_API UProcSpinBox : public USpinBox
{
	GENERATED_BODY()

public:
	void SetValueNoNotify(float InValue);
	
};
