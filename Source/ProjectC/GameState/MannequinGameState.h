#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MannequinGameState.generated.h"

UCLASS()
class PROJECTC_API AMannequinGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(class AMannequinPlayerState* ScoringPlayer);
	
	UPROPERTY(Replicated)
	TArray<AMannequinPlayerState*> TopScoringPlayers;

	// Teams
	UPROPERTY()
	TArray<AMannequinPlayerState*> RedTeam;

	UPROPERTY()
	TArray<AMannequinPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

	UFUNCTION()
	void OnRep_RedTeamScore();

	UFUNCTION()
	void OnRep_BlueTeamScore();
	
private:
	float TopScore = 0.f;
};
