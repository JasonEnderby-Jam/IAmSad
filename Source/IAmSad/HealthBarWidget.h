// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.generated.h"

class UProgressBar;
class UHealthComponent;

UCLASS()
class IAMSAD_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetHealthComponent(UHealthComponent* InHealthComponent);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnHealthChanged(float CurrentHealth, float MaxHealth);

	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* HealthBar;

	UPROPERTY()
	UHealthComponent* HealthComponent;
};
