#include "MannequinPlayerController.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "ProjectC/Abilities/AttributeSets/CharacterAttributes.h"
#include "ProjectC/Character/MannequinCharacter.h"
#include "ProjectC/Components/CardAbilitySystemComponent.h"
#include "ProjectC/Components/CombatComponent.h"
#include "ProjectC/GameMode/MatchGameMode.h"
#include "ProjectC/GameState/MannequinGameState.h"
#include "ProjectC/HUD/Announcement.h"
#include "ProjectC/HUD/CardPickerWidget.h"
#include "ProjectC/HUD/CharacterOverlay.h"
#include "ProjectC/HUD/MannequinHUD.h"
#include "ProjectC/HUD/PauseMenu.h"
#include "ProjectC/PlayerState/MannequinPlayerState.h"
#include "ProjectC/Types/Announcement.h"

void AMannequinPlayerController::BeginPlay()
{
	Super::BeginPlay();
	MannequinHUD = Cast<AMannequinHUD>(GetHUD());
	ServerCheckMatchState();
}

void AMannequinPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMannequinPlayerController, MatchState)
	DOREPLIFETIME(AMannequinPlayerController, bShowTeamScores)
}

void AMannequinPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SetHUDTime();
	CheckTimeSync(DeltaSeconds);
	PollInit();
	CheckPing(DeltaSeconds);
}

void AMannequinPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (const AMannequinCharacter* MannequinCharacter = Cast<AMannequinCharacter>(InPawn))
	{
		SetHUDHealth(MannequinCharacter->GetHealth(), MannequinCharacter->GetMaxHealth());
	}
}

void AMannequinPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;
	//todo
	InputComponent->BindAction("Quit", IE_Pressed, this, &AMannequinPlayerController::ShowPauseMenu);
}

void AMannequinPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AMannequinPlayerController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	MatchState = State;
	if (MatchState == MatchState::PickingCards)
	{
		HandlePickingCards();
	}
	else if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AMannequinPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::PickingCards)
	{
		HandlePickingCards();
	}
	else if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AMannequinPlayerController::HandlePickingCards()
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD)
	{
		if (MannequinHUD->Announcement)
		{
			MannequinHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		if (MannequinHUD->CardPicker == nullptr)
		{
			// probably doesn't work a second time, because CardPicker is not nullptr anymore
			MannequinHUD->AddCardPicker();
			MannequinHUD->CardPicker->MenuSetup();
		}
	}
}

void AMannequinPlayerController::HandleMatchHasStarted(bool bTeamsMatch)
{
	if (HasAuthority()) bShowTeamScores = bTeamsMatch;
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD)
	{
		if (MannequinHUD->CharacterOverlay == nullptr) MannequinHUD->AddCharacterOverlay();
		// if (MannequinHUD->CardPicker == nullptr) MannequinHUD->AddCardPicker();

		if (MannequinHUD->Announcement)
		{
			MannequinHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}

		if (!HasAuthority()) return;
		bTeamsMatch ? InitTeamScores() : HideTeamScores();
	}
}

