#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CardComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTC_API UCardComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCardComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void AddCard(class ACard* Card);
	void RemoveCard(ACard* Card);

	UPROPERTY(EditAnywhere, Category = "Cards")
	TArray<ACard*> Deck;

protected:
	virtual void BeginPlay() override;

private:	


};
