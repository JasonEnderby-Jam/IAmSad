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
class UPaperFlipbookComponent;
class UPaperFlipbook;
class USoundBase;
class UAudioComponent;
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

	/** Character sprite */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Sprite, meta = (AllowPrivateAccess = "true"))
	UPaperFlipbookComponent* CharacterSprite;

	/** Flipbook animations */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flipbooks", meta = (AllowPrivateAccess = "true"))
	UPaperFlipbook* IdleFlipbook;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flipbooks", meta = (AllowPrivateAccess = "true"))
	UPaperFlipbook* RunFlipbook;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flipbooks", meta = (AllowPrivateAccess = "true"))
	UPaperFlipbook* JumpFlipbook;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flipbooks", meta = (AllowPrivateAccess = "true"))
	UPaperFlipbook* GlideFlipbook;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flipbooks", meta = (AllowPrivateAccess = "true"))
	UPaperFlipbook* ReverseGravityFlipbook;

	void UpdateFlipbook();

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
	float DashSpeed = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	float DashCooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	float DashDuration = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	float DashImpulseStrength = 2000.0f;

	bool bCanDash = true;
	bool bIsDashing = false;
	FVector DashDirection;
	FVector DashVelocity;

	FTimerHandle DashCooldownTimer;
	FTimerHandle DashDurationTimer;

	void ResetDash();
	void EndDash();

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
	float RainbowHue = 0.0f;

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

	virtual void FellOutOfWorld(const UDamageType& DmgType) override;

	virtual void Jump() override;
	virtual void StopJumping() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump")
	float JumpCutMultiplier = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump")
	float JumpBufferTime = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump")
	float MinFallTimeForGlide = 0.2f;

	float JumpBufferTimer = 0.0f;
	float FallTimer = 0.0f;
	bool bHoldingJump = false;
	float SpriteForward = 1.0f;
	bool bGravityReversed = false;
	bool bPlayingGravityTransition = false;

	void ReverseGravity();

	/** Fall damage settings */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fall Damage")
	float FallDamageThresholdSpeed = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fall Damage")
	float FallDamageMultiplier = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fall Damage")
	bool bFallDamageEnabled = false;

	/** Z height below which the character dies (kill plane) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fall Damage")
	float KillPlaneZ = -50000.0f;

	float LastFallSpeed = 0.0f;
	bool bIsDead = false;

	void ApplyFallDamage(float FallSpeed);

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** Returns HealthComponent subobject **/
	FORCEINLINE class UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	// ============================================
	// Sounds - Assign in Blueprint/Editor
	// ============================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundBase* JumpSound;

	/** Looping sound that plays while gliding */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundBase* GlideLoopSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundBase* DeathSound;

	/** Sound played when taking any damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundBase* DamageSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundBase* FallDamageSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundBase* ReverseGravitySound;

protected:
	/** Audio component for glide loop sound */
	UPROPERTY()
	UAudioComponent* GlideAudioComponent;

	// ============================================
	// Blueprint Hooks - Override these in Blueprint
	// ============================================

	/** Called when the character jumps */
	UFUNCTION(BlueprintImplementableEvent, Category = "Character|Hooks")
	void OnJump();

	/** Called when the character starts gliding */
	UFUNCTION(BlueprintImplementableEvent, Category = "Character|Hooks")
	void OnStartGlide();

	/** Called when the character stops gliding */
	UFUNCTION(BlueprintImplementableEvent, Category = "Character|Hooks")
	void OnStopGlide();

	/** Called when the character dies from falling. FallSpeed is the impact velocity. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Character|Hooks")
	void OnDeathByFalling(float FallSpeed);

	/** Called when the character takes fall damage (but survives). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Character|Hooks")
	void OnFallDamage(float FallSpeed, float DamageAmount);

	/** Called when gravity is reversed. bReversed is true if gravity is now reversed (upside down). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Character|Hooks")
	void OnReverseGravity(bool bReversed);

	/** Called when the character takes any damage */
	UFUNCTION(BlueprintImplementableEvent, Category = "Character|Hooks")
	void OnTakeDamage(float DamageAmount, float RemainingHealth);

	/** Called when the character dies (from any cause) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Character|Hooks")
	void OnDeath();

protected:
	/** Called when health reaches zero */
	UFUNCTION()
	void HandleDeath();
};

