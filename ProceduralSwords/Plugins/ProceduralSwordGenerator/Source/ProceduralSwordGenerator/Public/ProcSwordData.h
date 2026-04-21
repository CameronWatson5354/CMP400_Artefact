// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ProcSwordTypes.h"
#include "ProcSwordData.generated.h"

class UDynamicMeshComponent;
class UDynamicMesh;
/**
 * 
 */
UCLASS()
class PROCEDURALSWORDGENERATOR_API UProcSwordData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	void SetSwordMesh(UStaticMesh* NewSwordMesh);
	UStaticMesh* GetSwordMesh() const;

	void SetDynamicSwordMesh(UDynamicMeshComponent* NewDynamicSwordMesh);
	UDynamicMeshComponent* GetDynamicSwordMesh() const;

	void SetGenerationParameters(const FProcCombinedGenParams& NewGenerationParameters);
	const FProcCombinedGenParams& GetGenerationParameters() const;
	
	void SetSwordAttributes(const FProcSwordAttributes& NewSwordAttributes);
	const FProcSwordAttributes& GetSwordAttributes() const;

	

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMesh> SwordMesh{nullptr};

	UPROPERTY(EditAnywhere)
	TObjectPtr<UDynamicMeshComponent> DynamicSwordMesh{nullptr};

	UPROPERTY(EditAnywhere)
	FProcCombinedGenParams GenerationParameters;

	UPROPERTY(EditAnywhere)
	FProcSwordAttributes SwordAttributes;
	
};
