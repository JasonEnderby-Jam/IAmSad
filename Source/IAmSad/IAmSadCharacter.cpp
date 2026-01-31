// Copyright Epic Games, Inc. All Rights Reserved.

#include "IAmSadCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "HealthComponent.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AIAmSadCharacter

AIAmSadCharacter::AIAmSadCharacter()
{
	// Enable ticking
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;

	// Enable double jump
	JumpMaxCount = 2;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Constrain to 2.5D plane (lock X axis, move only on Y and Z)
	GetCharacterMovement()->SetPlaneConstraintEnabled(true);
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(1, 0, 0));

	// Create a camera boom (no collision test - character uses outline when occluded instead)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = false;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character)
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// Create Health Component
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

//////////////////////////////////////////////////////////////////////////
// BeginPlay

void AIAmSadCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Enable custom depth rendering for occlusion outline effect
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(1);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AIAmSadCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AIAmSadCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AIAmSadCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AIAmSadCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

	// Debug damage input (H key)
	PlayerInputComponent->BindKey(EKeys::H, IE_Pressed, this, &AIAmSadCharacter::DebugTakeDamage);

	// Dash input (Shift key)
	PlayerInputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &AIAmSadCharacter::Dash);
}

void AIAmSadCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// While gliding, A/D controls pitch (no direction change)
	if (bIsGliding)
	{
		// A (negative X) = pitch up, D (positive X) = pitch down
		GlidePitchInput = -MovementVector.X;
		return;
	}

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement
		//AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AIAmSadCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	/*
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
	*/
}

void AIAmSadCharacter::DebugTakeDamage()
{
	if (HealthComponent)
	{
		HealthComponent->TakeDamage(10.0f);
		UE_LOG(LogTemplateCharacter, Log, TEXT("Debug damage: Health is now %.1f"), HealthComponent->CurrentHealth);
	}
}

void AIAmSadCharacter::Dash()
{
	if (!bCanDash)
	{
		return;
	}

	// Get dash direction based on character facing direction
	FVector DashDirection = GetActorForwardVector();

	// Launch character in dash direction
	LaunchCharacter(DashDirection * DashDistance, true, true);

	// Start cooldown
	bCanDash = false;
	GetWorldTimerManager().SetTimer(DashCooldownTimer, this, &AIAmSadCharacter::ResetDash, DashCooldown, false);
}

void AIAmSadCharacter::ResetDash()
{
	bCanDash = true;
}

void AIAmSadCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsGliding)
	{
		FHitResult HitResult;
		FVector Start = GetActorLocation();
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		// Check if we hit the ground
		FVector GroundEnd = Start - FVector(0.0f, 0.0f, 50.0f);
		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, GroundEnd, ECC_Visibility, QueryParams))
		{
			StopGlide();
			return;
		}

		// Check if we hit a wall (trace in movement direction)
		FVector WallEnd = Start + FVector(0.0f, GlideDirection * 50.0f, 0.0f);
		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, WallEnd, ECC_Visibility, QueryParams))
		{
			StopGlide();
			// Kill upward momentum when hitting wall
			FVector Velocity = GetCharacterMovement()->Velocity;
			if (Velocity.Z > 0)
			{
				Velocity.Z = 0;
				GetCharacterMovement()->Velocity = Velocity;
			}
			return;
		}

		UpdateGlide(DeltaTime);
	}
}

