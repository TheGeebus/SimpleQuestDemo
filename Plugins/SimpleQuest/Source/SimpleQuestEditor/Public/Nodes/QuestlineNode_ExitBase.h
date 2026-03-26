// Copyright 2026, Greg Bussell, All Rights Reserved.

#pragma once

#include "EdGraph/EdGraphNode.h"
#include "QuestlineNode_ExitBase.generated.h"

UCLASS(Abstract)
class UQuestlineNode_ExitBase : public UEdGraphNode
{
	GENERATED_BODY()
public:
	virtual bool CanUserDeleteNode() const override { return true; }
	virtual bool CanDuplicateNode() const override { return true; }
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;
};
