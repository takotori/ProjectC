
#include "TeamMatchGameMode.h"

#include "Kismet/GameplayStatics.h"
#include "ProjectC/GameState/MannequinGameState.h"
#include "ProjectC/PlayerController/MannequinPlayerController.h"
#include "ProjectC/PlayerState/MannequinPlayerState.h"

ATeamMatchGameMode::ATeamMatchGameMode()
{
	bTeamsMatch = true;
}

void ATeamMatchGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	AMannequinGameState* MatchGameState = Cast<AMannequinGameState>(UGameplayStatics::GetGameState(this));
	if (MatchGameState)
	{
		AMannequinPlayerState* MPState = NewPlayer->GetPlayerState<AMannequinPlayerState>();
		if (MPState && MPState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (MatchGameState->BlueTeam.Num() >= MatchGameState->RedTeam.Num())
			{
				MatchGameState->RedTeam.AddUnique(MPState);
				MPState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				MatchGameState->BlueTeam.AddUnique(MPState);
				MPState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamMatchGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	AMannequinGameState* MatchGameState = Cast<AMannequinGameState>(UGameplayStatics::GetGameState(this));
	AMannequinPlayerState* MPState = Exiting->GetPlayerState<AMannequinPlayerState>();

	if (MatchGameState && MPState)
	{
		if (MatchGameState->RedTeam.Contains(MPState))
		{
			MatchGameState->RedTeam.Remove(MPState);
		}
		if (MatchGameState->BlueTeam.Contains(MPState))
		{
			MatchGameState->BlueTeam.Remove(MPState);
		}
	}
}

void ATeamMatchGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	AMannequinGameState*MatchGameState = Cast<AMannequinGameState>(UGameplayStatics::GetGameState(this));
	if (MatchGameState)
	{
		for (auto PState : MatchGameState->PlayerArray)
		{
			AMannequinPlayerState* MPState = Cast<AMannequinPlayerState>(PState.Get());
			if (MPState && MPState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (MatchGameState->BlueTeam.Num() >= MatchGameState->RedTeam.Num())
				{
					MatchGameState->RedTeam.AddUnique(MPState);
					MPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					MatchGameState->BlueTeam.AddUnique(MPState);
					MPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}

float ATeamMatchGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	AMannequinPlayerState* AttackerPState = Attacker->GetPlayerState<AMannequinPlayerState>();
	AMannequinPlayerState* VictimPState = Victim->GetPlayerState<AMannequinPlayerState>();
	if (AttackerPState == nullptr && VictimPState == nullptr) return BaseDamage;
	if (AttackerPState == VictimPState) return BaseDamage;

	if (AttackerPState->GetTeam() == VictimPState->GetTeam())
	{
		return 0.f;
	}
	return BaseDamage;
}

void ATeamMatchGameMode::PlayerEliminated(AMannequinCharacter* EliminatedCharacter,
	AMannequinPlayerController* VictimController, AMannequinPlayerController* AttackerController)
{
	Super::PlayerEliminated(EliminatedCharacter, VictimController, AttackerController);
	AMannequinGameState* MGameState = Cast<AMannequinGameState>(UGameplayStatics::GetGameState(this));
	AMannequinPlayerState* AttackerPlayerState = AttackerController ? Cast<AMannequinPlayerState>(AttackerController->PlayerState) : nullptr;
	if (MGameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			MGameState->BlueTeamScores();
		}
		if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			MGameState->RedTeamScores();
		}
	}
}
