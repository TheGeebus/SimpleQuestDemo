// Copyright 2026, Greg Bussell, All Rights Reserved.
#pragma once

#include "Nodes/QuestlineNode_ExitBase.h"
#include "QuestlineNode_Exit_Failure.generated.h"

UCLASS()
class UQuestlineNode_Exit_Failure : public UQuestlineNode_ExitBase
{
	GENERATED_BODY()
public:
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
};
