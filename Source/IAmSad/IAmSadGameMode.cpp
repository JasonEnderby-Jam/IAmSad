// Copyright Epic Games, Inc. All Rights Reserved.

#include "IAmSadGameMode.h"
#include "IAmSadCharacter.h"
#include "IAmSadHUD.h"
#include "UObject/ConstructorHelpers.h"

AIAmSadGameMode::AIAmSadGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default HUD class
	HUDClass = AIAmSadHUD::StaticClass();
}
