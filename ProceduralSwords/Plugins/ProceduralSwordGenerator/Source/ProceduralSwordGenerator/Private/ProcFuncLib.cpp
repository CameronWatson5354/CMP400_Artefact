// Fill out your copyright notice in the Description page of Project Settings.
#include "ProcFuncLib.h"
#include "Components/DynamicMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "DynamicMeshEditor.h"
#include "JsonObjectConverter.h"
#include "DynamicMesh/MeshTransforms.h"
#include "GeometryScript/MeshNormalsFunctions.h"
#include "GeometryScript/MeshQueryFunctions.h"
#include "Serialization/BufferWriter.h"

using namespace UE::Geometry;

//template function when needing to add any mesh modification functions
// TargetMesh->EditMesh([&](FDynamicMesh3& EditMesh)
// 		{
// 			
// 		});


UDynamicMesh* UProcFuncLib::AppendBladeMesh(UDynamicMesh* TargetMesh, FProcBladeGenParams& Params, FSwordPointData& PointData, FProcSwordAttributes& Attributes)
{
	//this function creates the blade mesh by first creating two polylines:
	// the outer polyline which serves as the top edge of the sword
	// the inner polyline which serves as the inner edge
	// these lines are then mirrored across axes and triangulated
	
	//creating top middle edge of the sword
	FDynamicMesh3 TempMesh;
	FPlane PlaneY{UKismetMathLibrary::MakePlaneFromPointAndNormal(FVector{0.0f, 0.0f, 0.0f}, FVector{0.0f, 0.f, 1.0f})};
	FPlane PlaneZ{UKismetMathLibrary::MakePlaneFromPointAndNormal(FVector::ZeroVector, FVector{-1.0f, 0.0f, 0.0f})};

	//three main points for the top outer blade
	FVector OuterStart{0.0f, 0.0f, Params.Height / 2};
	FVector OuterTipStart{0.0f, Params.Length - Params.TipTaperLength, Params.TipHeight / 2};
	FVector OuterTipEnd{0.0f, Params.Length, 0.0f};

	FVector OuterDirection{OuterTipStart - OuterStart};
	OuterDirection.Normalize();
	
	FVector FirstLineDirection{OuterTipStart - OuterStart};
	FirstLineDirection.Normalize();

	FVector OuterTipTaperDirection{OuterTipEnd - OuterTipStart};
	OuterTipTaperDirection.Normalize();
	
	//rotate the tip taper so I can use dot product to determine if the sword is concave or convex
	OuterTipTaperDirection = UKismetMathLibrary::RotateAngleAxis(OuterTipTaperDirection, 90, FVector::ForwardVector);
	
	//adjusts the point where taper curve begins by lerping between the start of the sword and where the tip starts
	FVector AdjustedOuterTipStart{FMath::Lerp(OuterTipStart, OuterStart, Params.TipCurve.MainValue)};
	FVector OuterControlPointEnd{0.0f, Params.Length, OuterTipStart.Z + OuterDirection.Z * Params.TipTaperLength};
	
	FProcPolyLine OuterEdgeLine;
	OuterEdgeLine.Points.Add(FProcVertex{FVector{0.0f, -50.0f, 0.0f}});
	OuterEdgeLine.Points.Add(FProcVertex{FVector{0.0f, -50.0f, Params.Height / 2}});

	FVector PressurePoint{};
	
	//if the main body of the blade and the tip make a convex shape
	float BladeShapeDot{static_cast<float>(FVector::DotProduct(FirstLineDirection, OuterTipTaperDirection))};
	if (BladeShapeDot >= 0.0f)
	{
		AdjustedOuterTipStart = OuterTipStart;
		SampleAlongLine(OuterStart, AdjustedOuterTipStart, OuterEdgeLine.Points, Params.SectionSteps);

		FProcQuadraticBezier OuterTaperCurve{OuterTipStart, FMath::Lerp(OuterTipStart, OuterControlPointEnd, Params.TipCurve.MainValue), OuterTipEnd};
		OuterEdgeLine.Points.Append(OuterTaperCurve.SamplePoints(Params.SectionSteps));
		
		PressurePoint.Z = OuterTaperCurve.SampleNormalised(0.95f).Z;

	}
	else
	{
		SampleAlongLine(OuterStart, AdjustedOuterTipStart, OuterEdgeLine.Points, Params.SectionSteps);

		FProcQuadraticBezier OuterTaperEdge{AdjustedOuterTipStart, OuterTipStart, OuterTipEnd};
		OuterEdgeLine.Points.Append(OuterTaperEdge.SamplePoints(Params.SectionSteps));

		PressurePoint.Z = OuterTaperEdge.SampleNormalised(0.95f).Z;
	}
	
	AppendPolyLineVertices(TempMesh, OuterEdgeLine.Points);


	
	//create middle of the sword
	float HeightDiff{Params.Height / 2 - Params.TipHeight / 2};
	
	FVector MiddleTopStart{-Params.Thickness / 2, 0.0f, Params.MiddleWidth / 2};
	FVector InnerTipStart{-Params.Thickness / 2, Params.Length - Params.TipTaperLength, Params.MiddleWidth / 2 - HeightDiff};
	InnerTipStart.Y = FMath::Clamp(InnerTipStart.Y, 0.0f, FLT_MAX);
	
	FVector InnerTipEnd{-Params.Thickness / 2, Params.Length - (Params.Height / 2 - Params.MiddleWidth / 2), 0.0f};
	InnerTipEnd.Y = FMath::Clamp(InnerTipEnd.Y, 0.0f, FLT_MAX);
	
	FVector OuterTipDirection{OuterTipStart - OuterTipEnd};
	OuterTipDirection.Normalize();

	FVector MiddleTopDirection{InnerTipStart - MiddleTopStart};
	MiddleTopDirection.Normalize();


	//try to find an intersection from the straight part of the inner sword and the final point so that the centre curve looks natural
	float T{0.0f};
	FVector InnerIntersection{InnerTipEnd};
	UKismetMathLibrary::LinePlaneIntersection_OriginNormal(MiddleTopStart, MiddleTopStart + MiddleTopDirection * Params.Length.MainValue, InnerTipEnd,
	                                                       FVector{0.0f, -OuterTipDirection.Z, OuterTipDirection.Y,}, T, InnerIntersection);

	//edge cases to ensure the intersection remains on a valid area of the sword
	if (InnerIntersection.Y > InnerTipEnd.Y || InnerIntersection.Z < 0.0f)
	{
		InnerIntersection = InnerTipEnd;
	}

	if (InnerIntersection.SquaredLength() == 0.0f)
	{
		InnerIntersection = InnerTipEnd;
	}

	FVector MiddleEdgeDirection{InnerIntersection - MiddleTopStart};
	MiddleEdgeDirection.Normalize();

	FVector MiddleControlPointEnd{InnerIntersection};

	float YDiff{static_cast<float>(FMath::Abs(InnerIntersection.Y - InnerTipEnd.Y))};

	MiddleControlPointEnd.Y = InnerTipEnd.Y;
	MiddleControlPointEnd.Z += MiddleEdgeDirection.Z * YDiff;

	
	FProcPolyLine FrontMiddleTop;
	FrontMiddleTop.Points.Add(FProcVertex{FVector{0.0f, -50.0f, 0.0f}});
	FrontMiddleTop.Points.Add(FProcVertex{FVector{-Params.Thickness / 2, -50.0f, Params.MiddleWidth / 2}});

	FVector AdjustedMiddleTopStart{FMath::Lerp(InnerTipStart, MiddleTopStart, Params.TipCurve.MainValue)};

	//is the shape of the inner edge is convex
	if (BladeShapeDot >= 0.0f)
	{
		SampleAlongLine(MiddleTopStart, InnerIntersection, FrontMiddleTop.Points, Params.SectionSteps);
		
		FProcQuadraticBezier MiddleTaperEdge{InnerIntersection, FMath::Lerp(InnerIntersection, MiddleControlPointEnd, Params.TipCurve.MainValue), InnerTipEnd};
		FrontMiddleTop.Points.Append(MiddleTaperEdge.SamplePoints(Params.SectionSteps));
		
		PressurePoint.Y = MiddleTaperEdge.SampleNormalised(0.95f).Y;
		PressurePoint.X = MiddleTaperEdge.SampleNormalised(0.95f).X;
	}
	else
	{
		SampleAlongLine(MiddleTopStart, AdjustedMiddleTopStart, FrontMiddleTop.Points, Params.SectionSteps);

		FProcQuadraticBezier OuterTaperEdge{AdjustedMiddleTopStart, InnerTipStart, InnerTipEnd};
		FrontMiddleTop.Points.Append(OuterTaperEdge.SamplePoints(Params.SectionSteps));

		PressurePoint.Y = OuterTaperEdge.SampleNormalised(0.95f).Y;
		PressurePoint.X = OuterTaperEdge.SampleNormalised(0.95f).X;
	}


	//fix middle lines going below the middle height of the sword
	for (auto& Point : FrontMiddleTop.Points)
	{
		Point.Location.Z = FMath::Max(Point.Location.Z, 0.0f);
	}
	AppendPolyLineVertices(TempMesh, FrontMiddleTop.Points);

	//mirror lines
	FProcPolyLine BottomEdgeOuter{OuterEdgeLine.MirrorByPlane(PlaneY)};
	AppendPolyLineVertices(TempMesh, BottomEdgeOuter.Points);
	
	FProcPolyLine FrontMiddleBottom{FrontMiddleTop.MirrorByPlane(PlaneY)};
	AppendPolyLineVertices(TempMesh, FrontMiddleBottom.Points);

	FProcPolyLine MiddlePointsOppositeA{FrontMiddleTop.MirrorByPlane(PlaneZ)};
	FProcPolyLine MiddlePointsOppositeB{FrontMiddleBottom.MirrorByPlane(PlaneZ)};
	AppendPolyLineVertices(TempMesh, MiddlePointsOppositeA.Points);
	AppendPolyLineVertices(TempMesh, MiddlePointsOppositeB.Points);

	//triangulate mesh
	TriangulatePolyLines(TempMesh, OuterEdgeLine.Points, FrontMiddleTop.Points);
	TriangulatePolyLines(TempMesh, FrontMiddleTop.Points, FrontMiddleBottom.Points);
	TriangulatePolyLines(TempMesh, FrontMiddleBottom.Points, BottomEdgeOuter.Points);
	
	TriangulatePolyLines(TempMesh, OuterEdgeLine.Points, MiddlePointsOppositeA.Points, true);
	TriangulatePolyLines(TempMesh, MiddlePointsOppositeA.Points, MiddlePointsOppositeB.Points, true);
	TriangulatePolyLines(TempMesh, MiddlePointsOppositeB.Points, BottomEdgeOuter.Points, true);

	for (int i = 0; i < TempMesh.TriangleCount(); ++i)
	{
		TempMesh.SetVertexUV(i, FVector2f{1.0f, 1.0f});
	}

	MeshTransforms::Translate(TempMesh, PointData.AttachPoint);
	SetMaterialIDAllTriangles(TempMesh, Params.TextureIndex);

	AppendMesh(TargetMesh, TempMesh);
	
	//attributes
	Attributes.HitboxOffset = PointData.AttachPoint.Y + Params.Length / 2.0f;
	Attributes.HitboxWidth = FMath::Max(Params.Height, Params.TipHeight);
	Attributes.BladeLength = Params.Length;
	Attributes.TotalLength += Params.Length;

	float SharpnessDot{0.0f};
	
	//calculate blade sharpness by going along the blade and sampling points
	for (int i = 1; i < 1 + Params.SectionSteps; ++i)
	{
		FVector A{FrontMiddleTop.Points[i].Location};
		FVector B{OuterEdgeLine.Points[i].Location};
		FVector C{MiddlePointsOppositeA.Points[i].Location};

		FVector BA{A - B};
		FVector BC{C - B};
		BA.Normalize();
		BC.Normalize();

		SharpnessDot += static_cast<float>(FVector::DotProduct(BA, BC));
	}
	
	SharpnessDot /= 1 + Params.SectionSteps;
	Attributes.BladeSharpness = UKismetMathLibrary::NormalizeToRange(SharpnessDot, -1.0f, 1.0f);
	
	//calculate tip sharpness and pressure using dot product of the points at the tip of the blade
	FVector A{OuterEdgeLine.Points[OuterEdgeLine.Points.Num() - 2].Location};
	FVector B{OuterEdgeLine.Points[OuterEdgeLine.Points.Num() - 1].Location};
	FVector C{BottomEdgeOuter.Points[BottomEdgeOuter.Points.Num() - 2].Location};

	FVector BA{A - B};
	FVector BC{C - B};
	BA.Normalize();
	BC.Normalize();
	
	float Length{static_cast<float>(FMath::Abs(PressurePoint.X * 2))};
	float Height{static_cast<float>(FMath::Abs(PressurePoint.Z * 2))};
	double TipArea{Length * Height * 0.0001f};
	Attributes.TipPressure = static_cast<float>((100.0f / TipArea) * 0.000001);

	SharpnessDot = FVector::DotProduct(BA, BC);
	Attributes.TipSharpness = UKismetMathLibrary::NormalizeToRange(SharpnessDot, -1.0f, 1.0f);
	
	return TargetMesh;
}

