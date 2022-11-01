#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
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

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;
};
