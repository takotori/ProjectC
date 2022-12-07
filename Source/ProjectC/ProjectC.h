// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define ECC_SkeletalMesh ECollisionChannel::ECC_GameTraceChannel1
#define ECC_HitBox ECollisionChannel::ECC_GameTraceChannel2

UENUM(BlueprintType)
enum class EGSAbilityInputID : uint8
{
	// 0 None
	None				UMETA(DisplayName = "None"),
	// 1 Confirm
	Confirm				UMETA(DisplayName = "Confirm"),
	// 2 Cancel
	Cancel				UMETA(DisplayName = "Cancel"),
	// 3 Sprint
	Sprint				UMETA(DisplayName = "Sprint"),
	// 4 Jump
	Jump				UMETA(DisplayName = "Jump"),
	// 5 Crouch
	Crouch				UMETA(DisplayName = "Crouch"),
	// 6 PrimaryFire
	Fire				UMETA(DisplayName = "Fire"),
	// 7 Reload
	Reload				UMETA(DisplayName = "Reload"),
	// 7 Reload
	ThrowGrenade		UMETA(DisplayName = "ThrowGrenade")
};
