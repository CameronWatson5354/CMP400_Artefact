// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcSwordData.h"

void UProcSwordData::SetSwordMesh(UStaticMesh* NewSwordMesh)
{
	SwordMesh = NewSwordMesh;
}

UStaticMesh* UProcSwordData::GetSwordMesh() const
{
	return SwordMesh;
}

void UProcSwordData::SetDynamicSwordMesh(UDynamicMeshComponent* NewDynamicSwordMesh)
{
	DynamicSwordMesh = NewDynamicSwordMesh;
}

UDynamicMeshComponent* UProcSwordData::GetDynamicSwordMesh() const
{
	return DynamicSwordMesh;
}

void UProcSwordData::SetGenerationParameters(const FProcCombinedGenParams& NewGenerationParameters)
{
	GenerationParameters = NewGenerationParameters;
}

const FProcCombinedGenParams& UProcSwordData::GetGenerationParameters() const
{
	return GenerationParameters;
}

void UProcSwordData::SetSwordAttributes(const FProcSwordAttributes& NewSwordAttributes)
{
	SwordAttributes = NewSwordAttributes;
}

const FProcSwordAttributes& UProcSwordData::GetSwordAttributes() const
{
	return SwordAttributes;
}
