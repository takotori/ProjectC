
#include "CardComponent.h"

UCardComponent::UCardComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
}

void UCardComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UCardComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCardComponent::AddCard(ACard* Card)
{
	Deck.Add(Card);
}

void UCardComponent::RemoveCard(ACard* Card)
{		
	Deck.Remove(Card);
}
