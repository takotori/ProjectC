#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectC/Character/MannequinCharacter.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	AMannequinCharacter* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	;
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<AMannequinCharacter*, uint32> Headshots;

	UPROPERTY()
	TMap<AMannequinCharacter*, uint32> Bodyshots;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTC_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULagCompensationComponent();
	friend class AMannequinCharacter;
	virtual void TickComponent(float DeltaTime,
	                           ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

	// Hitscan
	FServerSideRewindResult ServerSideRewind(class AMannequinCharacter* HitCharacter,
	                                         const FVector_NetQuantize& TraceStart,
	                                         const FVector_NetQuantize& HitLocation,
	                                         float HitTime);
	// Shotgun
	FShotgunServerSideRewindResult ShotgunServerSideRewind(
		const TArray<AMannequinCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime
	);

	// Projectile
	FServerSideRewindResult ProjectileServerSideRewind(
		AMannequinCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(
		AMannequinCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime,
		class AWeapon* DamageCauser
	);

	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(
		AMannequinCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(
		const TArray<AMannequinCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime
	);

protected:
	virtual void BeginPlay() override;
	void SaveFramePackage();
	void SaveFramePackage(FFramePackage& Package);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame,
	                                  float HitTime);
	void CacheBoxPositions(AMannequinCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(AMannequinCharacter* HitCharacter, const FFramePackage& Package);
	void ResetBoxes(AMannequinCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(AMannequinCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	FFramePackage GetFrameToCheck(AMannequinCharacter* HitCharacter, float HitTime);

	//todo refactor away hitcharacter as it is in the package
	// Hitscan
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package,
	                                   AMannequinCharacter* HitCharacter,
	                                   const FVector_NetQuantize& TraceStart,
	                                   const FVector_NetQuantize& HitLocation);

	// Shotgun
	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations
	);

	// Projectile
	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package,
		AMannequinCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

private:
	UPROPERTY()
	AMannequinCharacter* Character;

	UPROPERTY()
	class APlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;
};
