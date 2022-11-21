
#include "MannequinGameState.h"

#include "Net/UnrealNetwork.h"
#include "ProjectC/PlayerController/MannequinPlayerController.h"
#include "ProjectC/PlayerState/MannequinPlayerState.h"

void AMannequinGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMannequinGameState, TopScoringPlayers);
	DOREPLIFETIME(AMannequinGameState, RedTeamScore);
	DOREPLIFETIME(AMannequinGameState, BlueTeamScore);
}

void AMannequinGameState::UpdateTopScore(AMannequinPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}

}

void AMannequinGameState::RedTeamScores()
{
	RedTeamScore++;
	AMannequinPlayerController* MPlayer = Cast<AMannequinPlayerController>(GetWorld()->GetFirstPlayerController());
	if(MPlayer)
	{
		MPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void AMannequinGameState::BlueTeamScores()
{
	BlueTeamScore++;
	AMannequinPlayerController* MPlayer = Cast<AMannequinPlayerController>(GetWorld()->GetFirstPlayerController());
	if(MPlayer)
	{
		MPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void AMannequinGameState::OnRep_RedTeamScore()
{
	AMannequinPlayerController* MPlayer = Cast<AMannequinPlayerController>(GetWorld()->GetFirstPlayerController());
	if(MPlayer)
	{
		MPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void AMannequinGameState::OnRep_BlueTeamScore()
{
	AMannequinPlayerController* MPlayer = Cast<AMannequinPlayerController>(GetWorld()->GetFirstPlayerController());
	if(MPlayer)
	{
		MPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}
