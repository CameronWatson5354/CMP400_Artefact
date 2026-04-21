// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcSwordTypes.h"

#include "VectorTypes.h"
#include "Kismet/KismetMathLibrary.h"

FVector FSwordPointData::GetAttachPoint()
{
	if (AttachPoints.IsEmpty())
	{
		return FVector::ZeroVector;
	}
	
	return AttachPoints.Last();
}

void FSwordPointData::AddAttachPoint(const FVector& LocalAttachPoint)
{
	if (AttachPoints.IsEmpty())
	{
		AttachPoints.Add(LocalAttachPoint);
	}
	else
	{
		AttachPoints.Add(LocalAttachPoint + AttachPoints.Last());
	}
}

FVector FProcCubicBezier::SamplePoint(float T)
{
	float A{1 - T};

	//formula taken from Wikipedia - https://en.wikipedia.org/wiki/B%C3%A9zier_curve
	FVector Point{
		A * A * A * PointA
		+ 3 * (A * A) * T * PointB
		+ 3 * A * (T * T) * PointC
		+ T * T * T * PointD};
	return Point;
}

FVector FProcCubicBezier::SamplePointNormal(float T, const FVector& OtherAxis, float NormalDistance)
{
	FVector PointNormal{GetNormal(T, OtherAxis)};
	return SamplePoint(T) + PointNormal;
}

TArray<FProcVertex> FProcCubicBezier::SamplePoints(int Samples)
{
	int TotalSamples{FMath::Clamp(Samples + 1, 0, INT_MAX)};
	float SampleSize{1.0f / (TotalSamples)};
	
	TArray<FProcVertex> SamplePoints;
	for (int i = 0; i <= TotalSamples; ++i)
	{
		float T{SampleSize * i};
		SamplePoints.Add(SamplePoint(T));
	}
	
	return SamplePoints;
}

TArray<FProcVertex> FProcCubicBezier::SamplePointsOffNormals(int Samples, const FVector& OtherAxis, float NormalDistance, float EndNormalDistance)
{
	float AdjustedNormalDistance{NormalDistance};
	
	int TotalSamples{FMath::Clamp(Samples + 1, 0, INT_MAX)};
	float SampleSize{1.0f / TotalSamples};
	
	TArray<FProcVertex> SamplePoints;
	for (int i = 0; i <= TotalSamples; ++i)
	{
		float T{SampleSize * i};
		FVector PointNormal{GetNormal(T, OtherAxis)};

		if (EndNormalDistance != -1)
		{
			AdjustedNormalDistance = FMath::Lerp(NormalDistance, EndNormalDistance, T);
		}
		
		SamplePoints.Add(SamplePoint(T) + PointNormal * AdjustedNormalDistance);
	}
	
	return SamplePoints;
}

TArray<FProcVertex> FProcCubicBezier::SamplePointsRotated(int Samples, const FVector& OtherAxis, float Angle, FVector PointMultiplier)
{
	TArray<FProcVertex> OutPoints;

	int TotalSamples{FMath::Clamp(Samples + 1, 0, INT_MAX)};
	float SampleSize{1.0f / TotalSamples};

	for (int i = 0; i <= TotalSamples; ++i)
	{
		float T{SampleSize * i};
		FVector SampledPoint{SamplePoint(T)};
		FVector RotatedPoint{UKismetMathLibrary::RotateAngleAxis(SampledPoint, Angle, OtherAxis)};

		//point multiplier is used to adjust the point after rotation, for example, compressing the points along one axis
		RotatedPoint *= PointMultiplier;
		
		OutPoints.Emplace(RotatedPoint);
	}

	return OutPoints;
}


FVector FProcCubicBezier::GetTangent(float T)
{
	float OMT{1 - T};

	FVector A{PointA};
	FVector B{PointB};
	FVector C{PointC};
	FVector D{PointD};

	//make zero to keep normals right
	A.X = 0;
	B.X = 0;
	C.X = 0;
	D.X = 0;

	// formula from wikipedia - https://en.wikipedia.org/wiki/B%C3%A9zier_curve
	FVector Derivative{
		3.0f * OMT * OMT * (B - A)
		+ 6.0f * OMT * T * (C - B)
		+ 3.0f * T * T * (D - C)
	};

	return Derivative;
}

FVector FProcCubicBezier::GetNormal(float T, const FVector& OtherAxis)
{
	FVector Tangent{GetTangent(T)};
	return FVector::CrossProduct(Tangent.GetSafeNormal(), OtherAxis);
}

void FProcCubicBezier::CreateLengthLUT()
{
	ArcLengths.Reset();
	int Steps{10};

	FVector Previous{PointA};
	ArcLengths.Add(0.0f);

	//creates a look up table with the total distance along the spline at certain intervals
	for (int i = 1; i < Steps; ++i)
	{
		float T{static_cast<float>(i) / (Steps - 1)};
		FVector Point{SamplePoint(T)};

		float Diff{static_cast<float>(FVector{Previous - Point}.Length())};
		TotalLength += Diff;

		ArcLengths.Add(TotalLength);
		Previous = Point;
		
	}
}

