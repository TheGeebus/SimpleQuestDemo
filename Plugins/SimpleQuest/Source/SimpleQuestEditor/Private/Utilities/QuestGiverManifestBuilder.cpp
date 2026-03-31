// Copyright 2026, Greg Bussell, All Rights Reserved.

#include "Utilities/QuestGiverManifestBuilder.h"
#include "Components/QuestGiverComponent.h"
#include "SimpleQuestLog.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "Subsystems/QuestManagerSubsystem.h"

void FQuestGiverManifestBuilder::RebuildManifest()
{
	if (!GEditor)
	{
		UE_LOG(LogSimpleQuest, Warning, TEXT("RebuildManifest called outside of editor context."));
		return;
	}
	UQuestManagerSubsystem* CDO = GetMutableDefault<UQuestManagerSubsystem>();
	if (!CDO)
	{
		UE_LOG(LogSimpleQuest, Warning, TEXT("QuestGiverManifestBuilder: Could not find UQuestManagerSubsystem CDO. "
			"Enable SimpleQuest plugin and set manager subsystem class in Project Settings > Plugins > SimpleQuest."));
		return;
	}

	CDO->QuestsWithGivers.Empty();
	
	for (const FWorldContext& Context : GEditor->GetWorldContexts())
	{
		UWorld* World = Context.World();
		if (!World) continue;

		for (TActorIterator<AActor> It(World); It; ++It) // iterating over a large world could be slow, but this is not done often and only in-editor
		{
			if (!IsValid(*It)) continue;

			if (UQuestGiverComponent* Giver = (*It)->FindComponentByClass<UQuestGiverComponent>())
			{
				for (const FGameplayTag& Tag : Giver->GetQuestTagsToGive())
				{
					if (Tag.IsValid()) CDO->QuestsWithGivers.Add(Tag);
				}
			}
		}
	}

	CDO->SaveConfig(CPF_Config, *GGameIni);

	UE_LOG(LogSimpleQuest, Log,	TEXT("QuestGiverManifestBuilder: Rebuilt manifest with %d quest(s) with givers."), CDO->QuestsWithGivers.Num());
}
