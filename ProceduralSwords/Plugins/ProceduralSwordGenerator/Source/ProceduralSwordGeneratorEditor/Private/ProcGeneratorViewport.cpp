// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcGeneratorViewport.h"

#include "AdvancedPreviewScene.h"
#include "ProcEditorViewportClient.h"
#include "ProcViewportClient.h"

AActor* SProcSwordGeneratorViewport::SpawnActor(TSubclassOf<AActor> ActorClass, FTransform SpawnTransform)
{
	if (ActorClass)
	{
		if (AdvancedPreviewScene.IsValid() && AdvancedPreviewScene->GetWorld())
		{	
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			
			AActor* PreviewActor{AdvancedPreviewScene->GetWorld()->SpawnActor<AActor>(ActorClass, SpawnTransform, SpawnParams)};

			
			ViewportClient->FocusViewportOnBox(PreviewActor->GetComponentsBoundingBox(true));
			PreviewActors.Add(PreviewActor);
			
			return PreviewActor;
		}
	}

	return nullptr;
}

void SProcSwordGeneratorViewport::Construct(const FArguments& InArgs)
{
	AdvancedPreviewScene = MakeShareable(new FAdvancedPreviewScene(FPreviewScene::ConstructionValues()));
	AdvancedPreviewScene->SetFloorVisibility(false);

	SEditorViewport::Construct(SEditorViewport::FArguments());
}

TSharedRef<class SEditorViewport> SProcSwordGeneratorViewport::GetViewportWidget()
{
	return SharedThis(this);
}


TSharedPtr<FExtender> SProcSwordGeneratorViewport::GetExtenders() const
{
	return MakeShareable(new FExtender);
}

void SProcSwordGeneratorViewport::OnFloatingButtonClicked()
{
}

void SProcSwordGeneratorViewport::OnFocusViewportToSelection()
{
	if (PreviewActors.Last().IsValid())
	{
		ViewportClient->FocusViewportOnBox(PreviewActors.Last()->GetComponentsBoundingBox(true));
	}
}

TSharedPtr<SWidget> SProcSwordGeneratorViewport::MakeViewportToolbar()
{
	return SNew(SCommonEditorViewportToolbarBase, SharedThis(this))
		.Cursor(EMouseCursor::Default);
}

TSharedRef<FEditorViewportClient> SProcSwordGeneratorViewport::MakeEditorViewportClient()
{
	ViewportClient = MakeShareable(new FProcViewportClient(nullptr, AdvancedPreviewScene.Get(), SharedThis(this)));

	ViewportClient->ViewportType = LVT_Perspective;
	ViewportClient->bSetListenerPosition = false;
	ViewportClient->SetRealtime(true);

	return ViewportClient.ToSharedRef();
}
