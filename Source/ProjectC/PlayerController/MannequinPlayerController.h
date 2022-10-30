// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MannequinPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTC_API AMannequinPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SetHUDHealth(float Health, float MaxHealth);
	
protected:
	virtual void BeginPlay() override;
	
private:
	class AMannequinHUD* MannequinHUD;
};
