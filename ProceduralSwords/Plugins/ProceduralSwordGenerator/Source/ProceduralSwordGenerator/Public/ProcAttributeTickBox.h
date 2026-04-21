// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProcAttributeTickBox.generated.h"

class UTextBlock;
class UCheckBox;
/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTickboxValueChanged);

UCLASS()
class PROCEDURALSWORDGENERATOR_API UProcAttributeTickBox : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	UCheckBox* GetAttributeCheckBox();

	void SetAttributeBoolValue(bool* bNewAttributeBoolValue);

	void UpdateValue();

	UFUNCTION()
	void CheckBoxChangeCallback(bool bIsChecked);

	FOnTickboxValueChanged OnTickboxValueChanged;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UCheckBox> AttributeCheckBox;
	
	bool* bAttributeBoolValue;
};
