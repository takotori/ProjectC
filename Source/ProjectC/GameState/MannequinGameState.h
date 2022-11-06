#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MannequinGameState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTC_API AMannequinGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(class AMannequinPlayerState* ScoringPlayer);
	
	UPROPERTY(Replicated)
	TArray<AMannequinPlayerState*> TopScoringPlayers;

private:
	float TopScore = 0.f;
};
