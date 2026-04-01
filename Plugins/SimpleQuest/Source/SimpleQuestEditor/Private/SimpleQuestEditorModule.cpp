#pragma once

#include "SimpleQuestEditorModule.h"
#include "AssetTypes/QuestlineGraphAssetTypeActions.h"
#include "AssetToolsModule.h"
#include "Modules/ModuleManager.h"
#include "EdGraphUtilities.h"
#include "FileHelpers.h"
#include "SGraphNodeKnot.h"
#include "Graph/QuestlineGraphSchema.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Nodes/QuestlineNode_Knot.h"
#include "Settings/SimpleQuestSettings.h"
#include "Subsystems/QuestManagerSubsystem.h"


IMPLEMENT_MODULE(FSimpleQuestEditor, SimpleQuestEditor);

class FQuestlineGraphNodeFactory : public FGraphPanelNodeFactory
{
	virtual TSharedPtr<SGraphNode> CreateNode(UEdGraphNode* Node) const override
	{
		if (UQuestlineNode_Knot* KnotNode = Cast<UQuestlineNode_Knot>(Node))
		{
			return SNew(SGraphNodeKnot, KnotNode);
		}
		return nullptr;
	}
};

void FSimpleQuestEditor::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	QuestlineGraphAssetTypeActions = MakeShared<FQuestlineGraphAssetTypeActions>();
	AssetTools.RegisterAssetTypeActions(QuestlineGraphAssetTypeActions.ToSharedRef());

	QuestlineGraphNodeFactory = MakeShared<FQuestlineGraphNodeFactory>();
	FEdGraphUtilities::RegisterVisualNodeFactory(QuestlineGraphNodeFactory);

	QuestlineConnectionFactory = UQuestlineGraphSchema::MakeQuestlineConnectionFactory();
	FEdGraphUtilities::RegisterVisualPinConnectionFactory(QuestlineConnectionFactory);

	FEditorDelegates::PreBeginPIE.AddLambda([](bool)
	{
		const USimpleQuestSettings* Settings = GetDefault<USimpleQuestSettings>();

		// Strip _C from the class path to get the Blueprint asset path
		FString ClassPath = Settings->QuestManagerClass.ToString();
		if (!ClassPath.RemoveFromEnd(TEXT("_C")))
		{
			//return; // Not a Blueprint-generated class, nothing to compile
		}

		if (UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *ClassPath))
		{
			if (Blueprint->Status != EBlueprintStatus::BS_UpToDate)
			{
				FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::SkipGarbageCollection);
			}
		}
	});
}

void FSimpleQuestEditor::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AssetTools.UnregisterAssetTypeActions(QuestlineGraphAssetTypeActions.ToSharedRef());
	}
	FEdGraphUtilities::UnregisterVisualNodeFactory(QuestlineGraphNodeFactory);
	QuestlineGraphNodeFactory.Reset();

	FEdGraphUtilities::UnregisterVisualPinConnectionFactory(QuestlineConnectionFactory);
	QuestlineConnectionFactory.Reset();
	
	QuestlineGraphAssetTypeActions.Reset();



}
/*
class FQuestlineConnectionFactory : public FGraphPanelPinConnectionFactory
{
public:
	virtual FConnectionDrawingPolicy* CreateConnectionPolicy(
		const UEdGraphSchema* Schema,
		int32 InBackLayerID, int32 InFrontLayerID,
		float InZoomFactor,
		const FSlateRect& InClippingRect,
		FSlateWindowElementList& InDrawElements,
		UEdGraph* InGraphObj) const override
	{
		if (!Cast<UQuestlineGraphSchema>(Schema))
			return nullptr;

		// Ask every OTHER registered factory for a policy (picks up Electronic Nodes, etc.)
		FConnectionDrawingPolicy* InnerPolicy = nullptr;
		for (const TSharedPtr<FGraphPanelPinConnectionFactory>& OtherFactory
				: FEdGraphUtilities::VisualPinConnectionFactories)
		{
			if (OtherFactory.Get() == this) continue;
			InnerPolicy = OtherFactory->CreateConnectionPolicy(
				Schema, InBackLayerID, InFrontLayerID,
				InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
			if (InnerPolicy) break;
		}

		// Fall back to the schema's own policy if no other factory handled it
		if (!InnerPolicy)
		{
			InnerPolicy = Schema->CreateConnectionDrawingPolicy(
				InBackLayerID, InFrontLayerID,
				InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
		}

		return new FQuestlineDashOverlayPolicy(
			InnerPolicy,
			InBackLayerID, InFrontLayerID,
			InZoomFactor, InClippingRect, InDrawElements);
	}
};
*/
