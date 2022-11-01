// Fill out your copyright notice in the Description page of Project Settings.


#include "MannequinPlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "ProjectC/Character/MannequinCharacter.h"
#include "ProjectC/HUD/CharacterOverlay.h"
#include "ProjectC/HUD/MannequinHUD.h"

void AMannequinPlayerController::BeginPlay()
{
	Super::BeginPlay();
	MannequinHUD = Cast<AMannequinHUD>(GetHUD());
}

void AMannequinPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	AMannequinCharacter* MannequinCharacter = Cast<AMannequinCharacter>(InPawn);

	if (MannequinCharacter)
	{
		SetHUDHealth(MannequinCharacter->GetHealth(), MannequinCharacter->GetMaxHealth());
	}
}
void AMannequinPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->HealthBar && MannequinHUD->CharacterOverlay->HealthText)
	{
		const float HealthPercent = Health / MaxHealth;
		MannequinHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		MannequinHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}
void AMannequinPlayerController::SetHUDScore(float Score)
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->ScoreAmount)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		MannequinHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
}

void AMannequinPlayerController::SetHUDDefeats(int32 Defeats)
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->DefeatsAmount)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		MannequinHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}

}
