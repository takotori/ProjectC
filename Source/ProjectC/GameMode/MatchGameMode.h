#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MatchGameMode.generated.h"

UCLASS()
class PROJECTC_API AMatchGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AMatchGameMode();
	virtual void Tick(float DeltaSeconds) override;
	virtual void PlayerEliminated(class AMannequinCharacter* EliminatedCharacter,
	                              class AMannequinPlayerController* VictimController,
	                              AMannequinPlayerController* AttackerController);

	virtual void RequestRespawn(class ACharacter* EliminatedCharacter, AController* EliminatedController);
	

	UPROPERTY(EditDefaultsOnly)
	float WarmUpTime = 10.f;

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	
private:
	float CountdownTime = 0.f;
};
