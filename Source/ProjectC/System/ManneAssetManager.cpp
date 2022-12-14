
#include "ManneAssetManager.h"

#include "AbilitySystemGlobals.h"

UManneAssetManager::UManneAssetManager()
{
}

void UManneAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	UAbilitySystemGlobals::Get().InitGlobalData();
}

UManneAssetManager& UManneAssetManager::Get()
{
	UManneAssetManager* ManneAssetManager = Cast<UManneAssetManager>(GEngine->AssetManager);
	if (ManneAssetManager)
	{
		return *ManneAssetManager;
	}
	UE_LOG(LogTemp, Warning, TEXT("Invalid AssetManager in DefaultEngine.ini"))
	return *NewObject<UManneAssetManager>();
}
