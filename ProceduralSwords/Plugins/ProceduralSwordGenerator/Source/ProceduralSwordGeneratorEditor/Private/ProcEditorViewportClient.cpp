// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcEditorViewportClient.h"

void FProcEditorViewportClient::Draw(FViewport* InViewport, FCanvas* Canvas)
{
	FEditorViewportClient::Draw(Viewport, Canvas);
}

void FProcEditorViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);

	if (DeltaSeconds > 0.0f)
	{
		PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
	}
}
