// Copyright 2026, Greg Bussell, All Rights Reserved.

#pragma once

#include "ConnectionDrawingPolicy.h"


class FQuestlineConnectionDrawingPolicy : public FConnectionDrawingPolicy
{
public:
	FQuestlineConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect,
		FSlateWindowElementList& InDrawElements, UEdGraph* InGraph)
		: FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements), GraphObj(InGraph)
	{}

	virtual void DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, FConnectionParams& Params) override
	{
		FConnectionDrawingPolicy::DetermineWiringStyle(OutputPin, InputPin, Params);
		if (OutputPin) Params.StartDirection = OutputPin->Direction;
		if (InputPin) Params.EndDirection = InputPin->Direction;
		ApplyQuestlineWireParams(OutputPin, InputPin, Params);
	}

	virtual void Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries, FArrangedChildren& ArrangedNodes) override
	{
		FConnectionDrawingPolicy::Draw(InPinGeometries, ArrangedNodes);
	}

	virtual void DrawConnection(int32 LayerId, const FVector2f& Start, const FVector2f& End, const FConnectionParams& Params) override
	{
		if (!Params.bUserFlag1)
		{
			FConnectionDrawingPolicy::DrawConnection(LayerId, Start, End, Params);
			return;
		}

		const FVector2f SplineTangent = ComputeSplineTangent(Start, End);
		const FVector2f P0Tangent = Params.StartTangent.IsNearlyZero()
			? ((Params.StartDirection == EGPD_Output) ? SplineTangent : -SplineTangent)
			: Params.StartTangent;
		const FVector2f P1Tangent = Params.EndTangent.IsNearlyZero()
			? ((Params.EndDirection == EGPD_Input) ? SplineTangent : -SplineTangent)
			: Params.EndTangent;

		constexpr int32 NumSamples = 64;
		constexpr float DashLength = 10.f;
		constexpr float GapLength = 5.f;

		TArray<FVector2f> CurvePoints;
		CurvePoints.Reserve(NumSamples + 1);
		for (int32 i = 0; i <= NumSamples; ++i)
		{
			CurvePoints.Add(FMath::CubicInterp(Start, P0Tangent, End, P1Tangent, static_cast<float>(i) / NumSamples));
		}
		float Accumulated = 0.f;
		bool bDashing = true;
		TArray<FVector2f> CurrentDash;
		CurrentDash.Add(CurvePoints[0]);

		for (int32 i = 1; i < CurvePoints.Num(); ++i)
		{
			const FVector2f& A = CurvePoints[i - 1];
			const FVector2f& B = CurvePoints[i];
			const float SegLength = FVector2f::Distance(A, B);
			if (SegLength < KINDA_SMALL_NUMBER) continue;

			const FVector2f Dir = (B - A) / SegLength;
			float Consumed = 0.f;

			while (Consumed < SegLength)
			{
				const float Threshold = bDashing ? DashLength : GapLength;
				const float ToThreshold = Threshold - Accumulated;
				const float Available = SegLength - Consumed;

				if (Available < ToThreshold)
				{
					if (bDashing) CurrentDash.Add(B);
					Accumulated += Available;
					break;
				}

				const FVector2f Transition = A + Dir * (Consumed + ToThreshold);
				if (bDashing)
				{
					CurrentDash.Add(Transition);
					if (CurrentDash.Num() >= 2)
					{
						FSlateDrawElement::MakeLines(DrawElementsList, LayerId, FPaintGeometry(), CurrentDash,
							ESlateDrawEffect::None, Params.WireColor, true, Params.WireThickness);
					}
					CurrentDash.Reset();
				}

				Consumed += ToThreshold;
				Accumulated = 0.f;
				bDashing = !bDashing;
				if (bDashing) CurrentDash.Add(A + Dir * Consumed);
			}
		}
		
		if (bDashing && CurrentDash.Num() >= 2)
		{
			FSlateDrawElement::MakeLines(DrawElementsList, LayerId, FPaintGeometry(), CurrentDash, ESlateDrawEffect::None,
				Params.WireColor, true, Params.WireThickness);
		}

		if (Settings->bTreatSplinesLikePins)
		{
			const float ToleranceSq = FMath::Square(Settings->SplineHoverTolerance + Params.WireThickness * 0.5f);

			float BestDistSq = FLT_MAX;
			FVector2f BestPoint  = FVector2f::ZeroVector;

			for (int32 i = 1; i < CurvePoints.Num(); ++i)
			{
				const FVector2f Closest = FMath::ClosestPointOnSegment2D(LocalMousePosition, CurvePoints[i - 1], CurvePoints[i]);
				const float DistSq = (LocalMousePosition - Closest).SizeSquared();
				if (DistSq < BestDistSq)
				{
					BestDistSq = DistSq;
					BestPoint = Closest;
				}
			}

			if (BestDistSq < ToleranceSq && BestDistSq < SplineOverlapResult.GetDistanceSquared())
			{
				const float DistToPin1Sq = Params.AssociatedPin1 ? (Start - BestPoint).SizeSquared() : FLT_MAX;
				const float DistToPin2Sq = Params.AssociatedPin2 ? (End - BestPoint).SizeSquared() : FLT_MAX;
				SplineOverlapResult = FGraphSplineOverlapResult(Params.AssociatedPin1, Params.AssociatedPin2,
					BestDistSq, DistToPin1Sq, DistToPin2Sq, false);
			}
		}
	}

private:
	UEdGraph* GraphObj;
};