FVector FProcQuadraticBezier::SamplePoint(float T)
{
	FVector Point{(1 - T) * (1 - T) * PointA + 2 * (1 - T) * T * PointB + T * T * PointC};
	return Point;
}

FVector FProcQuadraticBezier::SampleNormalised(float T)
{
	float TDistance{T * TotalLength};
	float SectionT{1.0f / (ArcLengths.Num() - 1)};

	//uses arc length parameterisation
	//iterates through the table to find the index it sits between. once found, it then lerps between the two closest values to find the true distance along the curve
	for (int i = 0; i < ArcLengths.Num() - 1; ++i)
	{
		float StartT{SectionT * i};
		
		if (TDistance >= ArcLengths[i] && TDistance < ArcLengths[i + 1])
		{
			float BetweenAlpha{static_cast<float>(UKismetMathLibrary::NormalizeToRange(TDistance, ArcLengths[i], ArcLengths[i + 1]))};
			return SamplePoint(FMath::Lerp(StartT, StartT + SectionT, BetweenAlpha));
		}
	}
	
	return FVector{};
}

TArray<FProcVertex> FProcQuadraticBezier::SamplePoints(int Samples)
{
	Samples = FMath::Clamp(Samples, 0, INT_MAX);

	float SampleSize{1.0f / (Samples + 1)};

	
	
	TArray<FProcVertex> SamplePoints;
	SamplePoints.Emplace(PointA);
	
	for (int i = 1; i < Samples + 1; ++i)
	{
		float T{SampleSize * i};
		
		//quadratic bezier sample formula
		FVector Point{SampleNormalised(T)};
		SamplePoints.Emplace(Point);
	}

	SamplePoints.Emplace(PointC);

	return SamplePoints;
}

void FProcQuadraticBezier::CreateLengthLUT()
{
	//create a table with the total length of the curve at intervals along t
	//adapted from https://www.youtube.com/watch?v=o9RK6O2kOKo&t=2707s
	
	ArcLengths.Reset();
	int Steps{10};

	FVector Previous{PointA};
	ArcLengths.Add(0.0f);
	
	for (int i = 1; i < Steps; ++i)
	{
		float T{static_cast<float>(i) / (Steps - 1)};
		 FVector Point{SamplePoint(T)};

		float Diff{static_cast<float>(FVector{Previous - Point}.Length())};
		TotalLength += Diff;

		ArcLengths.Add(TotalLength);
		Previous = Point;
		
	}
 }

TArray<FProcVertex> FProcPolyLine::MirrorByPlane(const FPlane& Plane)
{
	TArray<FProcVertex> MirroredLine;
	
	if (Plane.IsValid())
	{
		for (auto& Point : Points)
		{
			FProcVertex Vertex{ UKismetMathLibrary::Vector_MirrorByPlane(Point.Location, Plane)};
			MirroredLine.Add(Vertex);
		}
	}
	
	return MirroredLine;
}

void FProcGenerationParameter::Clamp(TOptional<float> MinConstraint, TOptional<float> MaxConstraint)
{
	float AdjustedMax{MaxValue};
	float AdjustedMin{MinValue};
	
	if (MinConstraint.IsSet())
	{
		AdjustedMin = FMath::Max(MinConstraint.GetValue(), MinValue);
	}
	if (MaxConstraint.IsSet())
	{
		AdjustedMax = FMath::Min(MaxConstraint.GetValue(), MaxValue);
	}
	
	MainValue = FMath::Clamp(MainValue, AdjustedMin, AdjustedMax);
}

void FProcGenerationParameter::Randomise(TOptional<float> MinConstraint, TOptional<float> MaxConstraint)
{
	float AdjustedMin{MinValue};
	float AdjustedMax{MaxValue};

	if (MinConstraint.IsSet())
	{
		AdjustedMin = FMath::Max(MinConstraint.GetValue(), MinValue);
	}
	if (MaxConstraint.IsSet())
	{
		AdjustedMax = FMath::Min(MaxConstraint.GetValue(), MaxValue);
	}
	
	
	MainValue = UKismetMathLibrary::RandomFloatInRange(AdjustedMin, AdjustedMax);
}

FProcGenerationParameter::operator float() const
{
	return MainValue;
}

FProcGenerationParameter& FProcGenerationParameter::operator=(const float& F)
{
	MainValue = F;
	return *this;
}

