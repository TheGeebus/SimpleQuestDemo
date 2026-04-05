// Copyright 2026, Greg Bussell, All Rights Reserved.

#include "Toolkit/QuestlineGraphEditor.h"
#include "Toolkit/QuestlineGraphPanel.h"
#include "Quests/QuestlineGraph.h"
#include "GraphEditor.h"
#include "GraphEditorActions.h"
#include "Utilities/QuestlineGraphCompiler.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Commands/GenericCommands.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Toolkit/QuestlineGraphEditorCommands.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"
#include "Nodes/QuestlineNode_Quest.h"
#include "Widgets/Navigation/SBreadcrumbTrail.h"


const FName FQuestlineGraphEditor::GraphViewportTabId(TEXT("QuestlineGraphEditor_GraphViewport"));
const FName FQuestlineGraphEditor::DetailsTabId(TEXT("QuestlineGraphEditor_Details"));
const FName FQuestlineGraphEditor::HierarchyTabId(TEXT("QuestlineGraphEditor_Hierarchy"));


FQuestlineGraphEditor::~FQuestlineGraphEditor()
{
    for (int32 i = 0; i < GraphBackwardStack.Num(); ++i)
    {
        if (GraphBackwardStack[i])
        {
            GraphBackwardStack[i]->RemoveOnGraphChangedHandler(GraphChangedHandles[i]);
        }
    }
}

void FQuestlineGraphEditor::InitQuestlineGraphEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UQuestlineGraph* InQuestlineGraph)
{
    QuestlineGraph = InQuestlineGraph;

    const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("QuestlineGraphEditor_Layout_v4")
        ->AddArea
        (
            FTabManager::NewPrimaryArea()
            ->SetOrientation(Orient_Horizontal)
            ->Split
            (
                FTabManager::NewSplitter()
                ->SetOrientation(Orient_Vertical)
                ->SetSizeCoefficient(0.15f)
                ->Split
                (
                    FTabManager::NewStack()
                    ->SetSizeCoefficient(0.6f)
                    ->AddTab(HierarchyTabId, ETabState::OpenedTab)
                )
                ->Split
                (
                    FTabManager::NewStack()
                    ->SetSizeCoefficient(0.4f)
                    ->AddTab(DetailsTabId, ETabState::OpenedTab)
                )
            )
            ->Split
            (
                FTabManager::NewStack()
                ->SetSizeCoefficient(0.85f)
                ->AddTab(GraphViewportTabId, ETabState::OpenedTab)
            )
        );
    
    BindGraphCommands();
    ExtendToolbar();

    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.bHideSelectionTip = true;
    DetailsViewArgs.bAllowSearch = false;
    DetailsView = PropertyModule.CreateDetailView(DetailsViewArgs);

    InitAssetEditor(
        Mode,
        InitToolkitHost,
        FName(TEXT("QuestlineGraphEditorApp")),
        Layout,
        true,  // bCreateDefaultStandaloneMenu
        true,  // bCreateDefaultToolbar
        InQuestlineGraph);

}

FName FQuestlineGraphEditor::GetToolkitFName() const
{
    return FName(TEXT("QuestlineGraphEditor"));
}

FText FQuestlineGraphEditor::GetBaseToolkitName() const
{
    return NSLOCTEXT("SimpleQuestEditor", "QuestlineGraphEditorToolkit", "Questline Graph Editor");
}

FString FQuestlineGraphEditor::GetWorldCentricTabPrefix() const
{
    return TEXT("QuestlineGraph ");
}

FLinearColor FQuestlineGraphEditor::GetWorldCentricTabColorScale() const
{
    return FLinearColor(0.18f, 0.67f, 0.51f, 0.5f);
}

void FQuestlineGraphEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
    FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

    InTabManager->RegisterTabSpawner(
        GraphViewportTabId,
        FOnSpawnTab::CreateSP(this, &FQuestlineGraphEditor::SpawnGraphViewportTab))
        .SetDisplayName(NSLOCTEXT("SimpleQuestEditor", "GraphViewportTab", "Quest Graph Editor"));

    InTabManager->RegisterTabSpawner(
        DetailsTabId,
        FOnSpawnTab::CreateSP(this, &FQuestlineGraphEditor::SpawnDetailsTab))
        .SetDisplayName(NSLOCTEXT("SimpleQuestEditor", "DetailsTab", "Details"));

    InTabManager->RegisterTabSpawner(
        HierarchyTabId,
        FOnSpawnTab::CreateSP(this, &FQuestlineGraphEditor::SpawnHierarchyTab))
        .SetDisplayName(NSLOCTEXT("SimpleQuestEditor", "HierarchyTab", "Quest Tag Hierarchy"));
}

void FQuestlineGraphEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
    FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
    InTabManager->UnregisterTabSpawner(GraphViewportTabId);
    InTabManager->UnregisterTabSpawner(DetailsTabId);
    InTabManager->UnregisterTabSpawner(HierarchyTabId);
}

TSharedRef<SDockTab> FQuestlineGraphEditor::SpawnGraphViewportTab(const FSpawnTabArgs& Args)
{
    GraphPanelContainer = SNew(SBox);

    SAssignNew(BreadcrumbTrail, SBreadcrumbTrail<UEdGraph*>)
        .OnCrumbClicked(this, &FQuestlineGraphEditor::OnBreadcrumbClicked);

    NavigateTo(QuestlineGraph->QuestlineEdGraph);

    return SNew(SDockTab)
        .Label(NSLOCTEXT("SimpleQuestEditor", "GraphViewportTabLabel", "Graph"))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(4.f, 2.f)
            [
                BreadcrumbTrail.ToSharedRef()
            ]
            + SVerticalBox::Slot()
            .FillHeight(1.f)
            [
                GraphPanelContainer.ToSharedRef()
            ]
        ];
}

SGraphEditor::FGraphEditorEvents FQuestlineGraphEditor::MakeGraphEvents()
{
    SGraphEditor::FGraphEditorEvents GraphEvents;
    GraphEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FQuestlineGraphEditor::OnGraphSelectionChanged);
    GraphEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FQuestlineGraphEditor::OnNodeDoubleClicked);
    return GraphEvents;
}

TSharedRef<SQuestlineGraphPanel> FQuestlineGraphEditor::CreateGraphEditorWidget()
{
    check(QuestlineGraph);
    check(QuestlineGraph->QuestlineEdGraph);

    return SNew(SQuestlineGraphPanel, QuestlineGraph->QuestlineEdGraph, GraphEditorCommands)
        .GraphEvents(MakeGraphEvents());
}

void FQuestlineGraphEditor::BindGraphCommands()
{
    FGraphEditorCommands::Register();

    GraphEditorCommands = MakeShared<FUICommandList>();
    GraphEditorCommands->MapAction(
        FGenericCommands::Get().Delete,
        FExecuteAction::CreateSP(this, &FQuestlineGraphEditor::DeleteSelectedNodes));
    
    GraphEditorCommands->MapAction(
       FQuestlineGraphEditorCommands::Get().CompileQuestlineGraph,
       FExecuteAction::CreateSP(this, &FQuestlineGraphEditor::CompileQuestlineGraph));

    GraphEditorCommands->MapAction(
    FQuestlineGraphEditorCommands::Get().NavigateBack,
    FExecuteAction::CreateSP(this, &FQuestlineGraphEditor::NavigateBack),
    FCanExecuteAction::CreateSP(this, &FQuestlineGraphEditor::CanNavigateBack));

    GraphEditorCommands->MapAction(
        FQuestlineGraphEditorCommands::Get().NavigateForward,
        FExecuteAction::CreateSP(this, &FQuestlineGraphEditor::NavigateForward),
        FCanExecuteAction::CreateSP(this, &FQuestlineGraphEditor::CanNavigateForward));

}

void FQuestlineGraphEditor::DeleteSelectedNodes()
{
    if (!GraphEditorWidget.IsValid()) return;

    const FScopedTransaction Transaction(
        NSLOCTEXT("SimpleQuestEditor", "DeleteSelectedNodes", "Delete Selected Nodes"));

    QuestlineGraph->QuestlineEdGraph->Modify();

    for (UObject* Obj : GraphEditorWidget->GetGraphEditor()->GetSelectedNodes())
    {
        UEdGraphNode* Node = Cast<UEdGraphNode>(Obj);
        if (Node && Node->CanUserDeleteNode())
        {
            Node->Modify();
            Node->GetSchema()->BreakNodeLinks(*Node);
            QuestlineGraph->QuestlineEdGraph->RemoveNode(Node);
        }
    }
}

