// Copyright 2026, Greg Bussell, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UQuestlineGraph;
class UQuestlineNode_Quest;
class UEdGraphPin;

/**
 * Compiles a UQuestlineGraph asset, walking the graph structure and writing derived data to each Quest CDO. Intended to
 * be subclassed by external modules that need to extend or replace the default compilation behavior.
 *
 * Compilation runs in two phases:
 * - Phase 1 - Inner step graphs: populates QuestSteps on each Quest CDO (stub, pending inner graph scaffolding)
 * - Phase 2 - Outer questline graph: populates inter-quest relationships, generates Gameplay Tags and GUIDs
 *
 * Use ISimpleQuestEditorModule::RegisterCompilerFactory to provide a custom subclass.
 * @see ISimpleQuestEditorModule::RegisterCompilerFactory
 */
class SIMPLEQUESTEDITOR_API FQuestlineGraphCompiler
{
public:
	virtual ~FQuestlineGraphCompiler() = default;

	/** Entry point. Runs Phase 1 then Phase 2. Returns true if compilation succeeded with no errors. */
	virtual bool Compile(UQuestlineGraph* InGraph);

protected:
	/** Phase 1: populate QuestSteps on each Quest CDO from inner step graphs. Stub — not yet implemented. */
	virtual void CompileInnerGraphs(UQuestlineGraph* InGraph);

	/** Phase 2: walk the outer graph, validate, generate tags, write CDO data. */
	virtual void CompileOuterGraph(UQuestlineGraph* InGraph);

	/**
	 * Follows an output pin through any number of knot nodes and collects all terminal UQuestlineNode_Quest nodes at
	 * the other end.
	 */	
	virtual void ResolveConnections(UEdGraphPin* FromPin, TArray<UQuestlineNode_Quest*>& OutNodes);
	
	/**
	 * Sanitizes a designer-entered node label into a valid Gameplay Tag segment. Replaces spaces and invalid characters
	 * with underscores, removes leading/trailing whitespace.
	 */
	virtual FString SanitizeTagSegment(const FString& InLabel) const;

	/** Logs a compile error and sets the internal error flag. */
	void AddError(const FString& Message);

	/** Logs a compile warning without setting the internal error flag. */
	void AddWarning(const FString& Message);

	bool bHasErrors = false;
};
