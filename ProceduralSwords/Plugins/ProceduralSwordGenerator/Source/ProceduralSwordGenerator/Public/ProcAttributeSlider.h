// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProcAttributeSlider.generated.h"

class UProcSpinBox;
class UTextBlock;
class USpinBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnValueChanged);

UENUM()
enum class EValueType
{
	None = 0,
	Float,
	Int
};

UCLASS()
class PROCEDURALSWORDGENERATOR_API UProcAttributeSlider : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	USpinBox* GetAttributeSlider();
	UTextBlock* GetTitleText();

	void SetAttributeFloatValue(float* NewAttributeFloatValue);
	void SetAttributeIntValue(int* NewAttributeIntValue);

	void UpdateValue(bool bSliderNotify);

	FOnValueChanged OnAnyValueChanged;

	UFUNCTION(BlueprintImplementableEvent)
	void ModifyMainValueAppearance();

protected:
	UFUNCTION()
	void SliderChangeCallback(float InValue);

	UFUNCTION()
	void CommitChangeCallback(float InValue, ETextCommit::Type CommitMethod);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<USpinBox> AttributeSlider;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	float* AttributeFloatValue;
	int* AttributeIntValue;

	EValueType ValueType{0};

	UPROPERTY()
	TOptional<bool> bNotifChanges;
};
