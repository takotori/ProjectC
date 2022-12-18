#pragma once

#include "Containers/Map.h"
#include "GameplayTagContainer.h"
#include "HAL/Platform.h"

class UGameplayTagsManager;

/**
 * FLyraGameplayTags
 *
 *	Singleton containing native gameplay tags.
 */
struct FBaseGameplayTags
{
public:

	static const FBaseGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeTags();
	static FGameplayTag FindTagByString(FString TagString, bool bMatchPartialString = false);
	
	FGameplayTag InputTag_Character_Move;
	FGameplayTag InputTag_Character_Look;
	FGameplayTag InputTag_Quit;

	FGameplayTag InputTag_Character_Jump;
	FGameplayTag InputTag_Character_Crouch;
	
	FGameplayTag InputTag_Weapon_Fire;

	TMap<uint8, FGameplayTag> MovementModeTagMap;
	TMap<uint8, FGameplayTag> CustomMovementModeTagMap;

protected:

	void AddAllTags(UGameplayTagsManager& Manager);
	void AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName);
	void AddMovementModeTag(FGameplayTag& OutTag, const ANSICHAR* TagName, uint8 MovementMode);
	void AddCustomMovementModeTag(FGameplayTag& OutTag, const ANSICHAR* TagName, uint8 CustomMovementMode);

private:
	static FBaseGameplayTags GameplayTags;
};
