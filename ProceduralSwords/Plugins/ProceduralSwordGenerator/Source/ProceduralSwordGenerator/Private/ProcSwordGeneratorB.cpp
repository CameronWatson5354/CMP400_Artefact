// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcSwordGeneratorB.h"

#include "ProcFuncLib.h"
#include "ProcSwordData.h"
#include "Components/DynamicMeshComponent.h"
#include "Components/SplineComponent.h"
#include "GeometryScript/MeshAssetFunctions.h"
#include "GeometryScriptingCore/Public/GeometryScript/MeshQueryFunctions.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"


// Sets default values
AProcSwordGeneratorB::AProcSwordGeneratorB()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DefaultRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRootComp"));
	SetRootComponent(DefaultRootComp);

	DynamicMeshComp = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("DynamicMeshComp"));
	DynamicMeshComp->SetupAttachment(GetRootComponent());

	SubtractionMesh = CreateDefaultSubobject<UDynamicMesh>(TEXT("SubtractionMesh"));
	BladeMesh = CreateDefaultSubobject<UDynamicMesh>(TEXT("BladeMesh"));
	GuardMesh = CreateDefaultSubobject<UDynamicMesh>(TEXT("GuardMesh"));
	GripMesh = CreateDefaultSubobject<UDynamicMesh>(TEXT("GripMesh"));
	PommelMesh = CreateDefaultSubobject<UDynamicMesh>(TEXT("PommelMesh"));


	//RebuildDynamicMesh();
}

void AProcSwordGeneratorB::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//RebuildDynamicMesh();
}

void AProcSwordGeneratorB::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	UpdateMesh();


	//RebuildDynamicMesh();
}

void AProcSwordGeneratorB::UpdateMesh()
{
	DefaultGenerationParams.EnforceRules();
	CurrentGenerationParams.EnforceRules();

	RebuildDynamicMesh();
}

void AProcSwordGeneratorB::ResetParameters()
{
	CurrentGenerationParams = DefaultGenerationParams;
	RebuildDynamicMesh();
}

#if WITH_EDITOR
void AProcSwordGeneratorB::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	UpdateMesh();
}
#endif

void AProcSwordGeneratorB::RebuildDynamicMesh()
{
	SubtractionMesh->Reset();
	BladeMesh->Reset();
	GuardMesh->Reset();
	GripMesh->Reset();
	PommelMesh->Reset();

	PointData = FSwordPointData{};
	Attributes = FProcSwordAttributes{};

	DynamicMeshComp->GetDynamicMesh()->Reset();

	OnRebuildDynamicMesh(DynamicMeshComp->GetDynamicMesh());
}

void AProcSwordGeneratorB::GenerateRandomMesh(EProcRandomiseType RandomiseType)
{
	CurrentGenerationParams.Randomise(RandomiseType);
}

void AProcSwordGeneratorB::NativeDrawDebugLines()
{
	DrawDebugLines();

	for (auto& MassObject : PointData.MeshMassInfos)
	{
		DrawDebugSphere(GetWorld(), MassObject.Position, 5.0f, 12, FColor::White);
	}

	DrawDebugSphere(GetWorld(), Attributes.CentreOfMass, 10.0f, 12, FColor::Red);
}