UDynamicMesh* UProcFuncLib::AppendGuardMesh(UDynamicMesh* TargetMesh, FProcGuardGenParams& Params, FSwordPointData& PointData, FProcSwordAttributes& Attributes)
{
	if (!IsValid(TargetMesh))
	{
		return TargetMesh;
	}

	FPlane PlaneY{UKismetMathLibrary::MakePlaneFromPointAndNormal(FVector::ZeroVector, FVector{0.0f, 0.f, 1.0f})};
	FPlane PlaneZ{UKismetMathLibrary::MakePlaneFromPointAndNormal(FVector::ZeroVector, FVector{-1.0f, 0.0f, 0.0f})};

	FDynamicMesh3 TempMesh;

	//height period is used to ensure the bezier points are equally spaces along the horizontal axis
	float HeightPeriod{Params.Length / 2.0f / 3.0f};

	//this bezier makes a centreline and points are offset to make the top and bottom edges of the guard
	FProcCubicBezier FrontCentreLine{};
	FrontCentreLine.PointA = FVector{-Params.Width / 2, 0.0f, 0.0f};
	FrontCentreLine.PointB = FVector{-Params.InnerWidthTaper / 2, 0.0f, -HeightPeriod};
	FrontCentreLine.PointC = FVector{-Params.InnerWidthTaper / 2, Params.CurveOffsetInner, -HeightPeriod * 2};
	FrontCentreLine.PointD = FVector{-Params.WidthTaper / 2, Params.CurveOffsetOuter, -Params.Length / 2.0f};

	FProcPolyLine UpperEdge;
	FProcPolyLine LowerEdge;

	//determines the end thickness based on a value between 0 and 1
	float NormalisedThicknessTaper{FMath::Lerp(Params.Thickness / 2, -Params.Thickness / 2, Params.ThicknessTaper.MainValue)};

	UpperEdge.Points.Append(FrontCentreLine.SamplePointsOffNormals(Params.SectionSteps, -FVector::ForwardVector, Params.Thickness / 2));
	LowerEdge.Points.Append(FrontCentreLine.SamplePointsOffNormals(Params.SectionSteps, FVector::ForwardVector, Params.Thickness / 2, NormalisedThicknessTaper));

	FProcPolyLine OppositeTopOuterLine;
	FProcPolyLine OppositeBottomOuterLine;

	//if the guard is symmetrical, it can simple be mirrored 
	if (Params.bSymmetrical)
	{
		OppositeTopOuterLine.Points.Append(UpperEdge.MirrorByPlane(PlaneY));
		OppositeBottomOuterLine.Points.Append(LowerEdge.MirrorByPlane(PlaneY));
	}
	else
	{
		//similar process to the previous edge
		FProcCubicBezier OppositeTopOuter{};
		OppositeTopOuter.PointA = FVector{-Params.Width / 2, 0.0f, 0.0f};
		OppositeTopOuter.PointB = FVector{-Params.InnerWidthTaper / 2, 0.0f, HeightPeriod};
		OppositeTopOuter.PointC = FVector{-Params.InnerWidthTaper / 2, Params.OppositeCurveOffsetInner, HeightPeriod * 2};
		OppositeTopOuter.PointD = FVector{-Params.WidthTaper / 2, Params.OppositeCurveOffsetOuter, Params.Length / 2};

		OppositeTopOuterLine.Points.Append(OppositeTopOuter.SamplePointsOffNormals(Params.SectionSteps, FVector::ForwardVector, Params.Thickness / 2));
		OppositeBottomOuterLine.Points.Append(OppositeTopOuter.SamplePointsOffNormals(Params.SectionSteps, -FVector::ForwardVector, Params.Thickness / 2, NormalisedThicknessTaper));
	}

	//prepend the opposite points so that only two lines have to be appended
	for (auto& Point : OppositeTopOuterLine.Points)
	{
		UpperEdge.Points.EmplaceAt(0, Point);
	}

	for (auto& Point : OppositeBottomOuterLine.Points)
	{
		LowerEdge.Points.EmplaceAt(0, Point);
	}
	
	OppositeTopOuterLine.Points.Empty();
	OppositeBottomOuterLine.Points.Empty();

	OppositeTopOuterLine.Points.Append(UpperEdge.MirrorByPlane(PlaneZ));
	OppositeBottomOuterLine.Points.Append(LowerEdge.MirrorByPlane(PlaneZ));
	AppendPolyLineVertices(TempMesh, UpperEdge.Points);
	AppendPolyLineVertices(TempMesh, LowerEdge.Points);
	AppendPolyLineVertices(TempMesh, OppositeTopOuterLine.Points);
	AppendPolyLineVertices(TempMesh, OppositeBottomOuterLine.Points);

	
	TriangulatePolyLines(TempMesh, UpperEdge.Points, LowerEdge.Points);
	TriangulatePolyLines(TempMesh, OppositeTopOuterLine.Points, OppositeBottomOuterLine.Points, true);

	TriangulatePolyLines(TempMesh, OppositeTopOuterLine.Points, UpperEdge.Points);
	TriangulatePolyLines(TempMesh, LowerEdge.Points, OppositeBottomOuterLine.Points);

	//make a face for the ends of the guard so there isnt a hole in the mesh
	FProcPolyLine TopCapA;
	TopCapA.Points.Add(OppositeTopOuterLine.Points[0].Location);
	TopCapA.Points.Add(UpperEdge.Points[0].Location);

	FProcPolyLine TopCapB;
	TopCapB.Points.Add(OppositeBottomOuterLine.Points[0].Location);
	TopCapB.Points.Add(LowerEdge.Points[0].Location);

	AppendPolyLineVertices(TempMesh, TopCapA.Points);
	AppendPolyLineVertices(TempMesh, TopCapB.Points);
	TriangulatePolyLines(TempMesh, TopCapA.Points, TopCapB.Points);
	
	FProcPolyLine BottomCapA;
	BottomCapA.Points.Add(OppositeTopOuterLine.Points.Last().Location);
	BottomCapA.Points.Add(UpperEdge.Points.Last().Location);

	FProcPolyLine BottomCapB;
	BottomCapB.Points.Add(OppositeBottomOuterLine.Points.Last().Location);
	BottomCapB.Points.Add(LowerEdge.Points.Last().Location);

	AppendPolyLineVertices(TempMesh, BottomCapA.Points);
	AppendPolyLineVertices(TempMesh, BottomCapB.Points);

	TriangulatePolyLines(TempMesh, BottomCapA.Points, BottomCapB.Points, true);


	//post process
	for (int i = 0; i < TempMesh.TriangleCount(); ++i)
	{
		TempMesh.SetVertexUV(i, FVector2f{1.0f, 1.0f});
	}

	MeshTransforms::Translate(TempMesh, FVector{0.0f, -Params.Thickness / 2, 0.0f} + PointData.GetAttachPoint());
	PointData.AddAttachPoint(FVector{0.0f, -Params.Thickness, 0.0f});
	
	SetMaterialIDAllTriangles(TempMesh, Params.TextureIndex);
	AppendMesh(TargetMesh, TempMesh);
	
	PointData.AttachPoint = PointData.AttachPoint + FVector{0.0f, Params.Thickness, 0.0f};
	Attributes.TotalLength += Params.Thickness;

	return TargetMesh;
}

