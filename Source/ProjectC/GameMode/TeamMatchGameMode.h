#pragma once

#include "CoreMinimal.h"
#include "MatchGameMode.h"
#include "TeamMatchGameMode.generated.h"

UCLASS()
class PROJECTC_API ATeamMatchGameMode : public AMatchGameMode
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	
protected:
	virtual void HandleMatchHasStarted() override;
};
