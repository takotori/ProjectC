
#include "MatchGameMode.h"

#include "ProjectC/Character/MannequinCharacter.h"

void AMatchGameMode::PlayerEliminated(AMannequinCharacter* EliminatedCharacter,
                                      AMannequinPlayerController* VictimController, AMannequinPlayerController* AttackerController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Elim();
	}
}
