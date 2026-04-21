// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProcSwordTypes.generated.h"

UENUM(BlueprintType)
enum class EProcRandomiseType : uint8
{
	All = 0,
	Blade,
	Guard,
	Grip,
	Pommel
};

USTRUCT(BlueprintType)
struct FProcMeshMassInfo
{
	GENERATED_BODY()

	FProcMeshMassInfo()
		: Position(FVector::ZeroVector)
	{};

	FProcMeshMassInfo(float InMass, const FVector& InPosition, int InTextureIndex)
		: Mass(InMass),
	Position(InPosition),
	TextureIndex(InTextureIndex)
	{};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Mass{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Position;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int TextureIndex{0};

	UPROPERTY(visibleAnywhere, BlueprintReadOnly)
	float Weight{0.0f};
	
};

USTRUCT(BlueprintType)
struct FSwordPointData
{
	GENERATED_BODY()
	
	FSwordPointData()
	{
		AttachPoint = FVector::ZeroVector;
	};

	FVector GetAttachPoint();
	void AddAttachPoint(const FVector& LocalAttachPoint);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector AttachPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FVector> AttachPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FProcMeshMassInfo> MeshMassInfos;
	
};

USTRUCT(BlueprintType)
struct FProcCubicBezier
{
	GENERATED_BODY()

	FProcCubicBezier()
	{};

	FProcCubicBezier(const FVector& InPointA, const FVector& InPointB, const FVector& InPointC, const FVector& InPointD)
		: PointA(InPointA),
	PointB(InPointB),
	PointC(InPointC),
	PointD(InPointD)
	{};

	FVector SamplePoint(float T);
	FVector SamplePointNormal(float T, const FVector& OtherAxis, float NormalDistance);
	
	TArray<FProcVertex> SamplePoints(int Samples);
	TArray<FProcVertex> SamplePointsOffNormals(int Samples, const FVector& OtherAxis, float NormalDistance, float EndNormalDistance = -1);

	TArray<FProcVertex> SamplePointsRotated(int Samples, const FVector& OtherAxis, float Angle, FVector PointMultiplier = FVector::OneVector);

	FVector GetTangent(float T);

	FVector GetNormal(float T, const FVector& OtherAxis);
	
	void CreateLengthLUT();


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector PointA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector PointB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector PointC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector PointD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Offset;

protected:
	TArray<float> ArcLengths;
	float TotalLength{0.0f};
};

USTRUCT(BlueprintType)
struct FProcQuadraticBezier
{
	GENERATED_BODY()

	FProcQuadraticBezier()
	{};

	FProcQuadraticBezier(const FVector& InPointA, const FVector& InPointB, const FVector& InPointC)
		: PointA(InPointA),
	PointB(InPointB),
	PointC(InPointC)
	{
		CreateLengthLUT();
	};

	FVector SamplePoint(float T);
	FVector SampleNormalised(float T);
	TArray<FProcVertex> SamplePoints(int Samples);
	
	void CreateLengthLUT();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector PointA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector PointB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector PointC;

protected:
	TArray<float> ArcLengths;
	float TotalLength{0.0f};
};

USTRUCT()
struct FProcVertex
{
	GENERATED_BODY()

	FProcVertex()
		: Location()
	{}
	
	FProcVertex(FVector InLocation)
		: Location(InLocation)
	{};

	int Index{0};

	FVector Location;
};

USTRUCT()
struct FProcPolyLine
{
	GENERATED_BODY()

	FProcPolyLine()
	{};

	FProcPolyLine(const TArray<FProcVertex>& InPoints)
		: Points(InPoints)
	{};

	TArray<FProcVertex> MirrorByPlane(const FPlane& Plane);

	TArray<FProcVertex> Points;
};


USTRUCT(BlueprintType)
struct FProcGenerationParameter
{
	GENERATED_BODY()

	FProcGenerationParameter()
	{}

