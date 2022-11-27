#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CardPickerWidget.generated.h"

UCLASS()
class PROJECTC_API UCardPickerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void MenuSetup();
	
private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Card1;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Card2;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Card3;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Card4;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Card5;

	UPROPERTY()
	class APlayerController* PlayerController;
};
