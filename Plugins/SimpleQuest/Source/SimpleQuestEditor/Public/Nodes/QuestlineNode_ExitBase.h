// Copyright 2026, Greg Bussell, All Rights Reserved.

#pragma once

#include "QuestlineNodeBase.h"
#include "QuestlineNode_ExitBase.generated.h"

UCLASS(Abstract)
class SIMPLEQUESTEDITOR_API UQuestlineNode_ExitBase : public UQuestlineNodeBase
{
	GENERATED_BODY()
public:
	virtual bool IsExitNode() const override { return true; }
	
	virtual bool CanUserDeleteNode() const override { return true; }
	virtual bool CanDuplicateNode() const override { return true; }
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;
};
