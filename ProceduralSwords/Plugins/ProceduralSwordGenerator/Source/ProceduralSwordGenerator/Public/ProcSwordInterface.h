// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ProcSwordInterface.generated.h"

class UProcSwordData;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UProcSwordInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROCEDURALSWORDGENERATOR_API IProcSwordInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PickupSword(UProcSwordData* NewSwordData);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UProcSwordData* GetSwordData();
};