UDynamicMesh* UProcFuncLib::AppendGripMesh(UDynamicMesh* TargetMesh, FProcGripGenParams& Params, FSwordPointData& PointData, FProcSwordAttributes& Attributes)
{
	if (!IsValid(TargetMesh))
	{
		return TargetMesh;
	}

	FDynamicMesh3 TempMesh;
	TArray<FProcPolyLine> PolylineCylinder;

	
	float LengthSection{Params.Length / 3.0f};

	// a bezier curve is used to make the linear profile of the grip mesh.
	// this profile is then rotated around and sampled to create the final mesh
	FProcCubicBezier TopOuter{};
	TopOuter.PointA = FVector{0.0f, 0.0f, Params.PommelRadius};
	TopOuter.PointB = FVector{0.0f, LengthSection, Params.InnerPommelRadius};
	TopOuter.PointC = FVector{0.0f, LengthSection * 2, Params.InnerGuardRadius};
	TopOuter.PointD = FVector{0.0f, Params.Length, Params.GuardRadius};

	float AngleSection{360.0f / Params.RadialSections};

	//rotate line around several times
	for (int i = 0; i < Params.RadialSections; ++i)
	{
		FProcPolyLine PolyLine;

		PolyLine.Points.Emplace(FVector{0.0f, 0.0f, 0.0f});
		PolyLine.Points.Append(TopOuter.SamplePointsRotated(Params.SectionSteps, FVector::RightVector, -AngleSection * i, FVector{Params.HandleWidthMultiplier, 1.0f, 1.0f}));
		PolyLine.Points.Emplace(FVector{0.0f, Params.Length + 50.0f, Params.GuardRadius}); //point to extend the overall length if the guard curves upwards
		PolyLine.Points.Emplace(FVector{0.0f, Params.Length, 0.0f});

		AppendPolyLineVertices(TempMesh, PolyLine.Points);
		PolylineCylinder.Add(PolyLine);
	}

	//triangulate the last polylines
	if (Params.RadialSections >= 2)
	{
		for (int i = 0; i < PolylineCylinder.Num() - 1; ++i)
		{
			TriangulatePolyLines(TempMesh, PolylineCylinder[i].Points, PolylineCylinder[i + 1].Points);
		}

		TriangulatePolyLines(TempMesh, PolylineCylinder.Last().Points, PolylineCylinder[0].Points);
	}


	//post process
	for (int i = 0; i < TempMesh.TriangleCount(); ++i)
	{
		TempMesh.SetVertexUV(i, FVector2f{1.0f, 1.0f});
	}

	PointData.AddAttachPoint(FVector{0.0f, -Params.Length, 0.0f});
	MeshTransforms::Translate(TempMesh, PointData.GetAttachPoint());
	
	SetMaterialIDAllTriangles(TempMesh, Params.TextureIndex);
	AppendMesh(TargetMesh, TempMesh);
	
	Attributes.TotalLength += Params.Length;
	Attributes.GripLength = Params.Length;

	return TargetMesh;
}

