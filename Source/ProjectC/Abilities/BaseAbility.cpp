
#include "BaseAbility.h"

AMannequinCharacter* UBaseAbility::GetCharacterFromActorInfo() const
{
	return CurrentActorInfo ? Cast<AMannequinCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
}
