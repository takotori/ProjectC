#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MatchGameMode.generated.h"

UCLASS()
class PROJECTC_API AMatchGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(class AMannequinCharacter* EliminatedCharacter,
	                              class AMannequinPlayerController* VictimController,
	                              AMannequinPlayerController* AttackerController);
};
