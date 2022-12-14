#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "ProjectC/ProjectC.h"
#include "ManneAssetManager.generated.h"

UCLASS()
class PROJECTC_API UManneAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	UManneAssetManager();

	virtual void StartInitialLoading() override;

	static const FPrimaryAssetType CharacterItemType;

	// Return the asset manager singleton class
	static UManneAssetManager& Get();
};
