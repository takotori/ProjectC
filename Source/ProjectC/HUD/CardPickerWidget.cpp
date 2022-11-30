
#include "CardPickerWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/GameModeBase.h"
#include "ProjectC/GameMode/MatchGameMode.h"

void UCardPickerWidget::MenuSetup()
{
	bIsFocusable = true;
	SetVisibility(ESlateVisibility::Visible);
	
	if (const UWorld* World = GetWorld())
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	if (const UWorld* World = GetWorld())
	{
		if (AMatchGameMode* MatchGameMode = World->GetAuthGameMode<AMatchGameMode>())
		{
			TArray<ABaseCard*> DrawnCards = MatchGameMode->DrawCards();
			
		}
	}
}