void FQuestlineGraphEditor::CompileQuestlineGraph()
{
    TUniquePtr<FQuestlineGraphCompiler> Compiler = ISimpleQuestEditorModule::Get().CreateCompiler();
    const bool bSuccess = Compiler->Compile(QuestlineGraph);

    const FText Notification = bSuccess
        ? NSLOCTEXT("SimpleQuestEditor", "CompileSuccess", "Questline compiled successfully.")
        : NSLOCTEXT("SimpleQuestEditor", "CompileFailed", "Questline compilation failed. Check the Output Log for details.");

    FNotificationInfo Info(Notification);
    Info.ExpireDuration = 3.f;
    Info.bUseSuccessFailIcons = true;
    FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(bSuccess ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);

    CompileStatus = bSuccess ? EQuestlineCompileStatus::UpToDate : EQuestlineCompileStatus::Error;

    if (bSuccess && HierarchyPanel.IsValid()) HierarchyPanel->Refresh();
}

void FQuestlineGraphEditor::SaveAsset_Execute()
{
    CompileQuestlineGraph();
    FAssetEditorToolkit::SaveAsset_Execute();
}

void FQuestlineGraphEditor::ExtendToolbar()
{
    TSharedPtr<FExtender> ToolbarExtender = MakeShared<FExtender>();
    ToolbarExtender->AddToolBarExtension(
        "Asset",
        EExtensionHook::After,
        GraphEditorCommands,
        FToolBarExtensionDelegate::CreateSP(this, &FQuestlineGraphEditor::FillToolbar));
    AddToolbarExtender(ToolbarExtender);
}

void FQuestlineGraphEditor::FillToolbar(FToolBarBuilder& ToolbarBuilder)
{
    ToolbarBuilder.BeginSection("Compile");
    ToolbarBuilder.AddToolBarButton(FQuestlineGraphEditorCommands::Get().CompileQuestlineGraph, NAME_None, TAttribute<FText>(), TAttribute<FText>(),
        TAttribute<FSlateIcon>::CreateLambda(
            [this]() -> FSlateIcon
            {
                return GetCompileStatusIcon();
            })
        );
    ToolbarBuilder.EndSection();

    ToolbarBuilder.BeginSection("Navigation");
    ToolbarBuilder.AddToolBarButton(FQuestlineGraphEditorCommands::Get().NavigateBack,    NAME_None, INVTEXT("←"), TAttribute<FText>(), FSlateIcon(FAppStyle::GetAppStyleSetName(), "EditorViewport.PreviousView"));
    ToolbarBuilder.AddToolBarButton(FQuestlineGraphEditorCommands::Get().NavigateForward, NAME_None, INVTEXT("→"), TAttribute<FText>(), FSlateIcon(FAppStyle::GetAppStyleSetName(), "EditorViewport.NextView"));
    ToolbarBuilder.EndSection();

}

FText FQuestlineGraphEditor::GetGraphDisplayName(UEdGraph* Graph) const
{
    if (Graph == QuestlineGraph->QuestlineEdGraph) return FText::FromString(QuestlineGraph->GetName());

    // Inner graph — find the Quest node that owns it
    if (UObject* Outer = Graph->GetOuter())
    {
        if (UQuestlineNode_Quest* QuestNode = Cast<UQuestlineNode_Quest>(Outer))
        {
            return QuestNode->GetNodeTitle(ENodeTitleType::FullTitle);
        }
    }
    return FText::FromString(Graph->GetName());
}

void FQuestlineGraphEditor::OnGraphChanged(const FEdGraphEditAction&)
{
    CompileStatus = EQuestlineCompileStatus::Unknown;
}

FSlateIcon FQuestlineGraphEditor::GetCompileStatusIcon() const
{
    static const FName Background("Blueprint.CompileStatus.Background");
    static const FName Unknown("Blueprint.CompileStatus.Overlay.Unknown");
    static const FName Good("Blueprint.CompileStatus.Overlay.Good");
    static const FName Error("Blueprint.CompileStatus.Overlay.Error");

    switch (CompileStatus)
    {
    case EQuestlineCompileStatus::UpToDate:
        return FSlateIcon(FAppStyle::GetAppStyleSetName(), Background, NAME_None, Good);
    case EQuestlineCompileStatus::Error:
        return FSlateIcon(FAppStyle::GetAppStyleSetName(), Background, NAME_None, Error);
    default:
        return FSlateIcon(FAppStyle::GetAppStyleSetName(), Background, NAME_None, Unknown);
    }
}


