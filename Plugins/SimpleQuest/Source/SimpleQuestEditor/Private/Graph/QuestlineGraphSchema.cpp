// Copyright 2026, Greg Bussell, All Rights Reserved.

#include "Graph/QuestlineGraphSchema.h"

#include "Nodes/QuestlineNode_Entry.h"
#include "Nodes/QuestlineNode_Knot.h"
#include "Nodes/QuestlineNode_Quest.h"
#include "Utilities/SimpleQuestEditorUtils.h"
#include "ConnectionDrawingPolicy.h"
#include "EdGraphUtilities.h"
#include "NodeFactory.h" 
#include "Nodes/QuestlineNode_Exit_Failure.h"
#include "Nodes/QuestlineNode_Exit_Success.h"
#include "ScopedTransaction.h"


// Shared helper — called by both drawing policies to avoid duplicating color/flag logic
static void ApplyQuestlineWireParams(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, FConnectionParams& Params)
{
	if (OutputPin)
	{
		const FName Category = OutputPin->PinType.PinCategory;
		if      (Category == TEXT("QuestSuccess"))  Params.WireColor = SQ_ED_GREEN;
		else if (Category == TEXT("QuestFailure"))  Params.WireColor = SQ_ED_RED;
		else                                        Params.WireColor = FLinearColor::White;
	}
	if (InputPin && InputPin->PinName == TEXT("Prerequisites"))
		Params.bUserFlag1 = true;
}

class FQuestlineConnectionDrawingPolicy : public FConnectionDrawingPolicy
{
public:
	FQuestlineConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID,
		float InZoomFactor, const FSlateRect& InClippingRect,
		FSlateWindowElementList& InDrawElements, UEdGraph* InGraph)
		: FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements)
		, GraphObj(InGraph)
	{}

	virtual void DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, FConnectionParams& Params) override
	{
		FConnectionDrawingPolicy::DetermineWiringStyle(OutputPin, InputPin, Params);
		if (OutputPin) Params.StartDirection = OutputPin->Direction;
		if (InputPin)  Params.EndDirection   = InputPin->Direction;
		ApplyQuestlineWireParams(OutputPin, InputPin, Params);
	}

	virtual void Draw(
		TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries,
		FArrangedChildren& ArrangedNodes) override
	{
		FConnectionDrawingPolicy::Draw(InPinGeometries, ArrangedNodes);
	}

	virtual void DrawConnection(
		int32 LayerId, const FVector2f& Start, const FVector2f& End, const FConnectionParams& Params) override
	{
		// Dashes are the overlay's responsibility — skip them here so they aren't drawn twice
		if (Params.bUserFlag1) return;

		FConnectionDrawingPolicy::DrawConnection(LayerId, Start, End, Params);
	}
	
	/*
void FQuestlineConnectionDrawingPolicy::DrawConnection(int32 LayerId, const FVector2f& Start, const FVector2f& End, const FConnectionParams& Params)
{
    // Solid wires: delegate entirely to the base class.
    // This allows Electronic Nodes (and other plugins) to inject their own rendering.
    if (!Params.bUserFlag1)
    {
        Super::DrawConnection(LayerId, Start, End, Params);
        return;
    }

    // Dashed prerequisite wires must be rendered manually to produce dashes.
    // Use ComputeSplineTangent (virtual — Electronic Nodes can override it) and
    // Params.StartDirection/EndDirection (set by Super::DetermineWiringStyle) so
    // the curve shape stays consistent with whatever solid wires look like.
    const FVector2f SplineTangent = ComputeSplineTangent(Start, End);
    const FVector2f P0Tangent = (Params.StartDirection == EGPD_Output) ?  SplineTangent : -SplineTangent;
    const FVector2f P1Tangent = (Params.EndDirection   == EGPD_Input)  ? -SplineTangent :  SplineTangent;

    // Sample the bezier into discrete points for dash segment drawing
    const int32 NumSamples = 64;
    TArray<FVector2f> CurvePoints;
    CurvePoints.Reserve(NumSamples + 1);
    for (int32 i = 0; i <= NumSamples; ++i)
    {
        const float T = static_cast<float>(i) / static_cast<float>(NumSamples);
        CurvePoints.Add(FMath::CubicInterp(Start, P0Tangent, End, P1Tangent, T));
    }

    const float DashLength = 10.f;
    const float GapLength  =  5.f;

    float Accumulated = 0.f;
    bool bDrawing = true;
    TArray<FVector2f> CurrentDash;
    CurrentDash.Add(CurvePoints[0]);

    for (int32 i = 1; i < CurvePoints.Num(); ++i)
    {
        const FVector2f& A = CurvePoints[i - 1];
        const FVector2f& B = CurvePoints[i];
        const float SegLength = FVector2f::Distance(A, B);
        if (SegLength < KINDA_SMALL_NUMBER) continue;

        const FVector2f SegDir = (B - A) / SegLength;
        float Consumed = 0.f;

        while (Consumed < SegLength)
        {
            const float Threshold   = bDrawing ? DashLength : GapLength;
            const float ToThreshold = Threshold - Accumulated;
            const float Available   = SegLength - Consumed;

            if (Available < ToThreshold)
            {
                if (bDrawing) CurrentDash.Add(B);
                Accumulated += Available;
                break;
            }

            const FVector2f TransitionPoint = A + SegDir * (Consumed + ToThreshold);
            if (bDrawing)
            {
                CurrentDash.Add(TransitionPoint);
                if (CurrentDash.Num() >= 2)
                {
                    FSlateDrawElement::MakeLines(DrawElementsList, LayerId,
                        FPaintGeometry(), CurrentDash, ESlateDrawEffect::None,
                        Params.WireColor, true, Params.WireThickness);
                }
                CurrentDash.Reset();
            }

            Consumed    += ToThreshold;
            Accumulated  = 0.f;
            bDrawing     = !bDrawing;
            if (bDrawing) CurrentDash.Add(A + SegDir * Consumed);
        }
    }

    if (bDrawing && CurrentDash.Num() >= 2)
    {
        FSlateDrawElement::MakeLines(DrawElementsList, LayerId,
            FPaintGeometry(), CurrentDash, ESlateDrawEffect::None,
            Params.WireColor, true, Params.WireThickness);
    }
}
*/

