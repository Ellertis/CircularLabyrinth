// Copyright Epic Games, Inc. All Rights Reserved.

#include "LabyrinthCirculaireGameMode.h"
#include "LabyrinthCirculaireCharacter.h"
#include "UObject/ConstructorHelpers.h"

ALabyrinthCirculaireGameMode::ALabyrinthCirculaireGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
