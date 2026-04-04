// Copyright 2026, Greg Bussell, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "QuestlineGraph.generated.h"

class UQuestNodeBase;
class UEdGraph;
class UQuest;

/**
 * Authoring container for a questline, a directed graph of quest and step nodes. Owns a UEdGraph (QuestlineEdGraph) containing
 * the visual layout, and holds the compiler output used at runtime: entry node tags and the full node tag-to-class registry
 * (including nodes from linked questline assets inlined at compile time). This is the asset type the designer creates and
 * opens in the graph editor.
 */
UCLASS(BlueprintType)
class SIMPLEQUEST_API UQuestlineGraph : public UObject
{
    GENERATED_BODY()

    friend class FQuestlineGraphCompiler;
    friend class UQuestManagerSubsystem;

public:
    virtual void PostLoad() override;


private:
    /**
     * The tags representing the quests on this graph. Used for runtime lookups and event dispatch. Uses FName because we will
     * need to register these on PostLoad.
     */
    UPROPERTY()
    TArray<FName> CompiledQuestTags;

    /**
     * Tags of all content nodes directly reachable from this questline's Entry node. Populated by the compiler. Used by the
     * subsystem to know which nodes to activate when starting this questline.
     */
    UPROPERTY()
    TArray<FGameplayTag> EntryNodeTags;

    /**
     * All compiled node instances, keyed by tag. Owned by this asset. Populated by the compiler — includes nodes inlined from
     * linked questline graphs. The subsystem looks up and activates nodes directly from this map.
     */
    UPROPERTY()
    TMap<FGameplayTag, TObjectPtr<UQuestNodeBase>> CompiledNodes;

    /**
     * Identifier used as the Gameplay Tag scope for all quests in this questline. Must be unique across the project. Defaults
     * to the asset name if left empty. Override this when you need a stable tag namespace independent of the asset name,
     * or to disambiguate duplicate assets.
     *
     * Format: Quest.<QuestlineID>.<QuestNodeLabel>
     */
    UPROPERTY(EditAnywhere)
    FString QuestlineID;
    
    virtual void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const override;


public:
    const TArray<FGameplayTag>& GetEntryNodeTags() const { return EntryNodeTags; }
    const TMap<FGameplayTag, TObjectPtr<UQuestNodeBase>>& GetCompiledNodes() const { return CompiledNodes; }


    // Editor-only: the actual UEdGraph object is only needed in the editor. The data it represents is compiled in-editor for use at runtime
#if WITH_EDITORONLY_DATA
public:	
    /**
     * The questline graph object. Contains Quest nodes and the wiring between them.
     */
    UPROPERTY()
    TObjectPtr<UEdGraph> QuestlineEdGraph;

#endif

};