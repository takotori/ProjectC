
#include "BaseGameplayTags.h"

#include "GameplayTagsManager.h"

FBaseGameplayTags FBaseGameplayTags::GameplayTags;

void FBaseGameplayTags::InitializeNativeTags()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	GameplayTags.AddAllTags(Manager);

	// Notify manager that we are done adding native tags.
	Manager.DoneAddingNativeTags();
}

FGameplayTag FBaseGameplayTags::FindTagByString(FString TagString, bool bMatchPartialString)
{
	const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

	if (!Tag.IsValid() && bMatchPartialString)
	{
		FGameplayTagContainer AllTags;
		Manager.RequestAllGameplayTags(AllTags, true);

		for (const FGameplayTag TestTag : AllTags)
		{
			if (TestTag.ToString().Contains(TagString))
			{
				Tag = TestTag;
				break;
			}
		}
	}

	return Tag;
}

void FBaseGameplayTags::AddAllTags(UGameplayTagsManager& Manager)
{
	AddTag(InputTag_Character_Look, "InputTag.Character.Look");
	AddTag(InputTag_Character_Move, "InputTag.Character.Move");
	
	AddTag(InputTag_Quit, "InputTag.Quit");

	AddTag(InputTag_Character_Jump, "InputTag.Character.Jump");
	AddTag(InputTag_Character_Crouch, "InputTag.Character.Crouch");
	
	AddTag(InputTag_Weapon_Fire, "InputTag.Weapon.Fire");
}

void FBaseGameplayTags::AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName)
{
	OutTag = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TagName));
}

void FBaseGameplayTags::AddMovementModeTag(FGameplayTag& OutTag, const ANSICHAR* TagName, uint8 MovementMode)
{
}

void FBaseGameplayTags::AddCustomMovementModeTag(FGameplayTag& OutTag, const ANSICHAR* TagName,
	uint8 CustomMovementMode)
{
}
