// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcViewportClient.h"

FProcViewportClient::FProcViewportClient(FEditorModeTools* InModeTools, FPreviewScene* InPreviewScene, const TWeakPtr<SEditorViewport>& InEditorViewportWidget)
	: FEditorViewportClient(InModeTools, InPreviewScene, InEditorViewportWidget)
{
}

void FProcViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);

	if (PreviewScene)
	{
		if (IsValid(PreviewScene->GetWorld()))
		{
			PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
		}
	}
}
