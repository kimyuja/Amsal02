// Copyright Epic Games, Inc. All Rights Reserved.

#include "SecondProjectGameMode.h"
#include "SecondProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASecondProjectGameMode::ASecondProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
