// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcScenePreviewWidget.h"

#include "ProcGeneratorViewport.h"

AActor* UProcScenePreviewWidget::SpawnActorInScene(TSubclassOf<AActor> ActorClass, FTransform SpawnTransform)
{
	if (ActorClass)
	{
		return Viewport->SpawnActor(ActorClass, SpawnTransform);
	}
	
	return nullptr;
}

void UProcScenePreviewWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
}

void UProcScenePreviewWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	Viewport.Reset();

}

TSharedRef<SWidget> UProcScenePreviewWidget::RebuildWidget()
{
	if (IsDesignTime())
	{
		return SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Sword Generator Viewport")))
		];
	}
	else
	{
		Viewport = SNew(SProcSwordGeneratorViewport);
	}
	
	return Viewport.ToSharedRef();
}
