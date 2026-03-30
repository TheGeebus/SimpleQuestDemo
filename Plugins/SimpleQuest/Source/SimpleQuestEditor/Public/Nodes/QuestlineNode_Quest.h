// Copyright 2026, Greg Bussell, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "Misc/Guid.h"
#include "QuestlineNode_Quest.generated.h"

class UQuest;

UCLASS()
class UQuestlineNode_Quest : public UEdGraphNode
{
	GENERATED_BODY()

public:
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool CanUserDeleteNode() const override { return true; }
	virtual bool CanDuplicateNode() const override { return true; }
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;

	// The quest class this node represents
	UPROPERTY(EditAnywhere, Category = "Quest")
	TSubclassOf<UQuest> QuestClass;

	// Display name set by the designer in the graph
	UPROPERTY(EditAnywhere, Category = "Quest")
	FText NodeLabel;

	// Stable identity for this node. Used to derive QuestID at compile time. Generated once on placement and regenerated
	// on duplication — never hand-edited.
	UPROPERTY(VisibleAnywhere, Category = "Quest")
	FGuid QuestGuid;

	virtual void PostPlacedNewNode() override;
	virtual void PostDuplicate(bool bDuplicateForPIE) override;
};