TSharedRef<SDockTab> FQuestlineGraphEditor::SpawnDetailsTab(const FSpawnTabArgs& Args)
{
    return SNew(SDockTab)
        .Label(NSLOCTEXT("SimpleQuestEditor", "DetailsTabLabel", "Details"))
        [
            DetailsView.IsValid() ? DetailsView.ToSharedRef() : SNullWidget::NullWidget
        ];
}

void FQuestlineGraphEditor::OnGraphSelectionChanged(const FGraphPanelSelectionSet& SelectedNodes)
{
    if (!DetailsView.IsValid()) return;

    TArray<UObject*> Selected;
    for (UObject* Obj : SelectedNodes)
        Selected.Add(Obj);

    DetailsView->SetObjects(Selected);
}

TSharedRef<SDockTab> FQuestlineGraphEditor::SpawnHierarchyTab(const FSpawnTabArgs& Args)
{
    HierarchyPanel = SNew(SQuestlineHierarchyPanel, QuestlineGraph);

    return SNew(SDockTab)
        .Label(NSLOCTEXT("SimpleQuestEditor", "HierarchyTabLabel", "Hierarchy"))
        [
            HierarchyPanel.ToSharedRef()
        ];
}

void FQuestlineGraphEditor::NavigateTo(UEdGraph* Graph)
{
    if (!bIsNavigatingHistory) GraphForwardStack.Empty();
    GraphBackwardStack.Add(Graph);
    GraphChangedHandles.Add(Graph->AddOnGraphChangedHandler(FOnGraphChanged::FDelegate::CreateSP(this, &FQuestlineGraphEditor::OnGraphChanged)));

    TSharedRef<SQuestlineGraphPanel> Panel = SNew(SQuestlineGraphPanel, Graph, GraphEditorCommands).GraphEvents(MakeGraphEvents());
    GraphEditorWidget = Panel;
    GraphPanelContainer->SetContent(Panel);

    // Rebuild breadcrumb trail from full navigation stack
    if (BreadcrumbTrail.IsValid())
    {
        BreadcrumbTrail->ClearCrumbs();
        for (UEdGraph* StackGraph : GraphBackwardStack)
        {
            BreadcrumbTrail->PushCrumb(GetGraphDisplayName(StackGraph), StackGraph);
        }
    }
}

void FQuestlineGraphEditor::NavigateBack()
{
    if (GraphBackwardStack.Num() <= 1) return;
    UEdGraph* Leaving = GraphBackwardStack.Pop();
    Leaving->RemoveOnGraphChangedHandler(GraphChangedHandles.Pop());
    GraphForwardStack.Add(Leaving);                                                     // Save for forward navigation
    TGuardValue<bool> Guard(bIsNavigatingHistory, true);
    NavigateTo(GraphBackwardStack.Pop());                                         // re-navigate to previous (re-adds it)
}

void FQuestlineGraphEditor::NavigateForward()
{
    if (GraphForwardStack.Num() == 0) return;
    UEdGraph* Next = GraphForwardStack.Pop();
    TGuardValue Guard(bIsNavigatingHistory, true);
    NavigateTo(Next);
}

void FQuestlineGraphEditor::OnBreadcrumbClicked(UEdGraph* const& Graph)
{
    // Pop everything above the clicked graph
    while (GraphBackwardStack.Num() > 0 && GraphBackwardStack.Last() != Graph)
    {
        UEdGraph* Leaving = GraphBackwardStack.Pop();
        Leaving->RemoveOnGraphChangedHandler(GraphChangedHandles.Pop());
        GraphForwardStack.Add(Leaving);
    }
    if (GraphBackwardStack.Num() > 0)
    {
        UEdGraph* Target = GraphBackwardStack.Pop();
        Target->RemoveOnGraphChangedHandler(GraphChangedHandles.Pop());
        TGuardValue Guard(bIsNavigatingHistory, true);
        NavigateTo(Target);  // re-adds it cleanly
    }
}


void FQuestlineGraphEditor::OnNodeDoubleClicked(UEdGraphNode* Node)
{
    if (UQuestlineNode_Quest* QuestNode = Cast<UQuestlineNode_Quest>(Node))
    {
        if (QuestNode->GetInnerGraph())
        {
            NavigateTo(QuestNode->GetInnerGraph());
        }
    }
}
