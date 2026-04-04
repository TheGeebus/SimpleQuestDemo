// Copyright 2026, Greg Bussell, All Rights Reserved.

#include "Toolkit/QuestlineHierarchyPanel.h"
#include "Quests/QuestlineGraph.h"
#include "Quests/QuestNodeBase.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Text/STextBlock.h"

void SQuestlineHierarchyPanel::Construct(const FArguments& InArgs, UQuestlineGraph* InGraph)
{
    QuestlineGraph = InGraph;

    SAssignNew(TreeView, STreeView<TSharedPtr<FQuestlineHierarchyItem>>)
        .TreeItemsSource(&RootItems)
        .OnGenerateRow(this, &SQuestlineHierarchyPanel::GenerateRow)
        .OnGetChildren(this, &SQuestlineHierarchyPanel::GetChildQuestlineItems);

    ChildSlot[ TreeView.ToSharedRef() ];

    RebuildTree();
}

void SQuestlineHierarchyPanel::Refresh()
{
    RebuildTree();
}

void SQuestlineHierarchyPanel::RebuildTree()
{
    RootItems.Empty();
    if (!QuestlineGraph) return;

    const TMap<FName, TObjectPtr<UQuestNodeBase>>& CompiledNodes = QuestlineGraph->GetCompiledNodes();
    if (CompiledNodes.IsEmpty()) return;

    // Build a flat item map keyed by tag FName
    TMap<FName, TSharedPtr<FQuestlineHierarchyItem>> ItemMap;
    for (const auto& Pair : CompiledNodes)
    {
        auto Item = MakeShared<FQuestlineHierarchyItem>();
        Item->Tag  = Pair.Key;
        Item->Node = Pair.Value;

        // Display name = last dot-separated segment of the tag
        const FString TagStr = Pair.Key.ToString();
        FString Ignored, LastSegment;
        if (!TagStr.Split(TEXT("."), &Ignored, &LastSegment, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
            LastSegment = TagStr;
        Item->DisplayName = LastSegment;

        ItemMap.Add(Pair.Key, Item);
    }

    // Attach each item to its parent, or promote to root if the parent isn't in CompiledNodes
    for (const auto& ItemPair : ItemMap)
    {
        const TSharedPtr<FQuestlineHierarchyItem>& Item = ItemPair.Value;
        const FString TagStr = Item->Tag.ToString();

        FString ParentStr, LastSeg;
        if (TagStr.Split(TEXT("."), &ParentStr, &LastSeg, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
        {
            if (TSharedPtr<FQuestlineHierarchyItem>* ParentItem = ItemMap.Find(FName(*ParentStr)))
            {
                (*ParentItem)->Children.Add(Item);
                continue;
            }
        }
        RootItems.Add(Item);
    }

    if (TreeView.IsValid())
    {
        TreeView->RequestTreeRefresh();
        for (const auto& Root : RootItems)
            TreeView->SetItemExpansion(Root, true);
    }
}

TSharedRef<ITableRow> SQuestlineHierarchyPanel::GenerateRow(TSharedPtr<FQuestlineHierarchyItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedPtr<FQuestlineHierarchyItem>>, OwnerTable)
    [
        SNew(STextBlock)
        .Text(FText::FromString(Item->DisplayName))
    ];
}

void SQuestlineHierarchyPanel::GetChildQuestlineItems(TSharedPtr<FQuestlineHierarchyItem> Item, TArray<TSharedPtr<FQuestlineHierarchyItem>>& OutChildren)
{
    OutChildren = Item->Children;
}
