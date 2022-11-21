#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MatchGameMode.generated.h"

namespace MatchState
{
	extern PROJECTC_API const FName Cooldown; // Match duration has been reached. Display winner and begin cooldown timer
}

UCLASS()
class PROJECTC_API AMatchGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AMatchGameMode();
	virtual void Tick(float DeltaSeconds) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);
	virtual void PlayerEliminated(class AMannequinCharacter* EliminatedCharacter,
	                              class AMannequinPlayerController* VictimController,
	                              AMannequinPlayerController* AttackerController);

	virtual void RequestRespawn(class ACharacter* EliminatedCharacter, AController* EliminatedController);

	void PlayerLeftGame(class AMannequinPlayerState* PlayerLeaving);
	
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;
	
	UPROPERTY(EditDefaultsOnly)
	float WarmUpTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	
private:
	float CountdownTime = 0.f;
};
