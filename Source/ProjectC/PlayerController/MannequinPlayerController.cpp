#include "MannequinPlayerController.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "ProjectC/Character/MannequinCharacter.h"
#include "ProjectC/Components/CombatComponent.h"
#include "ProjectC/GameMode/MatchGameMode.h"
#include "ProjectC/GameState/MannequinGameState.h"
#include "ProjectC/HUD/Announcement.h"
#include "ProjectC/HUD/CharacterOverlay.h"
#include "ProjectC/HUD/MannequinHUD.h"
#include "ProjectC/HUD/PauseMenu.h"
#include "ProjectC/PlayerState/MannequinPlayerState.h"

void AMannequinPlayerController::BeginPlay()
{
	Super::BeginPlay();
	MannequinHUD = Cast<AMannequinHUD>(GetHUD());
	ServerCheckMatchState();
}

void AMannequinPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SetHUDTime();
	CheckTimeSync(DeltaSeconds);
	PollInit();
	CheckPing(DeltaSeconds);
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
		if (bPauseMenuOpen)
		{
			PauseMenu->MenuSetup();
		}
		else
		{
			PauseMenu->MenuTearDown();
		}
	}
}

void AMannequinPlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}

// Is the ping too high?
void AMannequinPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
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
				AMannequinCharacter* MannequinCharacter = Cast<AMannequinCharacter>(GetPawn());
				if (MannequinCharacter && MannequinCharacter->GetCombat())
				{
					if (bInitializeGrenades) SetHUDGrenades(MannequinCharacter->GetCombat()->GetGrenades());
				}
			}
		}
	}
}

void AMannequinPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMannequinPlayerController, MatchState)
	DOREPLIFETIME(AMannequinPlayerController, bShowTeamScores)
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

void AMannequinPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;
	InputComponent->BindAction("Quit", IE_Pressed, this, &AMannequinPlayerController::ShowPauseMenu);
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
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
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
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
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
		FString ScoreText = FString::Printf(TEXT("%d"), RedScore);
		MannequinHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void AMannequinPlayerController::SetHUDBlueTeamScore(int32 BlueScore)
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->RedTeamScore)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), BlueScore);
		MannequinHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));
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
		bInitializeDefeats = true;
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
	else
	{
		bInitializeAmmo = true;
		HUDWeaponAmmo = Ammo;
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

void AMannequinPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->Announcement && MannequinHUD->Announcement->WarmupTime)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		MannequinHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void AMannequinPlayerController::SetHUDGrenades(int32 Grenades)
{
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD && MannequinHUD->CharacterOverlay && MannequinHUD->CharacterOverlay->GrenadeAmount)
	{
		FString GrenadeText = FString::Printf(TEXT("%d"), Grenades);
		MannequinHUD->CharacterOverlay->GrenadeAmount->SetText(FText::FromString(GrenadeText));
	}
	else
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
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
		FString Zero("0");
		FString Spacer("|");
		MannequinHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(Zero));
		MannequinHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(Zero));
		MannequinHUD->CharacterOverlay->ScoreSpacerText->SetText(FText::FromString(Spacer));
	}
	else
	{
		bInitializeTeamScores = true;
	}
}

void AMannequinPlayerController::SetHUDTime()
{
	if (HasAuthority())
	{
		AMatchGameMode* MatchGameMode = Cast<AMatchGameMode>(UGameplayStatics::GetGameMode(this));
		if (MatchGameMode)
		{
			LevelStartingTime = MatchGameMode->LevelStartingTime;
		}
	}

	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress)
		TimeLeft = WarmupTime + MatchTime - GetServerTime() +
			LevelStartingTime;

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
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

void AMannequinPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AMannequinPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
                                                                       float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
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

void AMannequinPlayerController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	MatchState = State;
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AMannequinPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AMannequinPlayerController::HandleMatchHasStarted(bool bTeamsMatch)
{
	if (HasAuthority()) bShowTeamScores = bTeamsMatch;
	MannequinHUD = MannequinHUD == nullptr ? Cast<AMannequinHUD>(GetHUD()) : MannequinHUD;
	if (MannequinHUD)
	{
		if (MannequinHUD->CharacterOverlay == nullptr) MannequinHUD->AddCharacterOverlay();
		if (MannequinHUD->Announcement)
		{
			MannequinHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		if (!HasAuthority()) return;
		if (bTeamsMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
		}
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
			FString AnnouncementText("New Match starts in:");
			MannequinHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			AMannequinGameState* GameState = Cast<AMannequinGameState>(UGameplayStatics::GetGameState(this));
			AMannequinPlayerState* MannequinPlayerState = GetPlayerState<AMannequinPlayerState>();
			if (GameState && MannequinPlayerState)
			{
				TArray<AMannequinPlayerState*> TopPlayers = GameState->TopScoringPlayers;
				FString InfoTextString;
				if (TopPlayers.Num() == 0)
				{
					InfoTextString = FString("There is no winner");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == MannequinPlayerState)
				{
					InfoTextString = FString("You are the winner");
				}
				else if (TopPlayers.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *TopPlayers[0]->GetPlayerName());
				}
				else if (TopPlayers.Num() > 1)
				{
					InfoTextString = FString("Players tied for the win\n");
					for (auto TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}
				MannequinHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	AMannequinCharacter* MannequinCharacter = Cast<AMannequinCharacter>(GetPawn());
	if (MannequinCharacter && MannequinCharacter->GetCombat())
	{
		MannequinCharacter->bDisableGameplay = true;
		MannequinCharacter->GetCombat()->FireButtonPressed(false);
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

void AMannequinPlayerController::ServerCheckMatchState_Implementation()
{
	AMatchGameMode* GameMode = Cast<AMatchGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
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
