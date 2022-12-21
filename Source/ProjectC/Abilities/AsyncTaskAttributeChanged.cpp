#include "AsyncTaskAttributeChanged.h"

#include "AbilitySystemComponent.h"

UAsyncTaskAttributeChanged* UAsyncTaskAttributeChanged::ListenForAttributeChange(
	UAbilitySystemComponent* AbilitySystemComponent, FGameplayAttribute Attribute)
{
	UAsyncTaskAttributeChanged* WaitForAttributeChangedTask = NewObject<UAsyncTaskAttributeChanged>();
	WaitForAttributeChangedTask->Asc = AbilitySystemComponent;
	WaitForAttributeChangedTask->AttributeToListenFor = Attribute;

	if (!IsValid(AbilitySystemComponent) || !Attribute.IsValid())
	{
		WaitForAttributeChangedTask->RemoveFromRoot();
		return nullptr;
	}

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(
		WaitForAttributeChangedTask, &UAsyncTaskAttributeChanged::AttributeChanged);

	return WaitForAttributeChangedTask;
}

UAsyncTaskAttributeChanged* UAsyncTaskAttributeChanged::ListenForAttributesChange(
	UAbilitySystemComponent* AbilitySystemComponent, TArray<FGameplayAttribute> Attributes)
{
	UAsyncTaskAttributeChanged* WaitForAttributeChangedTask = NewObject<UAsyncTaskAttributeChanged>();
	WaitForAttributeChangedTask->Asc = AbilitySystemComponent;
	WaitForAttributeChangedTask->AttributesToListenFor = Attributes;

	if (!IsValid(AbilitySystemComponent) || Attributes.Num() < 1)
	{
		WaitForAttributeChangedTask->RemoveFromRoot();
		return nullptr;
	}

	for (const FGameplayAttribute Attribute : Attributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(
			WaitForAttributeChangedTask, &UAsyncTaskAttributeChanged::AttributeChanged);
	}

	return WaitForAttributeChangedTask;
}

void UAsyncTaskAttributeChanged::EndTask()
{
	if (IsValid(Asc))
	{
		Asc->GetGameplayAttributeValueChangeDelegate(AttributeToListenFor).RemoveAll(this);

		for (const FGameplayAttribute Attribute : AttributesToListenFor)
		{
			Asc->GetGameplayAttributeValueChangeDelegate(Attribute).RemoveAll(this);
		}
	}

	SetReadyToDestroy();
	MarkAsGarbage();
}

void UAsyncTaskAttributeChanged::AttributeChanged(const FOnAttributeChangeData& Data)
{
	OnAttributeChanged.Broadcast(Data.Attribute, Data.NewValue, Data.OldValue);
}
