#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Card.generated.h"

UCLASS()
class PROJECTC_API ACard : public AActor
{
	GENERATED_BODY()
	
public:
	ACard();
	void DrawCards();

protected:
	UPROPERTY(EditAnywhere)
	UDataTable* AllCards;
	
private:
	
};