UDynamicMesh* UProcFuncLib::AppendPommelMesh(UDynamicMesh* TargetMesh, FProcPommelGenParams& Params, FSwordPointData& PointData, FProcSwordAttributes& Attributes)
{
	if (!IsValid(TargetMesh))
	{
		return TargetMesh;
	}

	FDynamicMesh3 TempMesh;

	TArray<FProcPolyLine> PolylineCylinder;

	float LengthSection{Params.Length / 3.0f};

	FProcCubicBezier TopOuter{};
	TopOuter.PointA = FVector{0.0f, 0.0f, Params.BottomRadius};
	TopOuter.PointB = FVector{0.0f, LengthSection, Params.InnerBottomRadius};
	TopOuter.PointC = FVector{0.0f, LengthSection * 2, Params.InnerTopRadius};
	TopOuter.PointD = FVector{0.0f, Params.Length, Params.TopRadius};

	float AngleSection{360.0f / Params.RadialSections};

	for (int i = 0; i < Params.RadialSections; ++i)
	{
		FProcPolyLine PolyLine;

		PolyLine.Points.Emplace(FVector{0.0f, 0.0f, 0.0f});
		PolyLine.Points.Append(TopOuter.SamplePointsRotated(Params.SectionSteps, FVector::RightVector, -AngleSection * i,
		                                                    FVector{Params.WidthMultiplier, 1.0f, 1.0f}));
		PolyLine.Points.Emplace(FVector{0.0f, Params.Length, 0.0f});

		AppendPolyLineVertices(TempMesh, PolyLine.Points);
		PolylineCylinder.Add(PolyLine);
	}

	if (Params.RadialSections >= 2)
	{
		for (int i = 0; i < PolylineCylinder.Num() - 1; ++i)
		{
			TriangulatePolyLines(TempMesh, PolylineCylinder[i].Points, PolylineCylinder[i + 1].Points);
		}

		TriangulatePolyLines(TempMesh, PolylineCylinder.Last().Points, PolylineCylinder[0].Points);
	}

	//post process
	for (int i = 0; i < TempMesh.TriangleCount(); ++i)
	{
		TempMesh.SetVertexUV(i, FVector2f{1.0f, 1.0f});
	}

	PointData.AddAttachPoint(FVector{0.0f, -Params.Length, 0.0f});
	MeshTransforms::Translate(TempMesh, PointData.GetAttachPoint());
	
	SetMaterialIDAllTriangles(TempMesh, Params.TextureIndex);
	AppendMesh(TargetMesh, TempMesh);

	PointData.AttachPoint = PointData.AttachPoint + FVector{0.0f, Params.Length, 0.0f};
	Attributes.TotalLength += Params.Length;
	
	return TargetMesh;
}

