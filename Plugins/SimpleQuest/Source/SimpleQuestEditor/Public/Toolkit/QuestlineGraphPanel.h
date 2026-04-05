// Copyright 2026, Greg Bussell, All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GraphEditor.h"
#include "Widgets/SCompoundWidget.h"
#include "GraphEditor.h"

class UEdGraph;

class SQuestlineGraphPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SQuestlineGraphPanel) {}
		SLATE_ARGUMENT(SGraphEditor::FGraphEditorEvents, GraphEvents)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraph* InGraph, const TSharedPtr<FUICommandList>& InCommands);

	TSharedPtr<SGraphEditor> GetGraphEditor() const { return GraphEditor; }

	// SWidget interface
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual bool SupportsKeyboardFocus() const override { return false; }
	virtual FReply OnPreviewKeyDown(const FGeometry&, const FKeyEvent&) override;
	virtual FReply OnKeyDown(const FGeometry&, const FKeyEvent&) override;
	virtual FReply OnKeyUp(const FGeometry&, const FKeyEvent&) override;
	virtual FReply OnPreviewMouseButtonDown(const FGeometry&, const FPointerEvent&) override;

	/** Schedules a JumpToNode after the SGraphEditor's first render pass clears its initial ZoomToFit. */
	void JumpToNodeWhenReady(UEdGraphNode* Node);
	
private:
	static bool IsHotkey(FKey Key);
	FVector2D ToGraphCoords(const FGeometry& Geometry, FVector2D ScreenPos) const;

	template<typename TNode>
	UEdGraphNode* SpawnNode(FVector2D GraphPos);

	UEdGraphNode* SpawnNodeForKey(FKey Key, FVector2D GraphPos);

	TSharedPtr<SGraphEditor> GraphEditor;

	// Key+click state (no drag)
	FKey HeldHotkey = EKeys::Invalid;

	void HandleFocusChanging(const FFocusEvent& FocusEvent, const FWeakWidgetPath& OldFocusedWidgetPath, const TSharedPtr<SWidget>& OldFocusedWidget,
		const FWidgetPath& NewFocusedWidgetPath, const TSharedPtr<SWidget>& NewFocusedWidget);

	UEdGraphNode* PendingJumpNode = nullptr;
	bool bHasTicked = false;

};
