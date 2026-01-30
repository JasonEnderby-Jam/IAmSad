// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "IAmSadHUD.generated.h"

class UHealthBarWidget;

UCLASS()
class IAMSAD_API AIAmSadHUD : public AHUD
{
	GENERATED_BODY()

public:
	AIAmSadHUD();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UHealthBarWidget> HealthBarWidgetClass;

	UPROPERTY()
	UHealthBarWidget* HealthBarWidget;
};
