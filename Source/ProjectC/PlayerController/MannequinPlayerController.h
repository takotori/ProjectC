#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MannequinPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

UCLASS()
class PROJECTC_API AMannequinPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDRedTeamScore(int32 RedScore);
	void SetHUDBlueTeamScore(int32 BlueScore);
	
	void SetHUDDefeats(int32 Defeats);
	void SetHUDAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDGrenades(int32 Grenades);
	void HideTeamScores();
	void InitTeamScores();
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual float GetServerTime(); // Synced with server world clock
	virtual void ReceivedPlayer() override; // Sync with server clock as soon as possible
	void OnMatchStateSet(FName State, bool bTeamsMatch = false);

	float SingleTripTime = 0.f;

	FHighPingDelegate HighPingDelegate;
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	void CheckTimeSync(float DeltaSeconds);
	void SetHUDTime();
	void PollInit();
	
	// Sync time between client and server
	// Request the current server time, passing in the clients time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	// Difference between client and server time
	float ClientServerDelta = 0.f;

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;

	void HandleMatchHasStarted(bool bTeamsMatch = false);
	void HandleCooldown();

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfMatch, float Warmup, float Match, float StartingTime);

	void HighPingWarning();
	void StopHighPingWarning();
	void CheckPing(float DeltaSeconds);

	void ShowPauseMenu();

	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

	UFUNCTION()
	void OnRep_ShowTeamScores();
	
private:
	UPROPERTY()
	class AMannequinHUD* MannequinHUD;

	// Return to main menu
	UPROPERTY(EditAnywhere, Category = HUD)
	TSubclassOf<class UUserWidget> PauseMenuWidget;

	UPROPERTY()
	class UPauseMenu* PauseMenu;

	bool bPauseMenuOpen = false;

	float LevelStartingTime = 0.f; 
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	bool bInitializeHealth = false;
	bool bInitializeScore = false;
	bool bInitializeDefeats = false;
	bool bInitializeGrenades = false;
	bool bInitializeShield = false;
	bool bInitializeAmmo = false;
	bool bInitializeTeamScores = false;
	
	float HUDHealth;
	float HUDMaxHealth;
	float HUDShield;
	float HUDMaxShield;
	float HUDScore;
	int32 HUDDefeats;
	int32 HUDGrenades;
	int32 HUDWeaponAmmo;

	float HighPingRunningTime = 0.f;

	float PingAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 10.f;

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

	UPROPERTY(EditAnywhere)
	float HighPingThreshhold = 50.f;
};
