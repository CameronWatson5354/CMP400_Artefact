// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProcSwordTypes.h"
#include "GameFramework/Actor.h"
#include "ProcSwordGeneratorB.generated.h"

class UProcSwordData;
class UDynamicMesh;
class UDynamicMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeshCreatedSignature, const FProcSwordAttributes&, SwordAttributes);

UCLASS()
class PROCEDURALSWORDGENERATOR_API AProcSwordGeneratorB : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProcSwordGeneratorB();

	virtual void PostInitializeComponents() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	void UpdateMesh();

	UFUNCTION(BlueprintCallable)
	void ResetParameters();


#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(BlueprintCallable)
	void RebuildDynamicMesh();

	UFUNCTION(BlueprintCallable)
	void GenerateRandomMesh(EProcRandomiseType RandomiseType);

	UFUNCTION(BlueprintImplementableEvent, CallInEditor)
	void DrawDebugLines();

	void NativeDrawDebugLines();
	void CalculateGameplayAttributes(UDynamicMesh* TargetMesh);

	UFUNCTION(BlueprintImplementableEvent, CallInEditor)
	void OnRebuildDynamicMesh(UDynamicMesh* DynamicMesh);

	UFUNCTION(BlueprintCallable, CallInEditor)
	void NativePostProcessMesh(UDynamicMesh* TargetMesh);

	UFUNCTION(BlueprintCallable, CallInEditor)
	UProcSwordData* SaveSwordIntoData();

	UDynamicMeshComponent* GetDynamicMeshComp() const {return DynamicMeshComp;};
	const FProcSwordAttributes& GetSwordAttributes() const;

	UFUNCTION(BlueprintCallable)
	const FProcCombinedGenParams& GetCombinedGenParams() const;

	void SetCurrentGenParams(const FProcCombinedGenParams& NewCurrentGenParams);

	UPROPERTY(BlueprintCallable)
	FOnMeshCreatedSignature OnMeshCreated;

	void SetUseDefaultParams(bool bValue);

	UFUNCTION(BlueprintCallable)
	void TestRandomIterations(int Iterations, const FString& FilePath);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FVector CalculateCentreMass(TArray<FProcMeshMassInfo>& MeshMassInfos);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> DefaultRootComp{nullptr};

	UPROPERTY(visibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UDynamicMeshComponent> DynamicMeshComp{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bRenderDebugLines{false};
	
	//runtime
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FSwordPointData PointData;

	UPROPERTY(visibleAnywhere, BlueprintReadWrite)
	FProcSwordAttributes Attributes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenerationParams")
	FProcCombinedGenParams DefaultGenerationParams;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenerationParams")
	FProcCombinedGenParams CurrentGenerationParams;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UMaterialInterface>> MeshMaterials;

	UPROPERTY()
	bool bUseDefaultParams{true};

	UPROPERTY(visibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UDynamicMesh> SubtractionMesh;
	
	UPROPERTY(visibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UDynamicMesh> BladeMesh;

	UPROPERTY(visibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UDynamicMesh> GuardMesh;

	UPROPERTY(visibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UDynamicMesh> GripMesh;

	UPROPERTY(visibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UDynamicMesh> PommelMesh;

};

