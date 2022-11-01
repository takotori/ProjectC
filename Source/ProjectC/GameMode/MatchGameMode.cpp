
#include "MatchGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectC/Character/MannequinCharacter.h"
#include "ProjectC/PlayerController/MannequinPlayerController.h"
#include "ProjectC/PlayerState/MannequinPlayerState.h"

void AMatchGameMode::PlayerEliminated(AMannequinCharacter* EliminatedCharacter,
                                      AMannequinPlayerController* VictimController, AMannequinPlayerController* AttackerController)
{
	AMannequinPlayerState* AttackerPlayerState = AttackerController ? Cast<AMannequinPlayerState>(AttackerController->PlayerState) : nullptr;
	AMannequinPlayerState* VictimPlayerState = VictimController ? Cast<AMannequinPlayerState>(VictimController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Elim();
		
	}
}

void AMatchGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}
	if (EliminatedController)
	{
		// remove random respawning
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(EliminatedController, PlayerStarts[Selection]);
	}
}
