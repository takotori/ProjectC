
#include "MannequinPlayerState.h"

#include "AbilitySystemComponent.h"
#include "ProjectC/Character/MannequinCharacter.h"
#include "ProjectC/PlayerController/MannequinPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "ProjectC/Abilities/AttributeSets/CharacterAttributes.h"
#include "ProjectC/Components/CardAbilitySystemComponent.h"

AMannequinPlayerState::AMannequinPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UCardAbilitySystemComponent>(TEXT("Ability Component"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UCharacterAttributes>(TEXT("Attributes"));

	NetUpdateFrequency = 66.f;
}

void AMannequinPlayerState::BeginPlay()
{
	Super::BeginPlay();
	if (AbilitySystemComponent) AttributeSet = AbilitySystemComponent->GetSet<UCharacterAttributes>();
}

void AMannequinPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMannequinPlayerState, Defeats);
	DOREPLIFETIME(AMannequinPlayerState, Team);
}

void AMannequinPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<AMannequinCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AMannequinPlayerController>(Character->Controller) : Controller;
		if (Controller) Controller->SetHUDScore(GetScore());
	}
}

void AMannequinPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	Character = Character == nullptr ? Cast<AMannequinCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AMannequinPlayerController>(Character->Controller) : Controller;
		if (Controller) Controller->SetHUDScore(GetScore());
	}
}

void AMannequinPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<AMannequinCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AMannequinPlayerController>(Character->Controller) : Controller;
		if (Controller) Controller->SetHUDDefeats(Defeats);
	}
}

void AMannequinPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<AMannequinCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AMannequinPlayerController>(Character->Controller) : Controller;
		if (Controller) Controller->SetHUDDefeats(Defeats);
	}
}

UAbilitySystemComponent* AMannequinPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}