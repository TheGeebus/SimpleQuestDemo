// Copyright Epic Games, Inc. All Rights Reserved.

#include "SimpleQuest.h"
#include "GameplayTagsManager.h"
#include "SimpleQuestLog.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Misc/ConfigCacheIni.h"
#include "Quests/QuestlineGraph.h"

#define LOCTEXT_NAMESPACE "FSimpleQuestModule"

DEFINE_LOG_CATEGORY(LogSimpleQuest);

void FSimpleQuestModule::StartupModule()
{
	UGameplayTagsManager::OnLastChanceToAddNativeTags().AddStatic(&FSimpleQuestModule::RegisterCompiledQuestTags);
}

void FSimpleQuestModule::RegisterCompiledQuestTags()
{
	// Enumerate all UQuestlineGraph assets via the asset registry — no full loads
	IAssetRegistry& AR = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	TArray<FAssetData> Assets;
	AR.GetAssetsByClass(UQuestlineGraph::StaticClass()->GetClassPathName(), Assets);

	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	for (const FAssetData& Asset : Assets)
	{
		// Load the asset to read CompiledQuestTags — this is acceptable here
		// because it fires before DoneAddingNativeTags, at a defined startup point
		if (UQuestlineGraph* Graph = Cast<UQuestlineGraph>(Asset.GetAsset()))
		{
			for (const FName& TagName : Graph->GetCompiledQuestTags())
			{
				TagsManager.AddNativeGameplayTag(TagName);
			}
		}
	}
}

void FSimpleQuestModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSimpleQuestModule, SimpleQuest)