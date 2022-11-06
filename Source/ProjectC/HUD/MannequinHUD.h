// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MannequinHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	class UTexture2D* CrosshairsCenter;
	
	UPROPERTY()
	UTexture2D* CrosshairsLeft;
	
	UPROPERTY()
	UTexture2D* CrosshairsRight;
	
	UPROPERTY()
	UTexture2D* CrosshairsTop;
	
	UPROPERTY()
	UTexture2D* CrosshairsBottom;
	
	float CrosshairSpread;
	FLinearColor CrosshairsColor;
};


/**
 * 
 */
UCLASS()
class PROJECTC_API AMannequinHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
	void AddCharacterOverlay();
	void AddAnnouncement();

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;

	UPROPERTY(EditAnywhere, Category = "Announcement")
	TSubclassOf<class UUserWidget> AnnouncementClass;

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	UPROPERTY()
	class UAnnouncement* Announcement;

protected:
	virtual void BeginPlay() override;
	
private:
	FHUDPackage HUDPackage;
	void DrawCrosshair(UTexture2D* Texture, FVector2d ViewportCenter, FVector2d Spread, FLinearColor CrosshairColor);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;
	
public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