private:
	UEdGraph* GraphObj;
};

/**
 * Custom dashed line policy. Used in a second pass on graph to allow wire management plugins like Electronic Nodes to function
 * without destroying dashed prerequisite wires.
 */
class FQuestlineDashOverlayPolicy : public FConnectionDrawingPolicy
{
    FConnectionDrawingPolicy* InnerPolicy;

public:
    FQuestlineDashOverlayPolicy(
        FConnectionDrawingPolicy* InInnerPolicy,
        int32 InBackLayerID, int32 InFrontLayerID,
        float InZoomFactor, const FSlateRect& InClippingRect,
        FSlateWindowElementList& InDrawElements)
        : FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements)
        , InnerPolicy(InInnerPolicy)
    {}

    virtual ~FQuestlineDashOverlayPolicy() override
    {
        delete InnerPolicy;
    }
		
	virtual void Draw(
		TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries,
		FArrangedChildren& ArrangedNodes) override
    {
    	// Pass 1: inner policy draws everything (angular + dashes if EN active)
    	InnerPolicy->Draw(InPinGeometries, ArrangedNodes);

    	// Pass 2: only needed when EN is NOT active (inner is plain FQuestlineConnectionDrawingPolicy)
    	// When GENPolicyFactory is set, pass 1 already handled prereq dashes via DrawWireSegment.
    	if (!UQuestlineGraphSchema::IsENPolicyFactoryActive())
    		FConnectionDrawingPolicy::Draw(InPinGeometries, ArrangedNodes);
    }
		
    virtual void DetermineWiringStyle(
        UEdGraphPin* OutputPin, UEdGraphPin* InputPin, FConnectionParams& Params) override
    {
        FConnectionDrawingPolicy::DetermineWiringStyle(OutputPin, InputPin, Params);
        if (OutputPin) Params.StartDirection = OutputPin->Direction;
        if (InputPin)  Params.EndDirection   = InputPin->Direction;
        ApplyQuestlineWireParams(OutputPin, InputPin, Params);
    }

    virtual void DrawConnection(
        int32 LayerId, const FVector2f& Start, const FVector2f& End,
        const FConnectionParams& Params) override
    {
        // Solid wires were already drawn by the inner policy — skip them
        if (!Params.bUserFlag1) return;

        // Dashed prerequisite wire
        const bool  bGoingForward  = (End.X >= Start.X);
        const float ClampedTension = FMath::Clamp(FMath::Abs(End.X - Start.X), 30.f, 500.f);
        const FVector2f P0Tangent(bGoingForward ?  ClampedTension : -ClampedTension, 0.f);
        const FVector2f P1Tangent(bGoingForward ? -ClampedTension :  ClampedTension, 0.f);

        const int32 NumSamples = 64;
        TArray<FVector2f> CurvePoints;
        CurvePoints.Reserve(NumSamples + 1);
        for (int32 i = 0; i <= NumSamples; ++i)
        {
            const float T = static_cast<float>(i) / static_cast<float>(NumSamples);
            CurvePoints.Add(FMath::CubicInterp(Start, P0Tangent, End, P1Tangent, T));
        }

        const float DashLength = 10.f;
        const float GapLength  =  5.f;

        float Accumulated = 0.f;
        bool bDrawing = true;
        TArray<FVector2f> CurrentDash;
        CurrentDash.Add(CurvePoints[0]);

        for (int32 i = 1; i < CurvePoints.Num(); ++i)
        {
            const FVector2f& A = CurvePoints[i - 1];
            const FVector2f& B = CurvePoints[i];
            const float SegLength = FVector2f::Distance(A, B);
            if (SegLength < KINDA_SMALL_NUMBER) continue;

            const FVector2f SegDir = (B - A) / SegLength;
            float Consumed = 0.f;

            while (Consumed < SegLength)
            {
                const float Threshold   = bDrawing ? DashLength : GapLength;
                const float ToThreshold = Threshold - Accumulated;
                const float Available   = SegLength - Consumed;

                if (Available < ToThreshold)
                {
                    if (bDrawing) CurrentDash.Add(B);
                    Accumulated += Available;
                    break;
                }

                const FVector2f TransitionPoint = A + SegDir * (Consumed + ToThreshold);
                if (bDrawing)
                {
                    CurrentDash.Add(TransitionPoint);
                    if (CurrentDash.Num() >= 2)
                    {
                        FSlateDrawElement::MakeLines(DrawElementsList, LayerId,
                            FPaintGeometry(), CurrentDash, ESlateDrawEffect::None,
                            Params.WireColor, true, Params.WireThickness);
                    }
                    CurrentDash.Reset();
                }

                Consumed    += ToThreshold;
                Accumulated  = 0.f;
                bDrawing     = !bDrawing;
                if (bDrawing) CurrentDash.Add(A + SegDir * Consumed);
            }
        }

        if (bDrawing && CurrentDash.Num() >= 2)
        {
            FSlateDrawElement::MakeLines(DrawElementsList, LayerId,
                FPaintGeometry(), CurrentDash, ESlateDrawEffect::None,
                Params.WireColor, true, Params.WireThickness);
        }
    }
};

