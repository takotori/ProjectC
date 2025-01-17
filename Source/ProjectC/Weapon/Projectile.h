#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectC/Abilities/FGameplayAbilityTargetData_STH.h"
#include "Projectile.generated.h"

UCLASS()
class PROJECTC_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

	// Used with server side rewind
	bool bUseServerSideRewind = false;

	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000.f;

	// Only set this for grenades and rockets
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	// Doesn't matter for grenade and rockets
	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn="true"))
	FGameplayEffectSpecHandle GameplayEffectSpec;

protected:
	virtual void BeginPlay() override;
	void StartDestroyTimer();
	void DestroyTimerFinished();
	void SpawnTrailSystem();
	void ExplodeDamage();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
					   FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;
	
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticle;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;
	
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* ProjectileMesh;
	
private:
	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	UPROPERTY()
	UParticleSystemComponent* TracerComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;
};