UDynamicMesh* UProcFuncLib::AppendGuardSubtractionMesh(UDynamicMesh* TargetMesh, FProcGuardGenParams& Params)
{
	//this generates a guard using the same method as above, but extends edges to the extreme to use as part of a boolean preventing parts clipping through each other
	
	if (!IsValid(TargetMesh))
	{
		return TargetMesh;
	}

	FPlane PlaneY{UKismetMathLibrary::MakePlaneFromPointAndNormal(FVector::ZeroVector, FVector{0.0f, 0.f, 1.0f})};
	FPlane PlaneZ{UKismetMathLibrary::MakePlaneFromPointAndNormal(FVector::ZeroVector, FVector{-1.0f, 0.0f, 0.0f})};

	FDynamicMesh3 TempMesh;


	float HeightPeriod{Params.Length / 2.0f / 3.0f};

	FProcCubicBezier TopOuter{};
	TopOuter.PointA = FVector{-50.0f, 0.0f, 0.0f};
	TopOuter.PointB = FVector{-50.0f, 0.0f, -HeightPeriod};
	TopOuter.PointC = FVector{-50.0f, Params.CurveOffsetInner, -HeightPeriod * 2};
	TopOuter.PointD = FVector{-50.0f, Params.CurveOffsetOuter, -Params.Length / 2.0f};

	FProcPolyLine TopOuterLine;
	FProcPolyLine BottomOuterLine;

	float NormalisedThicknessTaper{FMath::Lerp(Params.Thickness / 2, -Params.Thickness / 2, Params.ThicknessTaper.MainValue)};

	TopOuterLine.Points.Append(TopOuter.SamplePointsOffNormals(Params.SectionSteps, -FVector::ForwardVector, Params.Thickness / 2));
	TopOuterLine.Points.Emplace(TopOuter.SamplePointNormal(1.0f, FVector::ForwardVector, NormalisedThicknessTaper));

	FVector BottomStart{-50.0f, -100.0f, 0.0f};
	FVector BottomEnd{-50.0f, -100.0f, -Params.Length / 2};
	SampleAlongLine(BottomStart, BottomEnd, BottomOuterLine.Points, Params.SectionSteps);
	BottomOuterLine.Points.Add({BottomOuterLine.Points.Last().Location});
	

	FProcPolyLine OppositeTopOuterLine;
	FProcPolyLine OppositeBottomOuterLine;

	if (Params.bSymmetrical)
	{
		OppositeTopOuterLine.Points.Append(TopOuterLine.MirrorByPlane(PlaneY));
		OppositeBottomOuterLine.Points.Append(BottomOuterLine.MirrorByPlane(PlaneY));
	}
	else
	{
		FProcCubicBezier OppositeTopOuter{};
		OppositeTopOuter.PointA = FVector{-50.0f, 0.0f, 0.0f};
		OppositeTopOuter.PointB = FVector{-50.0f, 0.0f, HeightPeriod};
		OppositeTopOuter.PointC = FVector{-50.0f, Params.OppositeCurveOffsetInner, HeightPeriod * 2};
		OppositeTopOuter.PointD = FVector{-50.0f, Params.OppositeCurveOffsetOuter, Params.Length / 2};

		OppositeTopOuterLine.Points.Append(OppositeTopOuter.SamplePointsOffNormals(Params.SectionSteps, FVector::ForwardVector, Params.Thickness / 2));
		OppositeBottomOuterLine.Points.Append(BottomOuterLine.MirrorByPlane(PlaneY));
	}

	//prepend the opposite points so i only have to triangulate two lines
	for (auto& Point : OppositeTopOuterLine.Points)
	{
		TopOuterLine.Points.EmplaceAt(0, Point);
	}

	for (auto& Point : OppositeBottomOuterLine.Points)
	{
		BottomOuterLine.Points.EmplaceAt(0, Point);
	}


	OppositeTopOuterLine.Points.Empty();
	OppositeBottomOuterLine.Points.Empty();

	OppositeTopOuterLine.Points.Append(TopOuterLine.MirrorByPlane(PlaneZ));
	OppositeBottomOuterLine.Points.Append(BottomOuterLine.MirrorByPlane(PlaneZ));

	AppendPolyLineVertices(TempMesh, TopOuterLine.Points);
	AppendPolyLineVertices(TempMesh, BottomOuterLine.Points);
	AppendPolyLineVertices(TempMesh, OppositeTopOuterLine.Points);
	AppendPolyLineVertices(TempMesh, OppositeBottomOuterLine.Points);

	TriangulatePolyLines(TempMesh, TopOuterLine.Points, BottomOuterLine.Points);
	TriangulatePolyLines(TempMesh, OppositeTopOuterLine.Points, OppositeBottomOuterLine.Points, true);

	TriangulatePolyLines(TempMesh, OppositeTopOuterLine.Points, TopOuterLine.Points);
	TriangulatePolyLines(TempMesh, BottomOuterLine.Points, OppositeBottomOuterLine.Points);

	//top cap
	FProcPolyLine TopCapA;
	TopCapA.Points.Add(OppositeTopOuterLine.Points[0].Location);
	TopCapA.Points.Add(TopOuterLine.Points[0].Location);

	FProcPolyLine TopCapB;
	TopCapB.Points.Add(OppositeBottomOuterLine.Points[0].Location);
	TopCapB.Points.Add(BottomOuterLine.Points[0].Location);

	AppendPolyLineVertices(TempMesh, TopCapA.Points);
	AppendPolyLineVertices(TempMesh, TopCapB.Points);
	TriangulatePolyLines(TempMesh, TopCapA.Points, TopCapB.Points);

	//bottom cap
	FProcPolyLine BottomCapA;
	BottomCapA.Points.Add(OppositeTopOuterLine.Points.Last().Location);
	BottomCapA.Points.Add(TopOuterLine.Points.Last().Location);

	FProcPolyLine BottomCapB;
	BottomCapB.Points.Add(OppositeBottomOuterLine.Points.Last().Location);
	BottomCapB.Points.Add(BottomOuterLine.Points.Last().Location);

	AppendPolyLineVertices(TempMesh, BottomCapA.Points);
	AppendPolyLineVertices(TempMesh, BottomCapB.Points);

	TriangulatePolyLines(TempMesh, BottomCapA.Points, BottomCapB.Points, true);
	AppendMesh(TargetMesh, TempMesh);
	
	return TargetMesh;
}

