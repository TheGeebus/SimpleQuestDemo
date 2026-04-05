// Copyright 2026, Greg Bussell, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Toolkit/QuestlineGraphPanel.h"
#include "Toolkit/QuestlineHierarchyPanel.h"
#include "IDetailsView.h"
#include "ISimpleQuestEditorModule.h"


class UQuestlineGraph;
class SGraphEditor;

class FQuestlineGraphEditor : public FAssetEditorToolkit
{
public:
	virtual ~FQuestlineGraphEditor() override;
	void InitQuestlineGraphEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UQuestlineGraph* InQuestlineGraph);

	// FAssetEditorToolkit interface
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;

private:
	TSharedRef<SDockTab> SpawnGraphViewportTab(const FSpawnTabArgs& Args);
	SGraphEditor::FGraphEditorEvents MakeGraphEvents();
	TSharedRef<SQuestlineGraphPanel> CreateGraphEditorWidget();
	void BindGraphCommands();
	void DeleteSelectedNodes();

	// Compile and save graph data layer
	void CompileQuestlineGraph();
	virtual void SaveAsset_Execute() override;
	void ExtendToolbar();
	void FillToolbar(FToolBarBuilder& ToolbarBuilder);

	FText GetGraphDisplayName(UEdGraph* Graph) const;

	enum class EQuestlineCompileStatus : uint8 { Unknown, UpToDate, Error };

	void OnGraphChanged(const FEdGraphEditAction& Action);
	FSlateIcon GetCompileStatusIcon() const;

	EQuestlineCompileStatus CompileStatus = EQuestlineCompileStatus::Unknown;

	TObjectPtr<UQuestlineGraph> QuestlineGraph;
	TSharedPtr<SQuestlineGraphPanel> GraphEditorWidget;
	TSharedPtr<FUICommandList> GraphEditorCommands;
	static const FName GraphViewportTabId;

	/*-----------------------------------------------------------------------------------
	 * Details Panel
	 *----------------------------------------------------------------------------------*/
	
	TSharedRef<SDockTab> SpawnDetailsTab(const FSpawnTabArgs& Args);
	void OnGraphSelectionChanged(const FGraphPanelSelectionSet& SelectedNodes);

	TSharedPtr<IDetailsView> DetailsView;
	static const FName DetailsTabId;

	/*-----------------------------------------------------------------------------------
	 * Questline Hierarchy Panel
	 *----------------------------------------------------------------------------------*/

	TSharedRef<SDockTab> SpawnHierarchyTab(const FSpawnTabArgs& Args);

	TSharedPtr<SQuestlineHierarchyPanel> HierarchyPanel;
	static const FName HierarchyTabId;

	/*-----------------------------------------------------------------------------------
	 * Nested Graph Navigation
	 *----------------------------------------------------------------------------------*/

	void NavigateTo(UEdGraph* Graph);
	void NavigateBack();
	void NavigateForward();
	void OnBreadcrumbClicked(UEdGraph* const& Graph);
	void OnNodeDoubleClicked(UEdGraphNode* Node);

	bool CanNavigateBack() const { return GraphBackwardStack.Num() > 1; }
	bool CanNavigateForward() const { return GraphForwardStack.Num() > 0; }
	
	TArray<UEdGraph*> GraphBackwardStack;						// backwards-looking stack of opened graphs
	TArray<UEdGraph*> GraphForwardStack;						// populates when using 'back' button, cleared on NavigateTo
	TArray<FDelegateHandle> GraphChangedHandles;				// one per graph in stack
	TSharedPtr<SBox> GraphPanelContainer;						// swapped by NavigateTo
	TSharedPtr<SBreadcrumbTrail<UEdGraph*>> BreadcrumbTrail;	// updated by NavigateTo
	bool bIsNavigatingHistory = false;

};
