// Copyright 2026, Greg Bussell, All Rights Reserved.

#include "Quests/QuestlineGraph.h"
#include "GameplayTagsManager.h"
#include "UObject/AssetRegistryTagsContext.h"

void UQuestlineGraph::PostLoad()
{
	Super::PostLoad();

	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	for (const FName& TagName : CompiledQuestTags)
	{
		TagsManager.AddNativeGameplayTag(TagName);
	}
}

void UQuestlineGraph::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
	UObject::GetAssetRegistryTags(Context);
	const FString EffectiveID = QuestlineID.IsEmpty() ? GetName() : QuestlineID;
	Context.AddTag(FAssetRegistryTag(TEXT("QuestlineEffectiveID"),	EffectiveID, FAssetRegistryTag::TT_Alphabetical));
}
