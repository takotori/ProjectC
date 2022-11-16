#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTC_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULagCompensationComponent();
	friend class AMannequinCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);
	FServerSideRewindResult ServerSideRewind(class AMannequinCharacter* HitCharacter,
	                      const FVector_NetQuantize& TraceStart,
	                      const FVector_NetQuantize& HitLocation, float HitTime);
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, AMannequinCharacter* HitCharacter,
	                                   const FVector_NetQuantize& TraceStart,
	                                   const FVector_NetQuantize& HitLocation);

protected:
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame,
	                                  float HitTime);
	void CacheBoxPositions(AMannequinCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(AMannequinCharacter* HitCharacter, const FFramePackage& Package);
	void ResetBoxes(AMannequinCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(AMannequinCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

private:
	UPROPERTY()
	AMannequinCharacter* Character;

	UPROPERTY()
	class APlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;
};