void AProcSwordGeneratorB::CalculateGameplayAttributes(UDynamicMesh* TargetMesh)
{
	float Area{0.0f};
	float Volume{0.0f};
	FVector Centre{};

	//gather volume, centre and texture index of all sword meshes
	UGeometryScriptLibrary_MeshQueryFunctions::GetMeshVolumeAreaCenter(BladeMesh, Area, Volume, Centre);
	PointData.MeshMassInfos.Emplace(Volume, Centre, GetCombinedGenParams().BladeGenParams.TextureIndex);

	UGeometryScriptLibrary_MeshQueryFunctions::GetMeshVolumeAreaCenter(GuardMesh, Area, Volume, Centre);
	PointData.MeshMassInfos.Emplace(Volume, Centre, GetCombinedGenParams().GuardGenParams.TextureIndex);

	UGeometryScriptLibrary_MeshQueryFunctions::GetMeshVolumeAreaCenter(GripMesh, Area, Volume, Centre);
	PointData.MeshMassInfos.Emplace(Volume, Centre, GetCombinedGenParams().GripGenParams.TextureIndex);

	UGeometryScriptLibrary_MeshQueryFunctions::GetMeshVolumeAreaCenter(PommelMesh, Area, Volume, Centre);
	PointData.MeshMassInfos.Emplace(Volume, Centre, GetCombinedGenParams().PommelGenParams.TextureIndex);

	Attributes.Weight = 0.0f;

	Attributes.HitboxOffset = Attributes.TotalLength;

	//centre mass calculation
	for (auto& MassObject : PointData.MeshMassInfos)
	{
		MassObject.Position = UKismetMathLibrary::RotateAngleAxis(MassObject.Position, 90, FVector::ForwardVector);

		//iterate over components to find the total weight of the sword
		if (MeshMaterials.Num() > MassObject.TextureIndex)
		{
			if (UPhysicalMaterial* PhysMat{MeshMaterials[MassObject.TextureIndex]->GetPhysicalMaterial()};
				IsValid(PhysMat))
			{
				MassObject.Weight = MassObject.Mass * (PhysMat->Density * 0.001);
				Attributes.Weight += MassObject.Weight * 0.000001;
				
				
			}
		}
	}

	Attributes.CentreOfMass = CalculateCentreMass(PointData.MeshMassInfos);

	float ArmForce{100.0f};

	//swing speed
	float Radius{static_cast<float>(Attributes.CentreOfMass.Z / 100 + 0.5f)};
	float MomentInertia{Attributes.Weight * (Radius * Radius)};
	float Torque{ArmForce * CurrentGenerationParams.StrengthMultiplier};
	float RotationalAcceleration{Torque / MomentInertia};
	Attributes.SwingSpeed = FMath::RadiansToDegrees(RotationalAcceleration);
	Attributes.SwingSpeed = FMath::Clamp(Attributes.SwingSpeed, CurrentGenerationParams.MinSwingAcceleration, CurrentGenerationParams.MaxSwingAcceleration);


	//slashboost is used to make light swords do more damage
	float SlashBoost{FMath::Clamp(FMath::Sqrt(RotationalAcceleration), 1.0f, 200.0f)};
	Attributes.SlashDamage = MomentInertia * Attributes.BladeSharpness * SlashBoost;

	
	float StabAcceleration{((ArmForce * CurrentGenerationParams.StrengthMultiplier) / Attributes.Weight) * 100.0f};
	Attributes.StabSpeed = StabAcceleration;

	float StabBoost{FMath::Clamp(FMath::Sqrt(StabAcceleration * 0.01f), 1.0f, 200.0f)};

	float AverageSharpness = Attributes.BladeSharpness * 0.25 + Attributes.TipSharpness * 0.75;
	Attributes.StabDamage = (Attributes.Weight * (AverageSharpness * AverageSharpness));
	Attributes.StabDamage += Attributes.StabDamage * FMath::Sqrt(Attributes.TipPressure) * StabBoost;

	float MaxKnockbackSpeed{1000.0f};
	Attributes.SlashKnockback = MaxKnockbackSpeed / (RotationalAcceleration * 2) * SlashBoost * (CurrentGenerationParams.StrengthMultiplier * CurrentGenerationParams.StrengthMultiplier);
	Attributes.StabKnockback = MaxKnockbackSpeed / (Attributes.StabSpeed * 0.01) * StabBoost * (CurrentGenerationParams.StrengthMultiplier * CurrentGenerationParams.StrengthMultiplier);
}

void AProcSwordGeneratorB::NativePostProcessMesh(UDynamicMesh* TargetMesh)
{
	CalculateGameplayAttributes(TargetMesh);
}

