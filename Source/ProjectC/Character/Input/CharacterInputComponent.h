#pragma once

#include "CoreMinimal.h"
#include "CharacterInputConfig.h"
#include "EnhancedInputComponent.h"
#include "UObject/Object.h"
#include "CharacterInputComponent.generated.h"

UCLASS()
class PROJECTC_API UCharacterInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	UCharacterInputComponent(const FObjectInitializer& ObjectInitializer);

	template <class UserClass, typename FuncType>
	void BindNativeAction(const UCharacterInputConfig* InputConfig, const FGameplayTag& InputTag,
	                      ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func);

	template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActions(const UCharacterInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc,
	                        ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles);
};

template <class UserClass, typename FuncType>
void UCharacterInputComponent::BindNativeAction(const UCharacterInputConfig* InputConfig, const FGameplayTag& InputTag,
                                                ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func)
{
	check(InputConfig);
	if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag))
	{
		BindAction(IA, TriggerEvent, Object, Func);
	}
}

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UCharacterInputComponent::BindAbilityActions(const UCharacterInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
{
	check(InputConfig);

	for (const FCharacterInputAction& Action : InputConfig->AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			if (PressedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, PressedFunc,
				                           Action.InputTag).GetHandle());
			}

			if (ReleasedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc,
				                           Action.InputTag).GetHandle());
			}
		}
	}
}
