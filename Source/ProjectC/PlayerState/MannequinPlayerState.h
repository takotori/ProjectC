#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ProjectC/Cards/BaseCard.h"
#include "ProjectC/Types/Team.h"
#include "MannequinPlayerState.generated.h"

UCLASS()
class PROJECTC_API AMannequinPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// Replication notifies
	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Defeats();

	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatsAmount);

private:
	UPROPERTY()
	class AMannequinCharacter* Character;

	UPROPERTY()
	class AMannequinPlayerController* Controller;

	UPROPERTY()
	TArray<ABaseCard*> Deck;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;

	UPROPERTY(Replicated)
	ETeam Team = ETeam::ET_NoTeam;

public:
	FORCEINLINE ETeam GetTeam() const { return Team; }
	FORCEINLINE void SetTeam(ETeam TeamToSet) { Team = TeamToSet; }
};