void AMannequinPlayerController::HandleCooldown()
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD)
	{
		MannequinHUD->CharacterOverlay->RemoveFromParent();
		if (MannequinHUD->Announcement)
		{
			MannequinHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			MannequinHUD->Announcement->AnnouncementText->SetText(FText::FromString(Announcement::NewMatchStartsIn));

			const AMannequinGameState* MannequinGameState = Cast<AMannequinGameState>(
				UGameplayStatics::GetGameState(this));
			const AMannequinPlayerState* MannequinPlayerState = GetPlayerState<AMannequinPlayerState>();
			if (MannequinGameState && MannequinPlayerState)
			{
				const TArray<AMannequinPlayerState*> TopPlayers = MannequinGameState->TopScoringPlayers;
				const FString InfoTextString = bShowTeamScores
					                               ? GetTeamsInfoText(MannequinGameState)
					                               : GetInfoText(TopPlayers);
				MannequinHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	if (AMannequinCharacter* MannequinCharacter = Cast<AMannequinCharacter>(GetPawn()); MannequinCharacter &&
		MannequinCharacter->GetCombat())
	{
		MannequinCharacter->bDisableGameplay = true;
		MannequinCharacter->GetCombat()->FireButtonPressed(false);
	}
}

FString AMannequinPlayerController::GetInfoText(const TArray<AMannequinPlayerState*>& Players)
{
	const AMannequinPlayerState* MannequinPlayerState = GetPlayerState<AMannequinPlayerState>();
	if (MannequinPlayerState) return FString();
	FString InfoTextString;
	if (Players.Num() == 0)
	{
		InfoTextString = Announcement::NoWinner;
	}
	else if (Players.Num() == 1 && Players[0] == MannequinPlayerState)
	{
		InfoTextString = Announcement::YouAreWinner;
	}
	else if (Players.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *Players[0]->GetPlayerName());
	}
	else if (Players.Num() > 1)
	{
		InfoTextString = Announcement::TiedForWin;
		for (const auto TiedPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}
	return InfoTextString;
}

void AMannequinPlayerController::ServerCheckMatchState_Implementation()
{
	if (const AMatchGameMode* GameMode = Cast<AMatchGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		WarmupTime = GameMode->WarmUpTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, LevelStartingTime);
		if (MannequinHUD && MatchState == MatchState::WaitingToStart)
		{
			MannequinHUD->AddAnnouncement();
		}
	}
}

void AMannequinPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match,
                                                                  float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if (MannequinHUD && MatchState == MatchState::WaitingToStart)
	{
		MannequinHUD->AddAnnouncement();
	}
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

void AMannequinPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	const float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AMannequinPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
                                                                       float TimeServerReceivedClientRequest)
{
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	const float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float AMannequinPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AMannequinPlayerController::OnRep_ShowTeamScores()
{
	bShowTeamScores ? InitTeamScores() : HideTeamScores();
}

void AMannequinPlayerController::HideTeamScores()
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->RedTeamScore && MannequinHUD->
		CharacterOverlay->BlueTeamScore && MannequinHUD->CharacterOverlay->ScoreSpacerText)
	{
		MannequinHUD->CharacterOverlay->RedTeamScore->SetText(FText());
		MannequinHUD->CharacterOverlay->BlueTeamScore->SetText(FText());
		MannequinHUD->CharacterOverlay->ScoreSpacerText->SetText(FText());
	}
}

void AMannequinPlayerController::InitTeamScores()
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->RedTeamScore && MannequinHUD->
		CharacterOverlay->BlueTeamScore && MannequinHUD->CharacterOverlay->ScoreSpacerText)
	{
		const FString Zero("0");
		const FString Spacer("|");
		MannequinHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(Zero));
		MannequinHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(Zero));
		MannequinHUD->CharacterOverlay->ScoreSpacerText->SetText(FText::FromString(Spacer));
	}
	else
	{
		bInitializeTeamScores = true;
	}
}

FString AMannequinPlayerController::GetTeamsInfoText(const AMannequinGameState* MannequinGameState)
{
	if (MannequinGameState) return FString();
	FString InfoTextString;
	const int32 RedTeamScore = MannequinGameState->RedTeamScore;
	const int32 BlueTeamScore = MannequinGameState->BlueTeamScore;
	if (RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoTextString = Announcement::NoWinner;
	}
	else if (RedTeamScore == BlueTeamScore)
	{
		InfoTextString = Announcement::TeamsTiedForWin;
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		InfoTextString = Announcement::RedTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("Red Team: %d\n"), RedTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("Blue Team: %d\n"), BlueTeamScore));
	}
	else if (BlueTeamScore > RedTeamScore)
	{
		InfoTextString = Announcement::BlueTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("Blue Team: %d\n"), BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("Red Team: %d\n"), RedTeamScore));
	}
	return InfoTextString;
}