class FQuestlineConnectionFactory : public FGraphPanelPinConnectionFactory
{
	mutable bool bInProgress = false;

public:
	virtual FConnectionDrawingPolicy* CreateConnectionPolicy(
		const UEdGraphSchema* Schema,
		int32 InBackLayerID, int32 InFrontLayerID,
		float InZoomFactor,
		const FSlateRect& InClippingRect,
		FSlateWindowElementList& InDrawElements,
		UEdGraph* InGraphObj) const override
	{
		if (!Cast<const UQuestlineGraphSchema>(Schema))
			return nullptr;

		// Guard against re-entry: when FNodeFactory calls back into us during
		// the inner iteration, return nullptr so EN's factory wins that call.
		if (bInProgress)
			return nullptr;

		bInProgress = true;

		// Ask every registered factory (including EN) for a policy.
		// Our factory returns nullptr above, so EN gets to run unobstructed.
		FConnectionDrawingPolicy* InnerPolicy = FNodeFactory::CreateConnectionPolicy(
			Schema, InBackLayerID, InFrontLayerID,
			InZoomFactor, InClippingRect, InDrawElements, InGraphObj);

		bInProgress = false;

		return new FQuestlineDashOverlayPolicy(
			InnerPolicy,
			InBackLayerID, InFrontLayerID,
			InZoomFactor, InClippingRect, InDrawElements);
	}
};

