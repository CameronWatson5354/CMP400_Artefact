// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "ProcSwordTypes.h"
#include "ProcGeneratorWidget.generated.h"

class UProcScenePreviewWidget;
class UEditorUtilityEditableTextBox;
class UEditableTextBox;
class AProcSwordGeneratorB;
class AProcSwordGenerator;
class UDetailsView;
class UProcParameterObject;

UCLASS(Blueprintable, BlueprintType)
class PROCEDURALSWORDGENERATOREDITOR_API UProcGeneratorWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

protected:
	void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	void CreateGenerator();

	void DestroyGenerator();

	UFUNCTION(BlueprintCallable)
	void RandomiseSwordMesh(EProcRandomiseType RandomiseType);

	UFUNCTION(BlueprintCallable)
	void SaveDataAsset();

	UFUNCTION(BlueprintImplementableEvent)
	void MeshCreated(const FProcSwordAttributes& SwordAttributes);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<AProcSwordGeneratorB> SwordGenerator{nullptr};

	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<AProcSwordGeneratorB> SwordGeneratorClass{nullptr};
	
	UPROPERTY(EditDefaultsOnly)
	FTransform GeneratorSpawnTransform;

	UPROPERTY(blueprintreadwrite, meta = (BindWidget))
	TObjectPtr<UDetailsView> ParamDetailsView;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UEditorUtilityEditableTextBox> FileNameTextBox;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UEditorUtilityEditableTextBox> PathTextBox;

	UPROPERTY(blueprintreadwrite, meta = (BindWidget))
	TObjectPtr<UProcScenePreviewWidget> ScenePreviewWidget;

	UPROPERTY(visibleAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UWorld> EditorWorld{nullptr};
};
