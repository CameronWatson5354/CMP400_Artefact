// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "ProcScenePreviewWidget.generated.h"

class SProcSwordGeneratorViewport;
/**
 * 
 */
UCLASS()
class PROCEDURALSWORDGENERATOREDITOR_API UProcScenePreviewWidget : public UWidget
{
	GENERATED_BODY()

public:
	AActor* SpawnActorInScene(TSubclassOf<AActor> ActorClass, FTransform SpawnTransform = FTransform{});
	
	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	TSharedPtr<SProcSwordGeneratorViewport> Viewport;
};