TSharedPtr<FGraphPanelPinConnectionFactory> UQuestlineGraphSchema::MakeQuestlineConnectionFactory()
{
	return MakeShared<FQuestlineConnectionFactory>();
}

void UQuestlineGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	FGraphNodeCreator<UQuestlineNode_Entry> Creator(Graph);
	UQuestlineNode_Entry* EntryNode = Creator.CreateNode();
	EntryNode->NodePosX = 0;
	EntryNode->NodePosY = 0;
	Creator.Finalize();
	SetNodeMetaData(EntryNode, FNodeMetadata::DefaultGraphNode);
}

void UQuestlineGraphSchema::OnPinConnectionDoubleCicked(UEdGraphPin* PinA, UEdGraphPin* PinB, const FVector2f& GraphPosition) const
{
	if (!PinA || !PinB) return;

	UEdGraph* Graph = PinA->GetOwningNode()->GetGraph();
	if (!Graph) return;

	const FScopedTransaction Transaction(NSLOCTEXT("SimpleQuestEditor", "CreateRerouteNode", "Create Reroute Node"));
	Graph->Modify();
	PinA->GetOwningNode()->Modify();
	PinB->GetOwningNode()->Modify();

	UEdGraphPin* OutputPin = (PinA->Direction == EGPD_Output) ? PinA : PinB;
	UEdGraphPin* InputPin  = (PinA->Direction == EGPD_Input)  ? PinA : PinB;

	FGraphNodeCreator<UQuestlineNode_Knot> Creator(*Graph);
	UQuestlineNode_Knot* KnotNode = Creator.CreateNode();
	KnotNode->NodePosX = FMath::RoundToInt(GraphPosition.X);
	KnotNode->NodePosY = FMath::RoundToInt(GraphPosition.Y);
	Creator.Finalize();

	UEdGraphPin* KnotIn  = KnotNode->FindPin(TEXT("KnotIn"));
	UEdGraphPin* KnotOut = KnotNode->FindPin(TEXT("KnotOut"));
	if (!KnotIn || !KnotOut) return;

	// Inherit the wire type so the knot locks to the correct signal colour
	KnotIn->PinType  = OutputPin->PinType;
	KnotOut->PinType = OutputPin->PinType;

	// Re-route: break existing link, wire through knot
	OutputPin->BreakLinkTo(InputPin);
	OutputPin->MakeLinkTo(KnotIn);
	KnotOut->MakeLinkTo(InputPin);

	Graph->NotifyGraphChanged();

	if (UObject* Outer = Graph->GetOuter())
	{
		Outer->PostEditChange();
		Outer->MarkPackageDirty();
	}
}

void UQuestlineGraphSchema::CollectSourceQuests(const UEdGraphPin* Pin, TSet<UQuestlineNode_Quest*>& OutSources,
                                                TSet<const UEdGraphNode*>& Visited)
{
	if (!Pin) return;

	const UEdGraphNode* Node = Pin->GetOwningNode();
	if (Visited.Contains(Node)) return;
	Visited.Add(Node);

	if (UQuestlineNode_Quest* Quest = const_cast<UQuestlineNode_Quest*>(Cast<const UQuestlineNode_Quest>(Node)))
	{
		OutSources.Add(Quest);
		return;
	}

	if (const UQuestlineNode_Knot* Knot = Cast<const UQuestlineNode_Knot>(Node))
	{
		const UEdGraphPin* KnotIn = Knot->FindPin(TEXT("KnotIn"));
		if (KnotIn)
		{
			for (const UEdGraphPin* Linked : KnotIn->LinkedTo)
				CollectSourceQuests(Linked, OutSources, Visited);
		}
	}
}

