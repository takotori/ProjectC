#include "MannequinPlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Net/UnrealNetwork.h"
#include "ProjectC/Character/MannequinCharacter.h"
#include "ProjectC/HUD/CharacterOverlay.h"
#include "ProjectC/HUD/MannequinHUD.h"

void AMannequinPlayerController::BeginPlay()
{
	Super::BeginPlay();
	MannequinHUD = Cast<AMannequinHUD>(GetHUD());
}

void AMannequinPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SetHUDTime();
	CheckTimeSync(DeltaSeconds);
	PollInit();
}

void AMannequinPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (MannequinHUD && MannequinHUD->CharacterOverlay)
		{
			CharacterOverlay = MannequinHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
			}
		}
	}
}

void AMannequinPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMannequinPlayerController, MatchState)
}

void AMannequinPlayerController::CheckTimeSync(float DeltaSeconds)
{
	TimeSyncRunningTime += DeltaSeconds;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
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
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->HealthBar && MannequinHUD->
		CharacterOverlay->HealthText)
	{
		const float HealthPercent = Health / MaxHealth;
		MannequinHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		MannequinHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;
	}
}

void AMannequinPlayerController::SetHUDAmmo(int32 Ammo)
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->AmmoAmount)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		MannequinHUD->CharacterOverlay->AmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AMannequinPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->MatchCountdownText)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		MannequinHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AMannequinPlayerController::SetHUDTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());
	if (CountdownInt != SecondsLeft)
	{
		SetHUDMatchCountdown(MatchTime - GetServerTime());
	}
	CountdownInt = SecondsLeft;
}

void AMannequinPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AMannequinPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
                                                                       float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float AMannequinPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AMannequinPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AMannequinPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	if (MatchState == MatchState::InProgress)
	{
		MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
		if (MannequinHUD)
		{
			MannequinHUD->AddCharacterOverlay();
		}
	}
}

void AMannequinPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
		if (MannequinHUD)
		{
			MannequinHUD->AddCharacterOverlay();
		}
	}
}