UDynamicMesh* UProcFuncLib::AppendGripSubtractionMesh(UDynamicMesh* TargetMesh, FProcGuardGenParams& Params)
{
	if (!IsValid(TargetMesh))
	{
		return TargetMesh;
	}

	FPlane PlaneX{UKismetMathLibrary::MakePlaneFromPointAndNormal(FVector::ZeroVector, FVector{0.0f, 1.f, 0.0f})};
	FPlane PlaneY{UKismetMathLibrary::MakePlaneFromPointAndNormal(FVector::ZeroVector, FVector{0.0f, 0.f, 1.0f})};
	FPlane PlaneZ{UKismetMathLibrary::MakePlaneFromPointAndNormal(FVector::ZeroVector, FVector{-1.0f, 0.0f, 0.0f})};

	FDynamicMesh3 TempMesh;


	float HeightPeriod{Params.Length / 2.0f / 3.0f};

	FProcCubicBezier TopOuter{};
	TopOuter.PointA = FVector{-50.0f, 0.0f, 0.0f};
	TopOuter.PointB = FVector{-50.0f, 0.0f, -HeightPeriod};
	TopOuter.PointC = FVector{-50.0f, Params.CurveOffsetInner, -HeightPeriod * 2};
	TopOuter.PointD = FVector{-50.0f, Params.CurveOffsetOuter, -Params.Length / 2.0f};

	FProcPolyLine TopOuterLine;
	FProcPolyLine BottomOuterLine;

	float NormalisedThicknessTaper{FMath::Lerp(Params.Thickness / 2, -Params.Thickness / 2, Params.ThicknessTaper.MainValue)};

	FVector TopStart{-50.0f, 100.0f, 0.0f};
	FVector TopEnd{-50.0f, 100.0f, -Params.Length / 2};
	SampleAlongLine(TopStart, TopEnd, TopOuterLine.Points, Params.SectionSteps);
	
	//TopOuterLine.Points.Append(TopOuter.SamplePointsNormals(Params.SectionSteps, -FVector::ForwardVector, Params.Thickness / 2));
	BottomOuterLine.Points.Append(TopOuter.SamplePointsOffNormals(Params.SectionSteps, FVector::ForwardVector, Params.Thickness / 2, NormalisedThicknessTaper));

	FProcPolyLine OppositeTopOuterLine;
	FProcPolyLine OppositeBottomOuterLine;

	if (Params.bSymmetrical)
	{
		OppositeTopOuterLine.Points.Append(TopOuterLine.MirrorByPlane(PlaneY));
		OppositeBottomOuterLine.Points.Append(BottomOuterLine.MirrorByPlane(PlaneY));
	}
	else
	{
		FProcCubicBezier OppositeTopOuter{};
		OppositeTopOuter.PointA = FVector{-Params.Width / 2, 0.0f, 0.0f};
		OppositeTopOuter.PointB = FVector{-Params.InnerWidthTaper / 2, 0.0f, HeightPeriod};
		OppositeTopOuter.PointC = FVector{-Params.InnerWidthTaper / 2, Params.OppositeCurveOffsetInner, HeightPeriod * 2};
		OppositeTopOuter.PointD = FVector{-Params.WidthTaper / 2, Params.OppositeCurveOffsetOuter, Params.Length / 2};

		//OppositeTopOuterLine.Points.Append(OppositeTopOuter.SamplePointsNormals(Params.SectionSteps, FVector::ForwardVector, Params.Thickness / 2));

		OppositeTopOuterLine.Points.Append(TopOuterLine.MirrorByPlane(PlaneY));
		OppositeBottomOuterLine.Points.Append(OppositeTopOuter.SamplePointsOffNormals(Params.SectionSteps, -FVector::ForwardVector, Params.Thickness / 2, NormalisedThicknessTaper));
	}

	//prepend the opposite points so i only have to triangulate two lines
	for (auto& Point : OppositeTopOuterLine.Points)
	{
		TopOuterLine.Points.EmplaceAt(0, Point);
	}

	for (auto& Point : OppositeBottomOuterLine.Points)
	{
		BottomOuterLine.Points.EmplaceAt(0, Point);
	}


	OppositeTopOuterLine.Points.Empty();
	OppositeBottomOuterLine.Points.Empty();

	OppositeTopOuterLine.Points.Append(TopOuterLine.MirrorByPlane(PlaneZ));
	OppositeBottomOuterLine.Points.Append(BottomOuterLine.MirrorByPlane(PlaneZ));

	AppendPolyLineVertices(TempMesh, TopOuterLine.Points);
	AppendPolyLineVertices(TempMesh, BottomOuterLine.Points);
	AppendPolyLineVertices(TempMesh, OppositeTopOuterLine.Points);
	AppendPolyLineVertices(TempMesh, OppositeBottomOuterLine.Points);

	TriangulatePolyLines(TempMesh, TopOuterLine.Points, BottomOuterLine.Points);
	TriangulatePolyLines(TempMesh, OppositeTopOuterLine.Points, OppositeBottomOuterLine.Points, true);

	TriangulatePolyLines(TempMesh, OppositeTopOuterLine.Points, TopOuterLine.Points);
	TriangulatePolyLines(TempMesh, BottomOuterLine.Points, OppositeBottomOuterLine.Points);

	//top cap
	FProcPolyLine TopCapA;
	TopCapA.Points.Add(OppositeTopOuterLine.Points[0].Location);
	TopCapA.Points.Add(TopOuterLine.Points[0].Location);

	FProcPolyLine TopCapB;
	TopCapB.Points.Add(OppositeBottomOuterLine.Points[0].Location);
	TopCapB.Points.Add(BottomOuterLine.Points[0].Location);

	AppendPolyLineVertices(TempMesh, TopCapA.Points);
	AppendPolyLineVertices(TempMesh, TopCapB.Points);
	TriangulatePolyLines(TempMesh, TopCapA.Points, TopCapB.Points);

	//bottom cap
	FProcPolyLine BottomCapA;
	BottomCapA.Points.Add(OppositeTopOuterLine.Points.Last().Location);
	BottomCapA.Points.Add(TopOuterLine.Points.Last().Location);

	FProcPolyLine BottomCapB;
	BottomCapB.Points.Add(OppositeBottomOuterLine.Points.Last().Location);
	BottomCapB.Points.Add(BottomOuterLine.Points.Last().Location);

	AppendPolyLineVertices(TempMesh, BottomCapA.Points);
	AppendPolyLineVertices(TempMesh, BottomCapB.Points);

	TriangulatePolyLines(TempMesh, BottomCapA.Points, BottomCapB.Points, true);
	AppendMesh(TargetMesh, TempMesh);
	
	return TargetMesh;
}

