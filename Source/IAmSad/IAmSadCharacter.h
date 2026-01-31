// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "IAmSadCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UHealthComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AIAmSadCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Health Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Health, meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComponent;

public:
	AIAmSadCharacter();
	

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Debug function to test damage */
	void DebugTakeDamage();

	/** Dash ability */
	void Dash();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	float DashDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	float DashCooldown = 1.0f;

	bool bCanDash = true;

	FTimerHandle DashCooldownTimer;

	void ResetDash();

	/** Glide ability - Elytra style */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float GlideGravity = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float GlideDrag = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float GlideMaxSpeed = 6000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float GlidePitchSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float GlidePitchSpeedMin = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float GlidePitchSpeedMax = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float GlidePitchSpeedReference = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float GlideStallSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float GlideSinkRate = 100.0f;

	// Camera zoom based on speed
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float GlideCameraMinDistance = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float GlideCameraMaxDistance = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float GlideCameraSpeedMin = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float GlideCameraSpeedMax = 3000.0f;

	bool bIsGliding = false;
	bool bIsStalling = false;
	float StallReflectionTarget = 0.0f;
	float StallRecoveryTimer = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float StallRecoveryDelay = 0.5f;
	FVector LastGlidePosition;
	float GlideDirection = 1.0f;
	float GlidePitchInput = 0.0f;
	float GlideSpeed = 0.0f;
	float GlidePitch = 0.0f;
	float OriginalCameraDistance = 0.0f;

	void GlidePitchUp();
	void GlidePitchDown();
	void GlidePitchStop();

	void StartGlide();
	void StopGlide();
	void UpdateGlide(float DeltaTime);

protected:

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void Landed(const FHitResult& Hit) override;

	virtual void Jump() override;
	virtual void StopJumping() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump")
	float JumpCutMultiplier = 0.05f;

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** Returns HealthComponent subobject **/
	FORCEINLINE class UHealthComponent* GetHealthComponent() const { return HealthComponent; }
};