void AIAmSadCharacter::UpdateGlide(float DeltaTime)
{
	// Check for stall - if speed too low, force nose down
	// Stay in stall until we're actually diving (pitch < -20)
	bool bShouldStall = GlideSpeed < GlideStallSpeed;
	if (bShouldStall && !bIsStalling)
	{
		// Entering stall - remember the pitch to reflect from
		bIsStalling = true;
		StallReflectionTarget = FMath::Min(-FMath::Abs(GlidePitch), -30.0f);
	}
	else if (bIsStalling && GlidePitch <= StallReflectionTarget + 5.0f)
	{
		// Reached reflection target, exit stall
		bIsStalling = false;
	}

	if (bIsStalling)
	{
		// Stalling - full reflection (90° up becomes 90° down)
		GlidePitch = FMath::FInterpTo(GlidePitch, StallReflectionTarget, DeltaTime, 8.0f);
	}
	else
	{
		// Normal pitch control (A/D keys)
		// Slower speed = tighter turns (faster pitch change)
		float SpeedFactor = FMath::Clamp(GlidePitchSpeedReference / FMath::Max(GlideSpeed, 100.0f), GlidePitchSpeedMin, GlidePitchSpeedMax);
		GlidePitch += GlidePitchInput * GlidePitchSpeed * SpeedFactor * DeltaTime;
	}

	// Wrap pitch to -180 to 180 range for loops
	while (GlidePitch > 180.0f) GlidePitch -= 360.0f;
	while (GlidePitch < -180.0f) GlidePitch += 360.0f;

	// Convert pitch to radians for calculations
	float PitchRad = FMath::DegreesToRadians(GlidePitch);

	// Gravity effect on speed:
	// - Diving (negative pitch) = gravity adds to speed
	// - Climbing (positive pitch) = gravity reduces speed
	float GravityEffect = -FMath::Sin(PitchRad) * GlideGravity * DeltaTime;
	GlideSpeed += GravityEffect;

	// Always lose a bit of energy (can't glide forever horizontally)
	GlideSpeed -= GlideGravity * 0.1f * DeltaTime;

	// Apply drag at high speeds
	if (GlideSpeed > 500.0f)
	{
		GlideSpeed -= GlideSpeed * GlideDrag * DeltaTime;
	}

	// Clamp speed (allow going below stall, but not below minimum)
	GlideSpeed = FMath::Clamp(GlideSpeed, 50.0f, GlideMaxSpeed);

	// Calculate velocity from speed and pitch
	// Always sink a bit even when flying level
	FVector Velocity;
	Velocity.X = 0.0f;
	Velocity.Y = GlideDirection * FMath::Cos(PitchRad) * GlideSpeed;
	Velocity.Z = FMath::Sin(PitchRad) * GlideSpeed - GlideSinkRate;

	// Apply velocity directly
	GetCharacterMovement()->Velocity = Velocity;

	// Rotate character to show glide direction (rolled 90 degrees to look like flying/lying down)
	// Use quaternion slerp to avoid gimbal lock issues during loops
	float YawAngle = (GlideDirection > 0) ? 90.0f : -90.0f;
	float RollAngle = (GlideDirection > 0) ? -90.0f : 90.0f;
	FQuat TargetQuat = FRotator(GlidePitch, YawAngle, RollAngle).Quaternion();
	FQuat CurrentQuat = GetActorRotation().Quaternion();
	FQuat NewQuat = FQuat::Slerp(CurrentQuat, TargetQuat, FMath::Clamp(DeltaTime * 8.0f, 0.0f, 1.0f));
	SetActorRotation(NewQuat.Rotator());

	// Camera zoom based on speed (with deadbands)
	float SpeedAlpha = FMath::Clamp((GlideSpeed - GlideCameraSpeedMin) / (GlideCameraSpeedMax - GlideCameraSpeedMin), 0.0f, 1.0f);
	float TargetCameraDistance = FMath::Lerp(GlideCameraMinDistance, GlideCameraMaxDistance, SpeedAlpha);
	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetCameraDistance, DeltaTime, 3.0f);

	// Reset pitch input after using it (will be set again by Move if keys are held)
	GlidePitchInput = 0.0f;
}

void AIAmSadCharacter::Jump()
{
	// If we've used all jumps and we're falling, start gliding
	if (JumpCurrentCount >= JumpMaxCount && GetCharacterMovement()->IsFalling() && !bIsGliding)
	{
		StartGlide();
	}
	else
	{
		// Stop gliding if we somehow can jump again
		if (bIsGliding)
		{
			StopGlide();
		}
		Super::Jump();
	}
}

void AIAmSadCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	StopGlide();
}

void AIAmSadCharacter::StartGlide()
{
	bIsGliding = true;
	bIsStalling = false;

	// Store original camera distance
	OriginalCameraDistance = CameraBoom->TargetArmLength;

	// Set direction based on current facing (use actor's current yaw)
	FRotator CurrentRotation = GetActorRotation();
	float CurrentYaw = CurrentRotation.Yaw;

	// Determine glide direction from current facing
	// Character facing +Y has yaw ~90, facing -Y has yaw ~-90
	GlideDirection = (FMath::Abs(CurrentYaw) < 90.0f) ? 1.0f : -1.0f;
	if (CurrentYaw > 0) GlideDirection = 1.0f;
	else GlideDirection = -1.0f;

	// Initialize glide with current momentum, start horizontal
	FVector Velocity = GetCharacterMovement()->Velocity;
	GlideSpeed = FMath::Max(Velocity.Size(), 400.0f);
	GlidePitch = 0.0f;

	// Disable movement component's rotation control
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// Don't snap rotation - let UpdateGlide smoothly interpolate to glide pose

	// Disable default gravity (we handle it ourselves)
	GetCharacterMovement()->GravityScale = 0.0f;
}

void AIAmSadCharacter::StopGlide()
{
	if (bIsGliding)
	{
		bIsGliding = false;
		bIsStalling = false;

		// Restore normal gravity
		GetCharacterMovement()->GravityScale = 1.0f;

		// Re-enable movement component's rotation control
		GetCharacterMovement()->bOrientRotationToMovement = true;

		// Reset character rotation (keep yaw, reset pitch and roll)
		FRotator CurrentRotation = GetActorRotation();
		SetActorRotation(FRotator(0.0f, CurrentRotation.Yaw, 0.0f));

		// Restore camera distance
		CameraBoom->TargetArmLength = OriginalCameraDistance;

		// Reset pitch input
		GlidePitchInput = 0.0f;
	}
}

void AIAmSadCharacter::GlidePitchUp()
{
	GlidePitchInput = 1.0f;
}

void AIAmSadCharacter::GlidePitchDown()
{
	GlidePitchInput = -1.0f;
}

void AIAmSadCharacter::GlidePitchStop()
{
	GlidePitchInput = 0.0f;
}
