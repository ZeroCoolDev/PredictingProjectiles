// Copyright Epic Games, Inc. All Rights Reserved.

#include "PredictingGameMode.h"
#include "PredictingCharacter.h"
#include "UObject/ConstructorHelpers.h"

APredictingGameMode::APredictingGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