const FPinConnectionResponse UQuestlineGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	if (!A || !B)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, NSLOCTEXT("SimpleQuestEditor", "PinNull", "Invalid pin"));
	}

	if (A->Direction == B->Direction)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, NSLOCTEXT("SimpleQuestEditor", "SameDirection", "Cannot connect two inputs or two outputs"));
	}

	const UEdGraphPin* OutputPin = A->Direction == EGPD_Output ? A : B;
	const UEdGraphPin* InputPin  = A->Direction == EGPD_Input  ? A : B;

	const UEdGraphNode* OutputNode = OutputPin->GetOwningNode();
	const UEdGraphNode* InputNode  = InputPin->GetOwningNode();

	const bool bOutputIsKnot = Cast<const UQuestlineNode_Knot>(OutputNode) != nullptr;
	const bool bInputIsKnot  = Cast<const UQuestlineNode_Knot>(InputNode)  != nullptr;

	auto CheckDuplicateSources = [&]() -> FPinConnectionResponse
	{
		TSet<UQuestlineNode_Quest*> IncomingSources;
		{
			TSet<const UEdGraphNode*> Visited;
			CollectSourceQuests(OutputPin, IncomingSources, Visited);
		}
		if (IncomingSources.Num() > 0)
		{
			for (const UEdGraphPin* Existing : InputPin->LinkedTo)
			{
				TSet<UQuestlineNode_Quest*> ExistingSources;
				TSet<const UEdGraphNode*> Visited;
				CollectSourceQuests(Existing, ExistingSources, Visited);

				if (int32 NumCollisions = IncomingSources.Intersect(ExistingSources).Num(); NumCollisions > 0)
				{
					FText DescriptionText;
					if (bOutputIsKnot && IncomingSources.Num() > 1)
					{
						DescriptionText = NumCollisions > 1
						? NSLOCTEXT("SimpleQuestEditor", "DuplicateSourceKnotMultiple", "This input already receives a signal from some of those Quest nodes.")
						: NSLOCTEXT("SimpleQuestEditor", "DuplicateSourceKnotSingle",   "This input already receives a signal from one of those Quest nodes.");
					}
					else
					{
						DescriptionText = NSLOCTEXT("SimpleQuestEditor", "DuplicateSourceKnotMultiple", "This input already receives a signal from that Quest node.");
					}
					return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, DescriptionText);
				}
			}
		}
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, FText::GetEmpty());
	};

	if (bOutputIsKnot || bInputIsKnot)
	{
		if (bOutputIsKnot && bInputIsKnot)
		{
			if (OutputNode == InputNode)
			{
				return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW,
					NSLOCTEXT("SimpleQuestEditor", "KnotSelfLoop", "Cannot connect a reroute node to itself"));
			}

			// Only enforce type consistency if the target knot's KnotIn already has connections.
			// An empty KnotIn accepts the first wire of any color (which then locks its type).
			if (InputPin->LinkedTo.Num() > 0)
			{
				const FName InCat  = Cast<const UQuestlineNode_Knot>(InputNode)->GetEffectiveCategory();
				const FName OutCat = Cast<const UQuestlineNode_Knot>(OutputNode)->GetEffectiveCategory();
				if (InCat != OutCat)
				{
					return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW,
						NSLOCTEXT("SimpleQuestEditor", "KnotTypeMismatch", "Reroute node signal types do not match"));
				}
			}
			return CheckDuplicateSources();
		}

		if (bInputIsKnot)
		{
			// Only enforce type consistency if the target knot's KnotIn already has connections.
			// An empty KnotIn accepts the first wire of any color (which then locks its type).
			if (InputPin->LinkedTo.Num() > 0)
			{
				const FName KnotCategory = Cast<const UQuestlineNode_Knot>(InputNode)->GetEffectiveCategory();
				const FName WireCategory = OutputPin->PinType.PinCategory;
				if (KnotCategory != WireCategory)
				{
					return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW,
						NSLOCTEXT("SimpleQuestEditor", "KnotTypeMismatch", "Signal type does not match this reroute node"));
				}
			}
			return CheckDuplicateSources();
		}

		// bOutputIsKnot, non-knot destination — fall through to duplicate check
	}
	else
	{
		if (OutputNode == InputNode)
		{
			if (Cast<const UQuestlineNode_Quest>(OutputNode) && InputPin->PinName == TEXT("Activate"))
			{
				return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, FText::GetEmpty());
			}
			return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW,
				NSLOCTEXT("SimpleQuestEditor", "SameNode", "Cannot connect a node to its own Prerequisites pin"));
		}

		if (Cast<const UQuestlineNode_Entry>(OutputNode))
		{
			if (!Cast<const UQuestlineNode_Quest>(InputNode))
			{
				return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW,
					NSLOCTEXT("SimpleQuestEditor", "EntryOnlyToQuest", "Quest Start may only connect to a Quest node"));
			}
			if (InputPin->PinName != TEXT("Activate"))
			{
				return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW,
					NSLOCTEXT("SimpleQuestEditor", "EntryOnlyActivate", "Quest Start may only connect to the Activate pin"));
			}
		}

		if (Cast<const UQuestlineNode_Quest>(OutputNode))
		{
			const bool bValidInput = Cast<const UQuestlineNode_Quest>(InputNode)
								  || Cast<const UQuestlineNode_Knot>(InputNode)
								  || Cast<const UQuestlineNode_Exit_Success>(InputNode)
								  || Cast<const UQuestlineNode_Exit_Failure>(InputNode);
			if (!bValidInput)
			{
				return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW,
					NSLOCTEXT("SimpleQuestEditor", "QuestOutputOnlyToQuest",
						"Quest outputs may only connect to other Quest nodes or exit nodes"));
			}
		}
	}

	return CheckDuplicateSources();
}

void UQuestlineGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	const FText AddQuestLabel = NSLOCTEXT("SimpleQuestEditor", "AddQuestNode", "Add Quest");
	const FText AddQuestTooltip = NSLOCTEXT("SimpleQuestEditor", "AddQuestNodeTooltip", "Add a Quest node to the graph");

	// Add Quest node action
	TSharedPtr<FEdGraphSchemaAction_NewNode> NewQuestAction(
		new FEdGraphSchemaAction_NewNode(
			FText::GetEmpty(),
			AddQuestLabel,
			AddQuestTooltip,
			0));

	NewQuestAction->NodeTemplate = NewObject<UQuestlineNode_Quest>(
		const_cast<UEdGraph*>(ContextMenuBuilder.CurrentGraph));

	ContextMenuBuilder.AddAction(NewQuestAction);

	// Add Reroute node action
	TSharedPtr<FEdGraphSchemaAction_NewNode> RerouteAction(
		new FEdGraphSchemaAction_NewNode(
			FText::GetEmpty(),
			NSLOCTEXT("SimpleQuestEditor", "AddRerouteNode", "Add Reroute Node"),
			NSLOCTEXT("SimpleQuestEditor", "AddRerouteNodeTooltip", "Add a reroute node"),
			0));

	RerouteAction->NodeTemplate = NewObject<UQuestlineNode_Knot>(
		const_cast<UEdGraph*>(ContextMenuBuilder.CurrentGraph));

	ContextMenuBuilder.AddAction(RerouteAction);

	// Add Questline Success exit
	{
		TSharedPtr<FEdGraphSchemaAction_NewNode> Action(new FEdGraphSchemaAction_NewNode(
			FText::GetEmpty(),
			NSLOCTEXT("SimpleQuestEditor", "AddExitSuccess", "Add Questline Success"),
			NSLOCTEXT("SimpleQuestEditor", "AddExitSuccessTooltip", "Add a Questline Success exit node"),
			0));
		Action->NodeTemplate = NewObject<UQuestlineNode_Exit_Success>(
			const_cast<UEdGraph*>(ContextMenuBuilder.CurrentGraph));
		ContextMenuBuilder.AddAction(Action);
	}

	// Add Questline Failure exit
	{
		TSharedPtr<FEdGraphSchemaAction_NewNode> Action(new FEdGraphSchemaAction_NewNode(
			FText::GetEmpty(),
			NSLOCTEXT("SimpleQuestEditor", "AddExitFailure", "Add Questline Failure"),
			NSLOCTEXT("SimpleQuestEditor", "AddExitFailureTooltip", "Add a Questline Failed exit node"),
			0));
		Action->NodeTemplate = NewObject<UQuestlineNode_Exit_Failure>(
			const_cast<UEdGraph*>(ContextMenuBuilder.CurrentGraph));
		ContextMenuBuilder.AddAction(Action);
	}

}

