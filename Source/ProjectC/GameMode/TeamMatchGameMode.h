#pragma once

#include "CoreMinimal.h"
#include "MatchGameMode.h"
#include "TeamMatchGameMode.generated.h"

UCLASS()
class PROJECTC_API ATeamMatchGameMode : public AMatchGameMode
{
	GENERATED_BODY()

public:
	ATeamMatchGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
	virtual void PlayerEliminated(class AMannequinCharacter* EliminatedCharacter,
							  class AMannequinPlayerController* VictimController,
							  AMannequinPlayerController* AttackerController) override;
	
protected:
	virtual void HandleMatchHasStarted() override;
};
