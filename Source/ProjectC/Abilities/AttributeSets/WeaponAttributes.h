#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "WeaponAttributes.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class PROJECTC_API UWeaponAttributes : public UAttributeSet
{
	GENERATED_BODY()

public:
	UWeaponAttributes();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Damage)
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UWeaponAttributes, Damage)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_RoundPerMinute)
	FGameplayAttributeData RoundsPerMinute;
	ATTRIBUTE_ACCESSORS(UWeaponAttributes, RoundsPerMinute)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Magazine)
	FGameplayAttributeData Magazine;
	ATTRIBUTE_ACCESSORS(UWeaponAttributes, Magazine)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_BulletSpeed)
	FGameplayAttributeData BulletSpeed;
	ATTRIBUTE_ACCESSORS(UWeaponAttributes, BulletSpeed)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Spread)
	FGameplayAttributeData Spread;
	ATTRIBUTE_ACCESSORS(UWeaponAttributes, Spread)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_CanBounce)
	FGameplayAttributeData bCanBounce;
	ATTRIBUTE_ACCESSORS(UWeaponAttributes, bCanBounce)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_CanExplode)
	FGameplayAttributeData bCanExplode;
	ATTRIBUTE_ACCESSORS(UWeaponAttributes, bCanExplode)

	UFUNCTION()
	virtual void OnRep_Damage(const FGameplayAttributeData& OldDamage);

	UFUNCTION()
	virtual void OnRep_RoundPerMinute(const FGameplayAttributeData& OldRoundsPerMinute);

	UFUNCTION()
	virtual void OnRep_Magazine(const FGameplayAttributeData& OldMagazine);

	UFUNCTION()
	virtual void OnRep_BulletSpeed(const FGameplayAttributeData& OldBulletSpeed);

	UFUNCTION()
	virtual void OnRep_Spread(const FGameplayAttributeData& OldSpread);

	UFUNCTION()
	virtual void OnRep_CanBounce(const FGameplayAttributeData& OldCanBounce);

	UFUNCTION()
	virtual void OnRep_CanExplode(const FGameplayAttributeData& OldCanExplode);
};