void UQuestlineGraphSchema::GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	// Node-specific context menu actions go here
}

FLinearColor UQuestlineGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	if (PinType.PinCategory == TEXT("QuestSuccess"))
	{
		return SQ_ED_GREEN;		// green
	}
	if (PinType.PinCategory == TEXT("QuestFailure"))
	{
		return SQ_ED_RED;		// red
	}
	return FLinearColor::White;		// QuestActivation — Activate, Prerequisites, AnyOutcome, Start, knots
}

/* Automatic reroute node placement when looping back to the same node -- currently always feeds wire into left side of node, needs a fix to create a clean loop */

/*
bool UQuestlineGraphSchema::TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const
{
	UEdGraphPin* OutputPin = A->Direction == EGPD_Output ? A : B;
	UEdGraphPin* InputPin  = A->Direction == EGPD_Input  ? A : B;

	// Intercept self-loop: insert two knots to route the wire around the node
	if (OutputPin->GetOwningNode() == InputPin->GetOwningNode() &&
		Cast<UQuestlineNode_Quest>(OutputPin->GetOwningNode()) &&
		InputPin->PinName == TEXT("Activate"))
	{
		UEdGraphNode* QuestNode = OutputPin->GetOwningNode();
		UEdGraph* Graph = QuestNode->GetGraph();
		Graph->Modify();

		const float NodeWidth  = 200.f;
		const float KnotOffset =  60.f;

		auto MakeKnot = [&](float X, float Y) -> UQuestlineNode_Knot*
		{
			FGraphNodeCreator<UQuestlineNode_Knot> Creator(*Graph);
			UQuestlineNode_Knot* Knot = Creator.CreateNode();
			Knot->NodePosX = X;
			Knot->NodePosY = Y;
			Creator.Finalize();
			if (UEdGraphPin* In  = Knot->FindPin(TEXT("KnotIn")))  In->PinType  = OutputPin->PinType;
			if (UEdGraphPin* Out = Knot->FindPin(TEXT("KnotOut"))) Out->PinType = OutputPin->PinType;
			return Knot;
		};

		UQuestlineNode_Knot* KnotRight = MakeKnot(QuestNode->NodePosX + NodeWidth, QuestNode->NodePosY - KnotOffset);
		UQuestlineNode_Knot* KnotLeft  = MakeKnot(QuestNode->NodePosX,             QuestNode->NodePosY - KnotOffset);

		Super::TryCreateConnection(OutputPin,                          KnotRight->FindPin(TEXT("KnotIn")));
		Super::TryCreateConnection(KnotRight->FindPin(TEXT("KnotOut")), KnotLeft->FindPin(TEXT("KnotIn")));
		Super::TryCreateConnection(KnotLeft->FindPin(TEXT("KnotOut")),  InputPin);

		return true;
	}

	return Super::TryCreateConnection(A, B);
}
*/

// File-local storage — never crosses DLL boundaries
static TFunction<FConnectionDrawingPolicy*(int32, int32, float, const FSlateRect&, FSlateWindowElementList&, UEdGraph*)>
	GENPolicyFactory;

void UQuestlineGraphSchema::RegisterENPolicyFactory(
	TFunction<FConnectionDrawingPolicy*(int32, int32, float, const FSlateRect&, FSlateWindowElementList&, UEdGraph*)> Factory)
{
	GENPolicyFactory = MoveTemp(Factory);
}

void UQuestlineGraphSchema::UnregisterENPolicyFactory()
{
	GENPolicyFactory = nullptr;
}

bool UQuestlineGraphSchema::IsENPolicyFactoryActive()
{
	return static_cast<bool>(GENPolicyFactory);
}

FConnectionDrawingPolicy* UQuestlineGraphSchema::CreateConnectionDrawingPolicy(
	int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor,
	const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements,
	UEdGraph* InGraphObj) const
{

	if (GENPolicyFactory)
		return GENPolicyFactory(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);

	return new FQuestlineConnectionDrawingPolicy(
		InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
}