FString UProcFuncLib::ReadStringFromFile(FString FilePath, bool& bOutSuccess, FString& OutInfoMessage)
{
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists((*FilePath)))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Read string from file failed - File doesn't exist. - %s"), *FilePath);
		return "";
	}

	FString ReturnString{""};

	if (!FFileHelper::LoadFileToString(ReturnString, *FilePath))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Read string from file failed - Was not able to read file. - %s"), *FilePath);
		return "";
	}

	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Read string from file succeeded - %s"), *FilePath);
	return ReturnString;
}

void UProcFuncLib::WriteStringToFile(FString FilePath, FString TextToWrite, bool& bOutSuccess, FString& OutInfoMessage)
{
	if (!FFileHelper::SaveStringToFile(TextToWrite, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Write string to file failed - Was not able to write to file - %s"), *FilePath);
		return;
	}

	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Write string to file succeeded - %s"), *FilePath);
}

TSharedPtr<FJsonObject> UProcFuncLib::ReadJson(FString FilePath, bool& bOutSuccess, FString& OutInfoMessage)
{
	FString JsonString{ReadStringFromFile(FilePath, bOutSuccess, OutInfoMessage)};
	if (!bOutSuccess)
	{
		return nullptr;
	}
	
	TSharedPtr<FJsonObject> ReturnJsonObject;
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(JsonString), ReturnJsonObject))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Read json failed - Was not able to deserialise the json string. = %s"), *JsonString);
		return nullptr;
	}

	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Read json succeeded - %s"), *FilePath);
	return ReturnJsonObject;
}

