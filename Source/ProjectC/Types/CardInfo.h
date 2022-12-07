#pragma once

#include "CardInfo.generated.h"

USTRUCT(BlueprintType)
struct FCardInfo : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Name"))
	FString Name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="CardDesign", MakeStructureDefaultValue="None"))
	TObjectPtr<UTexture2D> CardDesign;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Description"))
	FText Description;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Ability"))
	FString Ability;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="isEnabled", MakeStructureDefaultValue="True"))
	bool isEnabled;
};
