// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProcSwordTypes.h"
#include "Blueprint/UserWidget.h"
#include "ProcInGameGeneratorWidget.generated.h"

class UProcAttributeTickBox;
class UProcGenAttributeWidget;
class UProcAttributeSlider;
class UVerticalBox;


UCLASS()
class PROCEDURALSWORDGENERATOR_API UProcInGameGeneratorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void OnGeneratorChanged();

	void UpdateGeneratorParams(const FProcCombinedGenParams& NewGenerationParameters);

	UFUNCTION(BlueprintCallable)
	void ResetSwordMesh();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void AdjustSwordView();
	
protected:
	void SpawnGenerator();
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void GenerateSlidersRecursive(FProperty* Property, void* Container, UPanelWidget* ParentPanel = nullptr);

	UFUNCTION(BlueprintCallable)
	void FindObjectsToGenerate(AProcSwordGeneratorB* Generator);

	
	

	void TickFunction(bool bSliderNotify);

	UFUNCTION(BlueprintCallable)
	void UpdateVariablesNextTick(bool bSliderNotify);

	UFUNCTION()
	void SliderChangeCallback();

	UFUNCTION()
	void CheckBoxChangeCallback(bool bIsChecked);

	UFUNCTION(BlueprintCallable)
	void SaveSword();

	


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UVerticalBox> VerticalBoxList;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UProcGenAttributeWidget> AttributeWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UProcAttributeSlider> SliderClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UProcAttributeTickBox> TickBoxClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<AProcSwordGeneratorB> SwordGenerator{nullptr};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftClassPtr<AProcSwordGeneratorB> GeneratorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftClassPtr<UUserWidget> AttributeTitleClass;

	UPROPERTY()
	TArray<TSoftObjectPtr<UProcAttributeSlider>> AttributeSliders;

	UPROPERTY()
	TArray<TSoftObjectPtr<UProcAttributeTickBox>> AttributeTickBoxes;

	UPROPERTY(EditDefaultsOnly)
	TSet<FString> GeneratorVariableNames;

	UPROPERTY(EditDefaultsOnly)
	TSet<FString> VariablesToHide;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FRotator SwordPreviewRotation;
};
