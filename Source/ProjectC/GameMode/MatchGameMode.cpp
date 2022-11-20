
#include "MatchGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectC/Character/MannequinCharacter.h"
#include "ProjectC/GameState/MannequinGameState.h"
#include "ProjectC/PlayerController/MannequinPlayerController.h"
#include "ProjectC/PlayerState/MannequinPlayerState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

AMatchGameMode::AMatchGameMode()
{
	bDelayedStart = true;
}

void AMatchGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmUpTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		} 
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmUpTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		} 
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmUpTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		} 
	}
}

void AMatchGameMode::BeginPlay()
{
	Super::BeginPlay();
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AMatchGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMannequinPlayerController* MannequinPlayer = Cast<AMannequinPlayerController>(*It);
		if (MannequinPlayer)
		{
			MannequinPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void AMatchGameMode::PlayerEliminated(AMannequinCharacter* EliminatedCharacter,
                                      AMannequinPlayerController* VictimController,
                                      AMannequinPlayerController* AttackerController)
{
	AMannequinPlayerState* AttackerPlayerState = AttackerController ? Cast<AMannequinPlayerState>(AttackerController->PlayerState) : nullptr;
	AMannequinPlayerState* VictimPlayerState = VictimController ? Cast<AMannequinPlayerState>(VictimController->PlayerState) : nullptr;
	AMannequinGameState* MannequinGameState = GetGameState<AMannequinGameState>();
	
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && GameState)
	{
		AttackerPlayerState->AddToScore(1.f);
		MannequinGameState->UpdateTopScore(AttackerPlayerState);
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	if (VictimPlayerState == AttackerPlayerState)
	{
		AttackerPlayerState->AddToScore(-1.f);
		if (AttackerPlayerState->GetScore() > 0)
		{
			MannequinGameState->UpdateTopScore(AttackerPlayerState);
		}
	}
	
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Elim(false);
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

void AMatchGameMode::PlayerLeftGame(AMannequinPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	AMannequinGameState* MannequinGameState = GetGameState<AMannequinGameState>();
	if (MannequinGameState && MannequinGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		MannequinGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	AMannequinCharacter* CharacterLeaving = Cast<AMannequinCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}