	void Clamp(TOptional<float> MinConstraint = TOptional<float>{}, TOptional<float> MaxConstraint = TOptional<float>{});

	void Randomise(TOptional<float> MinConstraint = TOptional<float>{}, TOptional<float> MaxConstraint = TOptional<float>{});
	
	operator float() const;

	FProcGenerationParameter& operator=(const float& F);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinValue{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxValue{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MainValue{0.0f};
	
};


USTRUCT(BlueprintType)
struct FGeneralGenParams
{
	GENERATED_BODY()

	FGeneralGenParams()
	{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int SectionSteps{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int TextureIndex{0};
};

USTRUCT(BlueprintType)
struct FProcBladeGenParams : public FGeneralGenParams
{
	GENERATED_BODY()

	FProcBladeGenParams()
	{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter Length;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter Height;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter Thickness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter TipTaperLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter MiddleWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter TipHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 1, UIMin = 0, UIMax = 1))
	FProcGenerationParameter TipCurve;
};

USTRUCT(BlueprintType)
struct FProcGuardGenParams : public FGeneralGenParams
{
	GENERATED_BODY()

	FProcGuardGenParams()
	{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter Length;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter Width;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter Thickness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter CurveOffsetOuter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter CurveOffsetInner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSymmetrical{true};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bSymmetrical"))
	FProcGenerationParameter OppositeCurveOffsetOuter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bSymmetrical"))
	FProcGenerationParameter OppositeCurveOffsetInner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter WidthTaper;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter InnerWidthTaper;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter ThicknessTaper;
};

USTRUCT(BlueprintType)
struct FProcGripGenParams : public FGeneralGenParams
{
	GENERATED_BODY()

	FProcGripGenParams()
	{};


	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter Length;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter PommelRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter InnerPommelRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter InnerGuardRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter GuardRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int RadialSections{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter HandleWidthMultiplier;
};

USTRUCT(BlueprintType)
struct FProcPommelGenParams : public FGeneralGenParams
{
	GENERATED_BODY()

	FProcPommelGenParams()
	{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter Length;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int RadialSections{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter TopRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter InnerTopRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter InnerBottomRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter BottomRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProcGenerationParameter WidthMultiplier;
};

USTRUCT(BlueprintType)
struct FProcSwordAttributes
{
	GENERATED_BODY()

	FProcSwordAttributes()
		: CentreOfMass(FVector::ZeroVector)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight{0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	float BladeLength{0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	float HitboxWidth{0.0};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hitbox")
	float HitboxOffset{0.0f};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hitbox")
	FVector CentreOfMass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Damage")
	float SlashDamage{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Damage")
	float StabDamage{0.0f};
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage")
	float BladeSharpness{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage")
	float TipSharpness{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage")
	float TipPressure{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage")
	float SlashKnockback{0.0f};
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage")
	float StabKnockback{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Handling")
	float SwingSpeed{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Handling")
	float StabSpeed{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Handling")
	float TotalLength{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Handling")
	float GripLength{0.0f};

	
	
};

USTRUCT(BlueprintType)
struct FProcCombinedGenParams
{
	GENERATED_BODY()

	FProcCombinedGenParams() {};

	void EnforceRules();
	void Randomise(EProcRandomiseType RandomiseType = EProcRandomiseType::All);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenerationParams", meta = (ToolTip = "Higher values allow the swords to be swung easier, allowing less realistic designs to still be functional."))
	float StrengthMultiplier{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenerationParams", meta = (Units = "Degrees"))
	float MinSwingAcceleration{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenerationParams", meta = (Units = "Degrees"))
	float MaxSwingAcceleration{0.0f};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenerationParams")
	FProcBladeGenParams BladeGenParams;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenerationParams")
	FProcGuardGenParams GuardGenParams;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenerationParams")
	FProcGripGenParams GripGenParams;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenerationParams")
	FProcPommelGenParams PommelGenParams;
	
};
