// Copyright 2026, Greg Bussell, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "EdGraph/EdGraphSchema.h"
#include "QuestlineGraphSchema.generated.h"

struct FGraphPanelPinConnectionFactory;
class UQuestlineNode_Quest;


UCLASS()
class SIMPLEQUESTEDITOR_API UQuestlineGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()

public:
	// Called when the graph is first created — populates it with the entry node
	virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;

	// Helper that checks for connected quest source nodes when establishing a connection, through branching reroutes as needed 
	static void CollectSourceQuests(const UEdGraphPin* Pin, TSet<UQuestlineNode_Quest*>& OutSources, TSet<const UEdGraphNode*>& Visited);
	// Helper to find destination input pins of a given output pin, through branching reroutes as needed
	static void CollectDownstreamTerminalInputs(const UEdGraphPin* KnotOutPin, TArray<const UEdGraphPin*>& OutTerminalPins, TSet<const UEdGraphNode*>& Visited);
		
	// Determines whether a connection between two pins is valid
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;

	// Populates the right-click context menu for the graph background
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;

	// Populates the right-click context menu when dragging off a pin
	virtual void GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;

	virtual void OnPinConnectionDoubleCicked(UEdGraphPin* PinA, UEdGraphPin* PinB, const FVector2f& GraphPosition) const override;
	
	// Returns the name to display for this graph type in the editor
	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;

	//virtual bool TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const override;
	
	virtual FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(
	int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor,
	const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements,
	UEdGraph* InGraphObj) const override;
	
	static TSharedPtr<FGraphPanelPinConnectionFactory> MakeQuestlineConnectionFactory();

	// Optional integration point for SimpleQuestEditorEN.
	static void RegisterENPolicyFactory(TFunction<FConnectionDrawingPolicy*(int32, int32, float, const FSlateRect&, FSlateWindowElementList&, UEdGraph*)> Factory);
	static void UnregisterENPolicyFactory();
	static bool IsENPolicyFactoryActive();
		
	static void SetActiveDragFromPin(UEdGraphPin* Pin);
	static UEdGraphPin* GetActiveDragFromPin();
	static void ClearActiveDragFromPin();
};