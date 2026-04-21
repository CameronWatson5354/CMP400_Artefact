// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "ProcSwordTypes.h"
#include "ProcFuncLib.generated.h"

class UDynamicMesh;

UCLASS()
class PROCEDURALSWORDGENERATOR_API UProcFuncLib : public UBlueprintFunctionLibrary
{
	
	GENERATED_BODY()

public:

	
	
	

	UFUNCTION(BlueprintCallable, Category = "Procedural Sword Functions")
	static UDynamicMesh* AppendBladeMesh(UDynamicMesh* TargetMesh, UPARAM(ref) FProcBladeGenParams& Params, UPARAM(ref) FSwordPointData& PointData, UPARAM(ref) FProcSwordAttributes& Attributes);

	UFUNCTION(BlueprintCallable, Category = "Procedural Sword Functions")
	static UDynamicMesh* AppendGuardMesh(UDynamicMesh* TargetMesh, UPARAM(ref) FProcGuardGenParams& Params, UPARAM(ref) FSwordPointData& PointData, UPARAM(ref) FProcSwordAttributes& Attributes);

	UFUNCTION(BlueprintCallable, Category = "Procedural Sword Functions")
	static UDynamicMesh* AppendGripMesh(UDynamicMesh* TargetMesh, UPARAM(ref) FProcGripGenParams& Params, UPARAM(ref) FSwordPointData& PointData, UPARAM(ref) FProcSwordAttributes& Attributes);

	UFUNCTION(BlueprintCallable, Category = "Procedural Sword Functions")
	static UDynamicMesh* AppendPommelMesh(UDynamicMesh* TargetMesh, UPARAM(ref) FProcPommelGenParams& Params, UPARAM(ref) FSwordPointData& PointData, UPARAM(ref) FProcSwordAttributes& Attributes);

	

	UFUNCTION(BlueprintCallable, Category = "Procedural Sword Functions")
	static UDynamicMesh* AppendGuardSubtractionMesh(UDynamicMesh* TargetMesh, UPARAM(ref) FProcGuardGenParams& Params);

	UFUNCTION(BlueprintCallable, Category = "Procedural Sword Functions")
	static UDynamicMesh* AppendGripSubtractionMesh(UDynamicMesh* TargetMesh, UPARAM(ref) FProcGuardGenParams& Params);

	//functions for writing test data to text adapted from youtube tutorials
	//code from Alex Quevillion [En]: https://www.youtube.com/watch?v=YcTa_wMjCzY
	//{
	static FString ReadStringFromFile(FString FilePath, bool& bOutSuccess, FString& OutInfoMessage);
	static void WriteStringToFile(FString FilePath, FString TextToWrite, bool& bOutSuccess, FString& OutInfoMessage);
	//}
	
	//code from Alex Quevillion [En]: https://www.youtube.com/watch?v=4wJ45mrzrkM
	//{
	static TSharedPtr<FJsonObject> ReadJson(FString FilePath, bool& bOutSuccess, FString& OutInfoMessage);
	static void WriteJson(FString FilePath, TSharedPtr<FJsonObject> JsonObject, bool& bOutSuccess, FString& OutInfoMessage);

	UFUNCTION(BlueprintCallable)
	static void WriteStructToJsonFile(FString FilePath, FProcSwordAttributes Struct, bool& bOutSuccess, FString& OutInfoMessage);
	//]
	
private:
	static void AppendMesh(UDynamicMesh* TargetMesh, const UE::Geometry::FDynamicMesh3& AppendMesh);
	static void SetMaterialIDAllTriangles(UE::Geometry::FDynamicMesh3& Mesh, int MaterialIndex = 0);


	static void SampleAlongLine(const FVector& Start, const FVector& End, TArray<FProcVertex>& Points, int Samples);

	static void AppendPolyLineVertices(UE::Geometry::FDynamicMesh3& Mesh, TArray<FProcVertex>& PolyLine);
	static void TriangulatePolyLines(UE::Geometry::FDynamicMesh3& Mesh, const TArray<FProcVertex>& PolyLineA, const TArray<FProcVertex>& PolyLineB, bool bReverse = false);
};