UProcSwordData* AProcSwordGeneratorB::SaveSwordIntoData()
{
	UStaticMesh* StaticMesh{NewObject<UStaticMesh>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0))};

	EGeometryScriptOutcomePins Outcome;

	//mesh may be invalid if user cancels the creation due to the same name
	if (IsValid(StaticMesh))
	{
		FGeometryScriptCopyMeshToAssetOptions Options;
		FGeometryScriptMeshWriteLOD LOD;
		UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshToStaticMesh(GetDynamicMeshComp()->GetDynamicMesh(), StaticMesh, Options, LOD, Outcome);

		//copy materials from dynamic mesh to static mesh
		TArray<UMaterialInterface*> DynamicMaterials{GetDynamicMeshComp()->GetMaterials()};
		if (DynamicMaterials.Num() >= 4)
		{
			TArray<FStaticMaterial> StaticMaterials;
			StaticMaterials.Emplace(FStaticMaterial{DynamicMaterials[0], "Pommel"});
			StaticMaterials.Emplace(FStaticMaterial{DynamicMaterials[1], "Grip"});
			StaticMaterials.Emplace(FStaticMaterial{DynamicMaterials[2], "Guard"});
			StaticMaterials.Emplace(FStaticMaterial{DynamicMaterials[3], "Blade"});

			FMeshUVChannelInfo Info{1.0f};
			for (auto& Material : StaticMaterials)
			{
				Material.UVChannelData = Info;
			}

			StaticMesh->SetStaticMaterials(StaticMaterials);
		}
	}

	//create data asset
	UProcSwordData* SwordData{NewObject<UProcSwordData>(this)};
	if (IsValid(SwordData))
	{
		if (Outcome == EGeometryScriptOutcomePins::Success)
		{
			SwordData->SetSwordMesh(StaticMesh);
		}
		else
		{
			UDynamicMeshComponent* DuplicatedMesh{DuplicateObject(GetDynamicMeshComp(), this)};
			
			SwordData->SetDynamicSwordMesh(DuplicatedMesh);
		}

		SwordData->SetSwordAttributes(GetSwordAttributes());
		SwordData->SetGenerationParameters(CurrentGenerationParams);
	}

	

	return SwordData;
}


const FProcSwordAttributes& AProcSwordGeneratorB::GetSwordAttributes() const
{
	return Attributes;
}

const FProcCombinedGenParams& AProcSwordGeneratorB::GetCombinedGenParams() const
{
	if (bUseDefaultParams)
	{
		return DefaultGenerationParams;
	}

	return CurrentGenerationParams;
}

void AProcSwordGeneratorB::SetCurrentGenParams(const FProcCombinedGenParams& NewCurrentGenParams)
{
	CurrentGenerationParams = NewCurrentGenParams;

	UpdateMesh();
}

void AProcSwordGeneratorB::SetUseDefaultParams(bool bValue)
{
	bUseDefaultParams = bValue;
}

void AProcSwordGeneratorB::TestRandomIterations(int Iterations, const FString& FilePath)
{
	for (int i{0}; i < Iterations; ++i)
	{
		GenerateRandomMesh(EProcRandomiseType::All);
		UpdateMesh();

		bool bOutSuccess{false};
		FString Message{""};
		
		UProcFuncLib::WriteStructToJsonFile(FilePath, GetSwordAttributes(), bOutSuccess, Message);
	}
}

// Called when the game starts or when spawned
void AProcSwordGeneratorB::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AProcSwordGeneratorB::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bRenderDebugLines)
	{
		NativeDrawDebugLines();
	}
}

FVector AProcSwordGeneratorB::CalculateCentreMass(TArray<FProcMeshMassInfo>& MeshMassInfos)
{
	FVector Centre{};
	FVector MultipliedMasses{};

	float TotalWeight{0.0f};

	//find the centre of mass of the sword
	for (auto& MassObject : MeshMassInfos)
	{
		MultipliedMasses += MassObject.Position * MassObject.Weight;
		TotalWeight += MassObject.Weight;
	}

	Centre = MultipliedMasses / TotalWeight;

	return Centre;
}
