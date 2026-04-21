// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FProcViewportClient : public FEditorViewportClient
{
public:
	FProcViewportClient(FEditorModeTools* InModeTools, FPreviewScene* InPreviewScene, const TWeakPtr<SEditorViewport>& InEditorViewportWidget);

	virtual void Tick(float DeltaSeconds) override;
};
