// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProcGenAttributeWidget.generated.h"

class UTextBlock;
class UHorizontalBox;
/**
 * 
 */
UCLASS()
class PROCEDURALSWORDGENERATOR_API UProcGenAttributeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UTextBlock* GetAttributeText();
	UHorizontalBox* GetAttributeListBox();
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UHorizontalBox> AttributeListBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> AttributeText;
};
