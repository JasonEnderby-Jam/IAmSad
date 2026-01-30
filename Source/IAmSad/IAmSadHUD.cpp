// Copyright Epic Games, Inc. All Rights Reserved.

#include "IAmSadHUD.h"
#include "HealthBarWidget.h"
#include "HealthComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"

AIAmSadHUD::AIAmSadHUD()
{
	HealthBarWidget = nullptr;

	// Find the health bar widget blueprint
	static ConstructorHelpers::FClassFinder<UHealthBarWidget> HealthBarWidgetBPClass(TEXT("/Game/ThirdPerson/UI/WBP_HealthBar"));
	if (HealthBarWidgetBPClass.Succeeded())
	{
		HealthBarWidgetClass = HealthBarWidgetBPClass.Class;
	}
}

void AIAmSadHUD::BeginPlay()
{
	Super::BeginPlay();

	if (HealthBarWidgetClass)
	{
		APlayerController* PC = GetOwningPlayerController();
		if (PC)
		{
			HealthBarWidget = CreateWidget<UHealthBarWidget>(PC, HealthBarWidgetClass);
			if (HealthBarWidget)
			{
				HealthBarWidget->AddToViewport();

				// Find the health component on the player pawn
				APawn* PlayerPawn = PC->GetPawn();
				if (PlayerPawn)
				{
					UHealthComponent* HealthComp = PlayerPawn->FindComponentByClass<UHealthComponent>();
					if (HealthComp)
					{
						HealthBarWidget->SetHealthComponent(HealthComp);
					}
				}
			}
		}
	}
}