void AMannequinPlayerController::ShowPauseMenu()
{
	if (PauseMenuWidget == nullptr) return;
	if (PauseMenu == nullptr)
	{
		PauseMenu = CreateWidget<UPauseMenu>(this, PauseMenuWidget);
	}
	if (PauseMenu)
	{
		bPauseMenuOpen = !bPauseMenuOpen;
		bPauseMenuOpen ? PauseMenu->MenuSetup() : PauseMenu->MenuTearDown();
	}
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
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if (bInitializeAmmo) SetHUDAmmo(HUDWeaponAmmo);
				if (bInitializeTeamScores) InitTeamScores();
				if (const AMannequinCharacter* MannequinCharacter = Cast<AMannequinCharacter>(GetPawn());
					MannequinCharacter && MannequinCharacter->GetCombat())
				{
					if (bInitializeGrenades) SetHUDGrenades(MannequinCharacter->GetCombat()->GetGrenades());
				}
			}
		}
	}
}

void AMannequinPlayerController::UpdateHUD(FGameplayAttribute Attribute,float OldValue, float NewValue)
{
	if (Attribute.IsValid())
	{
			
	}
	UE_LOG(LogTemp, Warning, TEXT("asdöfj"))
}

void AMannequinPlayerController::HealthChanged(float OldHealth, float NewHealth)
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	const AMannequinPlayerState* MannequinPlayerState = GetPlayerState<AMannequinPlayerState>();
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->HealthBar && MannequinHUD->
		CharacterOverlay->HealthText && MannequinPlayerState && MannequinPlayerState->CharacterAttributeSet)
	{
		const float HealthPercent = NewHealth / MannequinPlayerState->CharacterAttributeSet->GetMaxHealth();
		MannequinHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		const FString HealthText =FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(NewHealth), FMath::CeilToInt(MannequinPlayerState->CharacterAttributeSet->GetMaxHealth()));
		MannequinHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void AMannequinPlayerController::MaxHealthChanged(float OldMaxHealth, float NewMaxHealth)
{
}

void AMannequinPlayerController::ShieldChanged(float OldShield, float NewShield)
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	const AMannequinPlayerState* MannequinPlayerState = GetPlayerState<AMannequinPlayerState>();
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->HealthBar && MannequinHUD->
		CharacterOverlay->HealthText && MannequinPlayerState && MannequinPlayerState->CharacterAttributeSet)
	{
		const float ShieldPercent = NewShield / MannequinPlayerState->CharacterAttributeSet->GetMaxShield();
		MannequinHUD->CharacterOverlay->HealthBar->SetPercent(ShieldPercent);
		const FString ShieldText =FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(NewShield), FMath::CeilToInt(MannequinPlayerState->CharacterAttributeSet->GetMaxShield()));
		MannequinHUD->CharacterOverlay->HealthText->SetText(FText::FromString(ShieldText));
	}
}

void AMannequinPlayerController::MaxShieldChanged(float OldMaxShield, float NewMaxShield)
{
}

void AMannequinPlayerController::AmmoChanged(int32 OldAmmo, int32 NewAmmo)
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->CharacterOverlay)
	{
		const FString AmmoText = FString::Printf(TEXT("%d"), NewAmmo);
		MannequinHUD->CharacterOverlay->AmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AMannequinPlayerController::MagazineChanged(float OldMagazine, float NewMagazine)
{
}

void AMannequinPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (UCardAbilitySystemComponent* Asc = GetAbilitySystemComponent())
	{
		Asc->ProcessAbilityInput(DeltaTime, bGamePaused);
	}
	Super::PostProcessInput(DeltaTime, bGamePaused);
}

UCardAbilitySystemComponent* AMannequinPlayerController::GetAbilitySystemComponent() const
{
	const AMannequinPlayerState* MannequinPlayerState = CastChecked<AMannequinPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
	return MannequinPlayerState ? MannequinPlayerState->GetAsc() : nullptr;
}

void AMannequinPlayerController::SetHUDTime()
{
	if (HasAuthority())
	{
		if (const AMatchGameMode* MatchGameMode = Cast<AMatchGameMode>(UGameplayStatics::GetGameMode(this)))
		{
			LevelStartingTime = MatchGameMode->LevelStartingTime;
		}
	}

	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress)
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

	const uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	CountdownInt = SecondsLeft;
}

void AMannequinPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->HealthBar && MannequinHUD->
		CharacterOverlay->HealthText)
	{
		const float HealthPercent = Health / MaxHealth;
		MannequinHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		const FString HealthText =
			FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		MannequinHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void AMannequinPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->ShieldBar && MannequinHUD->
		CharacterOverlay->ShieldText)
	{
		const float ShieldPercent = Shield / MaxShield;
		MannequinHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		const FString ShieldText =
			FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		MannequinHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void AMannequinPlayerController::SetHUDScore(float Score)
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->ScoreAmount)
	{
		const FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		MannequinHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void AMannequinPlayerController::SetHUDRedTeamScore(int32 RedScore)
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->RedTeamScore)
	{
		const FString ScoreText = FString::Printf(TEXT("%d"), RedScore);
		MannequinHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void AMannequinPlayerController::SetHUDBlueTeamScore(int32 BlueScore)
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->RedTeamScore)
	{
		const FString ScoreText = FString::Printf(TEXT("%d"), BlueScore);
		MannequinHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void AMannequinPlayerController::SetHUDDefeats(int32 Defeats)
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->DefeatsAmount)
	{
		const FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		MannequinHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void AMannequinPlayerController::SetHUDAmmo(int32 Ammo)
{
	// MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	// if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->AmmoAmount)
	// {
	// 	const FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
	// 	MannequinHUD->CharacterOverlay->AmmoAmount->SetText(FText::FromString(AmmoText));
	// }
	// else
	// {
	// 	bInitializeAmmo = true;
	// 	HUDWeaponAmmo = Ammo;
	// }
}

void AMannequinPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->MatchCountdownText)
	{
		const int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
		const int32 Seconds = CountdownTime - Minutes * 60;
		const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		MannequinHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AMannequinPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->Announcement && MannequinHUD->Announcement->WarmupTime)
	{
		const int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
		const int32 Seconds = CountdownTime - Minutes * 60;
		const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		MannequinHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void AMannequinPlayerController::SetHUDGrenades(int32 Grenades)
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->GrenadeAmount)
	{
		const FString GrenadeText = FString::Printf(TEXT("%d"), Grenades);
		MannequinHUD->CharacterOverlay->GrenadeAmount->SetText(FText::FromString(GrenadeText));
	}
	else
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
}

// Is the ping too high?
void AMannequinPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

void AMannequinPlayerController::CheckPing(float DeltaSeconds)
{
	HighPingRunningTime += DeltaSeconds;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if (PlayerState && PlayerState->GetCompressedPing() * 4 > HighPingThreshhold)
		{
			HighPingWarning();
			ServerReportPingStatus(true);
			PingAnimationRunningTime = 0.f;
		}
		else
		{
			ServerReportPingStatus(false);
		}
		HighPingRunningTime = 0.f;
	}

	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->HighPingAnimation &&
		MannequinHUD->CharacterOverlay->IsAnimationPlaying(MannequinHUD->CharacterOverlay->HighPingAnimation))
	{
		PingAnimationRunningTime += DeltaSeconds;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void AMannequinPlayerController::HighPingWarning()
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->HighPingImage && MannequinHUD
		->CharacterOverlay->HighPingAnimation)
	{
		MannequinHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		MannequinHUD->CharacterOverlay->PlayAnimation(MannequinHUD->CharacterOverlay->HighPingAnimation, 0.f, 10);
	}
}

void AMannequinPlayerController::StopHighPingWarning()
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->HighPingImage && MannequinHUD
		->CharacterOverlay->HighPingAnimation)
	{
		MannequinHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if (MannequinHUD->CharacterOverlay->IsAnimationPlaying(MannequinHUD->CharacterOverlay->HighPingAnimation))
		{
			MannequinHUD->CharacterOverlay->StopAnimation(MannequinHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}
