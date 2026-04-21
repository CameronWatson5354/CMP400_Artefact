// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcGeneratorWidget.h"

#include "EditorUtilityWidgetComponents.h"
#include "Editor/ScriptableEditorWidgets/Public/Components/DetailsView.h"
#include "ProcSwordGeneratorB.h"
#include "IAssetTools.h"
#include "ProcScenePreviewWidget.h"
#include "GeometryScript/MeshAssetFunctions.h"

#include "ProcSwordData.h"
#include "Components/DynamicMeshComponent.h"

void UProcGeneratorWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UProcGeneratorWidget::NativeConstruct()
{
	CreateGenerator();

	Super::NativeConstruct();
}

void UProcGeneratorWidget::CreateGenerator()
{
	//spawns generator in preview widget and binds events
	if (IsValid(ScenePreviewWidget))
	{
		FTransform CompareActorTransform = FTransform::Identity;
		CompareActorTransform.SetLocation(FVector{0.0f, 100.0f, 0.0f});
		
		ScenePreviewWidget->SpawnActorInScene(SwordGeneratorClass.LoadSynchronous(), CompareActorTransform);

		
		SwordGenerator = Cast<AProcSwordGeneratorB>(ScenePreviewWidget->SpawnActorInScene(SwordGeneratorClass.LoadSynchronous()));
		if (SwordGenerator.IsValid())
		{
			if (IsValid(ParamDetailsView))
			{
				ParamDetailsView->SetObject(SwordGenerator.Get());
			}

			SwordGenerator->OnMeshCreated.AddDynamic(this, &UProcGeneratorWidget::MeshCreated);
			SwordGenerator->SetUseDefaultParams(false);
			
		}
	}
}

void UProcGeneratorWidget::DestroyGenerator()
{
	if (SwordGenerator.IsValid())
	{
		SwordGenerator->Destroy();
	}
}

void UProcGeneratorWidget::RandomiseSwordMesh(EProcRandomiseType RandomiseType)
{
	if (SwordGenerator.IsValid())
	{
		SwordGenerator->GenerateRandomMesh(RandomiseType);
	}
}

void UProcGeneratorWidget::SaveDataAsset()
{
	IAssetTools& AssetTools{IAssetTools::Get()};
	
	FString MeshName{"SM_" + FileNameTextBox->GetText().ToString()};
	UStaticMesh* StaticMesh{Cast<UStaticMesh>(AssetTools.CreateAsset(MeshName, PathTextBox->GetText().ToString(), UStaticMesh::StaticClass(), nullptr))};
	
	FGeometryScriptCopyMeshToAssetOptions Options;
	FGeometryScriptMeshWriteLOD LOD;
	EGeometryScriptOutcomePins Outcome;

	//mesh may be invalid if user cancels the creation due to the same name
	if (IsValid(StaticMesh))
	{
		UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshToStaticMesh(SwordGenerator->GetDynamicMeshComp()->GetDynamicMesh(), StaticMesh, Options, LOD, Outcome);

		//copy materials from dynamic mesh to static mesh
		TArray<UMaterialInterface*> DynamicMaterials{SwordGenerator->GetDynamicMeshComp()->GetMaterials()};
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
	FString DataName{"SD_" + FileNameTextBox->GetText().ToString()};
	UProcSwordData* SwordData{Cast<UProcSwordData>(AssetTools.CreateAsset(DataName, PathTextBox->GetText().ToString(), UProcSwordData::StaticClass(), nullptr))};
	if (IsValid(SwordData))
	{
		SwordData->SetSwordMesh(StaticMesh);
		
		if (SwordGenerator.IsValid())
		{
			SwordData->SetSwordAttributes(SwordGenerator->GetSwordAttributes());
			SwordData->SetGenerationParameters(SwordGenerator->GetCombinedGenParams());
		}
		
	}
}



