#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_GrenadeLauncher UMETA(DisplayName = "Grenade Launcher"),
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};
