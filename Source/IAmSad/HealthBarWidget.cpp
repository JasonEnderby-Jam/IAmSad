// Copyright Epic Games, Inc. All Rights Reserved.

#include "HealthBarWidget.h"
#include "HealthComponent.h"
#include "Components/ProgressBar.h"

void UHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UHealthBarWidget::SetHealthComponent(UHealthComponent* InHealthComponent)
{
	HealthComponent = InHealthComponent;

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &UHealthBarWidget::OnHealthChanged);

		// Initialize with current values
		OnHealthChanged(HealthComponent->CurrentHealth, HealthComponent->MaxHealth);
	}
}

void UHealthBarWidget::OnHealthChanged(float CurrentHealth, float MaxHealth)
{
	if (HealthBar && MaxHealth > 0.0f)
	{
		HealthBar->SetPercent(CurrentHealth / MaxHealth);
	}
}
