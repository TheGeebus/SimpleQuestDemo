// Copyright 2026, Greg Bussell, All Rights Reserved.

#pragma once


class USimpleQuestEditorStatics
{
	
public:
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
};

