// Copyright 2026, Greg Bussell, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "QuestlineNode_ContentBase.generated.h"

UCLASS(Abstract)
class SIMPLEQUESTEDITOR_API UQuestlineNode_ContentBase : public UEdGraphNode
{
	GENERATED_BODY()

public:
	virtual void AllocateDefaultPins() override;
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;
	virtual bool CanUserDeleteNode() const override { return true; }
	virtual bool CanDuplicateNode() const override { return true; }
	virtual void PostPlacedNewNode() override;
	virtual void PostDuplicate(bool bDuplicateForPIE) override;

	// Display name set by the designer in the graph
	UPROPERTY(EditAnywhere, Category = "Quest")
	FText NodeLabel;

	// Stable identity for this node. Used to derive QuestID at compile time. Generated once on placement and regenerated
	// on duplication. Never hand-edited.
	UPROPERTY(VisibleAnywhere, Category = "Quest")
	FGuid QuestGuid;
};
