#pragma once

#include "SimpleQuestEditorModule.h"
#include "AssetTypes/QuestlineGraphAssetTypeActions.h"
#include "AssetToolsModule.h"
#include "Modules/ModuleManager.h"
#include "EdGraphUtilities.h"
#include "Utilities/QuestlineGraphCompiler.h"
#include "SGraphNodeKnot.h"
#include "Graph/QuestlineGraphSchema.h"
#include "Nodes/QuestlineNode_Knot.h"
#include "Toolkit/QuestlineGraphEditorCommands.h"
#include "Utilities/QuestGiverManifestBuilder.h"


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
	
	FQuestlineGraphEditorCommands::Register();

	FEditorDelegates::MapChange.AddRaw(this, &FSimpleQuestEditor::OnMapChanged);
	FEditorDelegates::PreBeginPIE.AddRaw(this, &FSimpleQuestEditor::OnPreBeginPIE);
}

void FSimpleQuestEditor::ShutdownModule()
{
	FEditorDelegates::MapChange.RemoveAll(this);
	FEditorDelegates::PreBeginPIE.RemoveAll(this);
	
	FQuestlineGraphEditorCommands::Unregister();
	
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

void FSimpleQuestEditor::RegisterCompilerFactory(FQuestlineCompilerFactoryDelegate InFactory)
{
	CompilerFactory = MoveTemp(InFactory);
}

void FSimpleQuestEditor::UnregisterCompilerFactory()
{
	CompilerFactory.Unbind();
}

TUniquePtr<FQuestlineGraphCompiler> FSimpleQuestEditor::CreateCompiler() const
{
	if (CompilerFactory.IsBound())
	{
		TUniquePtr<FQuestlineGraphCompiler> CustomCompiler = CompilerFactory.Execute();
		if (CustomCompiler.IsValid())
		{
			return CustomCompiler;
		}
		UE_LOG(LogTemp, Warning, TEXT("FSimpleQuestEditor::CreateCompiler : Registered factory returned null. Falling back to default compiler."));
	}
	return MakeUnique<FQuestlineGraphCompiler>();
}

void FSimpleQuestEditor::OnMapChanged(uint32 MapChangeEventFlag)
{
	if (MapChangeEventFlag == static_cast<uint32>(EMapChangeType::SaveMap)) FQuestGiverManifestBuilder::RebuildManifest();
}

void FSimpleQuestEditor::OnPreBeginPIE(const bool bIsSimulating)
{
	FQuestGiverManifestBuilder::RebuildManifest();
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