void UProcFuncLib::WriteJson(FString FilePath, TSharedPtr<FJsonObject> JsonObject, bool& bOutSuccess, FString& OutInfoMessage)
{
	FString JsonString{};

	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), TJsonWriterFactory<>::Create(&JsonString, 0)))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Write json failed - Was not able to serialise the json to string)"));
		return;
	}

	JsonString.Append(",\n");

	WriteStringToFile(FilePath, JsonString, bOutSuccess, OutInfoMessage);
	if (!bOutSuccess)
	{
		return;
	}

	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Write json succeeded - %s"), *FilePath);
}

void UProcFuncLib::WriteStructToJsonFile(FString FilePath, FProcSwordAttributes Struct, bool& bOutSuccess, FString& OutInfoMessage)
{
	TSharedPtr<FJsonObject> JsonObject{FJsonObjectConverter::UStructToJsonObject(Struct)};
	if (JsonObject == nullptr)
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Write struct json failed - Was not able to convert struct to json object."));
		return;
	}
	
	WriteJson(FilePath, JsonObject, bOutSuccess, OutInfoMessage);
}



void UProcFuncLib::AppendMesh(UDynamicMesh* TargetMesh, const FDynamicMesh3& AppendMesh)
{
	if (!IsValid(TargetMesh))
	{
		return;
	}

	if (TargetMesh->IsEmpty())
	{
		TargetMesh->EditMesh([&](FDynamicMesh3& EditMesh)
		{
			EditMesh.Copy(AppendMesh);
		});
	}
	else
	{
		TargetMesh->EditMesh([&](FDynamicMesh3& EditMesh)
		{
			FMeshIndexMappings TmpMappings;
			FDynamicMeshEditor Editor(&EditMesh);
			Editor.AppendMesh(&AppendMesh, TmpMappings);
		});
	}
}

void UProcFuncLib::SetMaterialIDAllTriangles(FDynamicMesh3& Mesh, int MaterialIndex)
{
	//iterates through all triangles and sets material index
	if (!Mesh.HasAttributes())
	{
		Mesh.EnableAttributes();
		Mesh.Attributes()->EnableMaterialID();
	}

	FDynamicMeshMaterialAttribute* MaterialIDs{Mesh.Attributes()->GetMaterialID()};
	for (int32 TriangleID : Mesh.TriangleIndicesItr())
	{
		MaterialIDs->SetValue(TriangleID, MaterialIndex);
	}
}

void UProcFuncLib::SampleAlongLine(const FVector& Start, const FVector& End, TArray<FProcVertex>& Points, int Samples)
{
	Samples = FMath::Clamp(Samples, 1, 100);

	float SampleSize{1.0f / Samples};

	Points.Emplace(Start);

	for (int i = 1; i < Samples + 1; ++i)
	{
		FProcVertex Point{FMath::Lerp(Start, End, SampleSize * i)};
		Points.Add(Point);
	}

	Points.Emplace(End);
}

void UProcFuncLib::AppendPolyLineVertices(FDynamicMesh3& Mesh, TArray<FProcVertex>& PolyLine)
{
	for (auto& Vertex : PolyLine)
	{
		Vertex.Index = Mesh.AppendVertex(Vertex.Location);
	}
}

void UProcFuncLib::TriangulatePolyLines(FDynamicMesh3& Mesh, const TArray<FProcVertex>& PolyLineA, const TArray<FProcVertex>& PolyLineB, bool bReverse)
{
	for (int i = 0; i < PolyLineA.Num() - 1; ++i)
	{
		if (PolyLineB.IsValidIndex(i) && PolyLineB.IsValidIndex(i + 1))
		{
			if (bReverse)
			{
				Mesh.AppendTriangle(PolyLineA[i].Index, PolyLineA[i + 1].Index, PolyLineB[i].Index);
				Mesh.AppendTriangle(PolyLineB[i + 1].Index, PolyLineB[i].Index, PolyLineA[i + 1].Index);
			}
			else
			{
				Mesh.AppendTriangle(PolyLineA[i + 1].Index, PolyLineA[i].Index, PolyLineB[i].Index);
				Mesh.AppendTriangle(PolyLineB[i].Index, PolyLineB[i + 1].Index, PolyLineA[i + 1].Index);
			}
		}
	}
}
