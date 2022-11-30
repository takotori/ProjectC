
#include "Card.h"

#include "ProjectC/Types/CardInfo.h"

ACard::ACard()
{
}

void ACard::DrawCards()
{
	if (AllCards)
	{
		FCardInfo* CardInfo = AllCards->FindRow<FCardInfo>("Poison", "");
		
	}
}