void FProcCombinedGenParams::EnforceRules()
{
	//blade
	BladeGenParams.Length.Clamp();
	BladeGenParams.Height.Clamp();
	BladeGenParams.Thickness.Clamp();
	
	BladeGenParams.MiddleWidth.Clamp(NullOpt, {BladeGenParams.Height});

	BladeGenParams.TipTaperLength.Clamp(NullOpt, {BladeGenParams.Length});
	BladeGenParams.TipHeight.Clamp();
	BladeGenParams.TipCurve.Clamp();

	//guard
	GuardGenParams.Length.Clamp({BladeGenParams.Height});
	GuardGenParams.Width.Clamp({BladeGenParams.Thickness});
	GuardGenParams.Thickness.Clamp();

	GuardGenParams.CurveOffsetOuter.Clamp({-GuardGenParams.Length}, {GuardGenParams.Length});
	GuardGenParams.CurveOffsetInner.Clamp({-GuardGenParams.Length}, {GuardGenParams.Length});
	
	GuardGenParams.OppositeCurveOffsetOuter.Clamp({-GuardGenParams.Length}, {GuardGenParams.Length});
	GuardGenParams.OppositeCurveOffsetInner.Clamp({-GuardGenParams.Length}, {GuardGenParams.Length});

	GuardGenParams.WidthTaper.Clamp();
	GuardGenParams.InnerWidthTaper.Clamp();

	GuardGenParams.ThicknessTaper.Clamp();

	//grip
	GripGenParams.HandleWidthMultiplier.Clamp();
	
	GripGenParams.Length.Clamp();
	
	GripGenParams.GuardRadius.Clamp(NullOpt, {GuardGenParams.Width / GripGenParams.HandleWidthMultiplier / 2});
	GripGenParams.InnerGuardRadius.Clamp();
	GripGenParams.InnerPommelRadius.Clamp();
	GripGenParams.PommelRadius.Clamp();


	//pommel
	PommelGenParams.WidthMultiplier.Clamp();
	
	PommelGenParams.Length.Clamp();
	
	PommelGenParams.BottomRadius.Clamp();
	PommelGenParams.InnerBottomRadius.Clamp();
	PommelGenParams.InnerTopRadius.Clamp();
	PommelGenParams.TopRadius.Clamp({GripGenParams.PommelRadius});
}

void FProcCombinedGenParams::Randomise(EProcRandomiseType RandomiseType)
{
	if (RandomiseType == EProcRandomiseType::All || RandomiseType == EProcRandomiseType::Blade)
	{
		//blade
		BladeGenParams.Length.Randomise();
		BladeGenParams.Height.Randomise();
		BladeGenParams.Thickness.Randomise();
	
		BladeGenParams.MiddleWidth.Randomise(NullOpt, {BladeGenParams.Height});

		BladeGenParams.TipTaperLength.Randomise(NullOpt, {BladeGenParams.Length});
		BladeGenParams.TipHeight.Randomise();
		BladeGenParams.TipCurve.Randomise();
	}
	if (RandomiseType == EProcRandomiseType::All || RandomiseType == EProcRandomiseType::Guard)
	{
		//guard
		GuardGenParams.Length.Randomise({BladeGenParams.Height});
		GuardGenParams.Width.Randomise({BladeGenParams.Thickness});
		GuardGenParams.Thickness.Randomise();

		GuardGenParams.CurveOffsetOuter.Randomise({-GuardGenParams.Length}, {GuardGenParams.Length});
		GuardGenParams.CurveOffsetInner.Randomise({-GuardGenParams.Length}, {GuardGenParams.Length});

		GuardGenParams.bSymmetrical = UKismetMathLibrary::RandomBool();
	
		GuardGenParams.OppositeCurveOffsetOuter.Randomise({-GuardGenParams.Length}, {GuardGenParams.Length});
		GuardGenParams.OppositeCurveOffsetInner.Randomise({-GuardGenParams.Length}, {GuardGenParams.Length});

		GuardGenParams.WidthTaper.Randomise();
		GuardGenParams.InnerWidthTaper.Randomise();

		GuardGenParams.ThicknessTaper.Randomise();
	}
	if (RandomiseType == EProcRandomiseType::All || RandomiseType == EProcRandomiseType::Grip)
	{
		//grip
		GripGenParams.HandleWidthMultiplier.Randomise();
	
		GripGenParams.Length.Randomise();
	
		GripGenParams.GuardRadius.Randomise(NullOpt, {GuardGenParams.Width / GripGenParams.HandleWidthMultiplier / 2});
		GripGenParams.InnerGuardRadius.Randomise();
		GripGenParams.InnerPommelRadius.Randomise();
		GripGenParams.PommelRadius.Randomise();
	}
	if (RandomiseType == EProcRandomiseType::All || RandomiseType == EProcRandomiseType::Pommel)
	{
		//pommel
		PommelGenParams.WidthMultiplier.Randomise();
	
		PommelGenParams.Length.Randomise();
	
		PommelGenParams.BottomRadius.Randomise();
		PommelGenParams.InnerBottomRadius.Randomise();
		PommelGenParams.InnerTopRadius.Randomise();
		PommelGenParams.TopRadius.Randomise({GripGenParams.PommelRadius});
	}
}




