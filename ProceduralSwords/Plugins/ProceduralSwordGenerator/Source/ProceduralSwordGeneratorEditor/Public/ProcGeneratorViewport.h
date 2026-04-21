// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SCommonEditorViewportToolbarBase.h"
#include "SEditorViewport.h"

class FProcEditorViewportClient;
class FProcViewportClient;
class FAdvancedPreviewScene;

class SProcSwordGeneratorViewport : public SEditorViewport,
                                    public ICommonEditorViewportToolbarInfoProvider
{
	SLATE_BEGIN_ARGS(SProcSwordGeneratorViewport)
	{
	}
	SLATE_END_ARGS()

	AActor* SpawnActor(TSubclassOf<AActor> ActorClass, FTransform SpawnTransform = FTransform{});

	void Construct(const FArguments& InArgs);

	virtual TSharedRef<class SEditorViewport> GetViewportWidget() override;
	virtual TSharedPtr<FExtender> GetExtenders() const override;
	virtual void OnFloatingButtonClicked() override;

protected:
	virtual void OnFocusViewportToSelection() override;

	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;

	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	
	TSharedPtr<FAdvancedPreviewScene> AdvancedPreviewScene{nullptr};

	TSharedPtr<FProcViewportClient> ViewportClient;

	TArray<TSoftObjectPtr<AActor>> PreviewActors{nullptr};
	
};
