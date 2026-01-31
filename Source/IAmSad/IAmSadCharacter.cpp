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
#include "DrawDebugHelpers.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"

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
	GetCharacterMovement()->JumpZVelocity = 650.f;
	GetCharacterMovement()->AirControl = 1.0f;
	GetCharacterMovement()->AirControlBoostMultiplier = 4.0f;
	GetCharacterMovement()->AirControlBoostVelocityThreshold = 0.0f;
	GetCharacterMovement()->FallingLateralFriction = 8.0f;

	// Single jump
	JumpMaxCount = 1;
	GetCharacterMovement()->MaxWalkSpeed = 700.f;
	GetCharacterMovement()->MaxFlySpeed = 700.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 4000.0f;
	GetCharacterMovement()->MaxAcceleration = 2500.f;
	GetCharacterMovement()->GravityScale = 1.3f;

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

	// Hide the skeletal mesh - we're using a sprite instead
	GetMesh()->SetVisibility(false);

	// Create character sprite (flipbook for animations)
	CharacterSprite = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("CharacterSprite"));
	CharacterSprite->SetupAttachment(RootComponent);
	CharacterSprite->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

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
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AIAmSadCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AIAmSadCharacter::StopJumping);

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

	// Reverse gravity (R key)
	PlayerInputComponent->BindKey(EKeys::R, IE_Pressed, this, &AIAmSadCharacter::ReverseGravity);
}

void AIAmSadCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// While gliding, W/S controls pitch
	if (bIsGliding)
	{
		// W = pitch down (dive), S = pitch up (climb)
		GlidePitchInput = -MovementVector.Y;
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
	DashDirection = GetActorForwardVector();

	// Set dash velocity
	DashVelocity = DashDirection * DashSpeed;

	// Start dashing
	bIsDashing = true;
	GetCharacterMovement()->Velocity = DashVelocity;

	// Start dash duration timer
	GetWorldTimerManager().SetTimer(DashDurationTimer, this, &AIAmSadCharacter::EndDash, DashDuration, false);

	// Start cooldown
	bCanDash = false;
	GetWorldTimerManager().SetTimer(DashCooldownTimer, this, &AIAmSadCharacter::ResetDash, DashCooldown, false);
}

void AIAmSadCharacter::EndDash()
{
	bIsDashing = false;
}

void AIAmSadCharacter::ResetDash()
{
	bCanDash = true;
}

void AIAmSadCharacter::ReverseGravity()
{
	// Don't allow gravity reversal while mid-air - that's cheesy as fuck
	// Check vertical velocity instead of IsFalling() because floor detection
	// doesn't work properly when standing on ceilings with reversed gravity
	float ZSpeed = FMath::Abs(GetCharacterMovement()->Velocity.Z);
	if (ZSpeed > 50.0f)
	{
		return;
	}

	bGravityReversed = !bGravityReversed;
	float NewGravityScale = bGravityReversed ? -1.3f : 1.3f;
	GetCharacterMovement()->GravityScale = NewGravityScale;

	// Flip jump velocity direction so jumping works with reversed gravity
	GetCharacterMovement()->JumpZVelocity = bGravityReversed ? -650.f : 650.f;

	// Force into falling mode so the new gravity takes effect immediately
	// (otherwise the character "sticks" to the ground like a spider)
	GetCharacterMovement()->SetMovementMode(MOVE_Falling);
}

void AIAmSadCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update flipbook based on current state
	UpdateFlipbook();

	// Keep sprite facing the camera and flip based on direction
	if (CharacterSprite)
	{
		if (bIsGliding)
		{
			// During gliding, rotate sprite around world X axis (camera view) to stay 2D
			float YawAngle = (GlideDirection > 0) ? 90.0f : -90.0f;
			float SpriteGlidePitch = (GlideDirection > 0) ? GlidePitch : -GlidePitch;
			FQuat YawQuat = FQuat(FVector::UpVector, FMath::DegreesToRadians(YawAngle));
			FQuat PitchQuat = FQuat(FVector::ForwardVector, FMath::DegreesToRadians(SpriteGlidePitch));
			CharacterSprite->SetWorldRotation((PitchQuat * YawQuat).Rotator());
		}
		else
		{
			// Flip sprite based on velocity direction using rotation
			float VelY = GetCharacterMovement()->Velocity.Y;
			if (VelY > 10.0f)
			{
				// Moving right - face right
				SpriteForward = 1.0f;
			}
			else if (VelY < -10.0f)
			{
				// Moving left - face left
				SpriteForward = -1.0f;
			}
			// Apply rotation (default right if never moved)
			// Flip upside down if gravity is reversed
			float YawAngle = (SpriteForward > 0) ? 90.0f : -90.0f;
			float RollAngle = bGravityReversed ? 180.0f : 0.0f;
			CharacterSprite->SetWorldRotation(FRotator(0.0f, YawAngle, RollAngle));
		}
	}

	// Track fall time for glide entry (only count actual falling, not rising)
	// Gliding disabled when gravity is reversed
	float ZVel = GetCharacterMovement()->Velocity.Z;
	bool bActuallyFalling = !bGravityReversed && GetCharacterMovement()->IsFalling() && ZVel < 0.0f;

	if (bActuallyFalling)
	{
		FallTimer += DeltaTime;

		// Auto-start glide after falling long enough
		if (!bIsGliding && FallTimer >= MinFallTimeForGlide)
		{
			StartGlide();
		}
	}
	else
	{
		FallTimer = 0.0f;
	}

	// Tick down jump buffer
	if (JumpBufferTimer > 0.0f)
	{
		JumpBufferTimer -= DeltaTime;
	}

	// During dash, maintain constant velocity and knock away physics objects
	if (bIsDashing)
	{
		// Force velocity to stay constant - we don't slow down for anything
		GetCharacterMovement()->Velocity = DashVelocity;

		// Check for actors we're hitting and apply impulse
		TArray<FHitResult> Hits;
		FVector Start = GetActorLocation();
		FVector End = Start + DashDirection * 100.0f;
		FCollisionShape Shape = FCollisionShape::MakeCapsule(GetCapsuleComponent()->GetScaledCapsuleRadius(), GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		if (GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_PhysicsBody, Shape, QueryParams))
		{
			for (const FHitResult& Hit : Hits)
			{
				if (Hit.GetActor() && Hit.GetComponent() && Hit.GetComponent()->IsSimulatingPhysics())
				{
					FVector Impulse = DashDirection * DashImpulseStrength;
					Hit.GetComponent()->AddImpulseAtLocation(Impulse, Hit.ImpactPoint, NAME_None);
				}
			}
		}
	}

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
	// Need speed above stall threshold AND be diving AND wait for recovery timer
	if (bIsStalling)
	{
		StallRecoveryTimer -= DeltaTime;

		// Exit stall only when: recovery timer done AND have speed AND are diving
		if (StallRecoveryTimer <= 0.0f && GlideSpeed >= GlideStallSpeed && GlidePitch < -30.0f)
		{
			bIsStalling = false;
		}
	}
	else if (GlideSpeed < GlideStallSpeed)
	{
		bIsStalling = true;
		StallRecoveryTimer = StallRecoveryDelay;
	}

	if (bIsStalling)
	{
		// Stalling - no player control, nose drops FAST toward -90 (straight down)
		float StallRate = 400.0f;

		// Extra aggressive when horizontal (near pitch 0) - snap down hard
		if (FMath::Abs(GlidePitch) < 30.0f)
		{
			StallRate = 1000.0f;
		}
		else if (GlidePitch > -30.0f)
		{
			StallRate = 600.0f;
		}

		// Always push toward diving (negative pitch)
		GlidePitch = FMath::FInterpTo(GlidePitch, -90.0f, DeltaTime, StallRate / 30.0f);

		// Fall faster during stall - reduce speed further
		GlideSpeed = FMath::Max(GlideSpeed - 100.0f * DeltaTime, 50.0f);
	}
	else
	{
		// Normal pitch control (A/D keys)
		// Slower speed = tighter turns (faster pitch change)
		float SpeedFactor = FMath::Clamp(GlidePitchSpeedReference / FMath::Max(GlideSpeed, 100.0f), GlidePitchSpeedMin, GlidePitchSpeedMax);
		GlidePitch += GlidePitchInput * GlidePitchSpeed * SpeedFactor * DeltaTime;

		// Gradual lift loss - nose drops as speed decreases (losing lift)
		// Stronger effect at lower speeds, no effect above 3500 speed
		float LiftLossThreshold = 3500.0f;
		if (GlideSpeed < LiftLossThreshold)
		{
			float LiftLossFactor = 1.0f - (GlideSpeed / LiftLossThreshold);
			float NoseDropRate = LiftLossFactor * LiftLossFactor * 150.0f * DeltaTime;
			// Always push toward negative pitch (diving down)
			GlidePitch -= NoseDropRate;
		}
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
	// Preserve some horizontal momentum when diving (but not when inverted/looping)
	bool bIsInverted = FMath::Abs(GlidePitch) > 90.0f;
	float HorizontalFactor = FMath::Cos(PitchRad);
	if (!bIsInverted && HorizontalFactor >= 0.0f)
	{
		HorizontalFactor = FMath::Max(HorizontalFactor, 0.3f);
	}
	FVector Velocity;
	Velocity.X = 0.0f;
	Velocity.Y = GlideDirection * HorizontalFactor * GlideSpeed;
	Velocity.Z = FMath::Sin(PitchRad) * GlideSpeed - GlideSinkRate;

	// Apply velocity directly
	GetCharacterMovement()->Velocity = Velocity;

	// Rotate character to show glide angle while keeping sprite 2D (facing camera)
	// Yaw keeps sprite facing camera, Roll shows the glide pitch angle
	float YawAngle = (GlideDirection > 0) ? 90.0f : -90.0f;
	float RollAngle = (GlideDirection > 0) ? -GlidePitch : GlidePitch;
	FQuat TargetQuat = FRotator(0.0f, YawAngle, RollAngle).Quaternion();
	FQuat CurrentQuat = GetActorRotation().Quaternion();
	FQuat NewQuat = FQuat::Slerp(CurrentQuat, TargetQuat, FMath::Clamp(DeltaTime * 8.0f, 0.0f, 1.0f));
	SetActorRotation(NewQuat.Rotator());

	// Camera zoom based on speed (with deadbands)
	float SpeedAlpha = FMath::Clamp((GlideSpeed - GlideCameraSpeedMin) / (GlideCameraSpeedMax - GlideCameraSpeedMin), 0.0f, 1.0f);
	float TargetCameraDistance = FMath::Lerp(GlideCameraMinDistance, GlideCameraMaxDistance, SpeedAlpha);
	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetCameraDistance, DeltaTime, 3.0f);

	// Draw rainbow trail line behind character
	FVector CurrentPosition = GetActorLocation();
	RainbowHue = FMath::Fmod(RainbowHue + DeltaTime * 200.0f, 360.0f);
	FLinearColor RainbowColor = FLinearColor::MakeFromHSV8(static_cast<uint8>(RainbowHue / 360.0f * 255.0f), 255, 255);
	DrawDebugLine(GetWorld(), LastGlidePosition, CurrentPosition, RainbowColor.ToFColor(true), false, 2.0f, 0, 2.0f);
	LastGlidePosition = CurrentPosition;

	// Reset pitch input after using it (will be set again by Move if keys are held)
	GlidePitchInput = 0.0f;
}

void AIAmSadCharacter::Jump()
{
	bHoldingJump = true;

	// Handle reversed gravity jump manually - floor detection doesn't work on ceilings
	if (bGravityReversed)
	{
		// Check if we're "grounded" on the ceiling (low vertical velocity)
		float ZSpeed = FMath::Abs(GetCharacterMovement()->Velocity.Z);
		if (ZSpeed < 50.0f)
		{
			// Apply jump velocity directly (already negative when gravity reversed)
			FVector Velocity = GetCharacterMovement()->Velocity;
			Velocity.Z = GetCharacterMovement()->JumpZVelocity;
			GetCharacterMovement()->Velocity = Velocity;
			GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		}
		return;
	}

	// Normal gravity jump logic
	// If falling and trying to jump, buffer it for when we land
	if (GetCharacterMovement()->IsFalling() && !bIsGliding)
	{
		// Buffer the jump for when we land
		JumpBufferTimer = JumpBufferTime;
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

void AIAmSadCharacter::StopJumping()
{
	Super::StopJumping();
	bHoldingJump = false;

	// Variable jump height - cut upward velocity when jump is released early
	if (GetCharacterMovement()->Velocity.Z > 0)
	{
		FVector Velocity = GetCharacterMovement()->Velocity;
		Velocity.Z *= JumpCutMultiplier;
		GetCharacterMovement()->Velocity = Velocity;
	}
}

void AIAmSadCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	StopGlide();

	// Check if space is being held
	APlayerController* PC = Cast<APlayerController>(GetController());
	bool bSpaceHeld = PC && PC->IsInputKeyDown(EKeys::SpaceBar);

	// Execute jump if holding jump or buffered
	if (bHoldingJump || bSpaceHeld || JumpBufferTimer > 0.0f)
	{
		JumpBufferTimer = 0.0f;
		Super::Jump();
	}
}

void AIAmSadCharacter::StartGlide()
{
	bIsGliding = true;
	bIsStalling = false;

	// Store starting position for trail
	LastGlidePosition = GetActorLocation();

	// Store original camera distance
	OriginalCameraDistance = CameraBoom->TargetArmLength;

	// Initialize glide with current momentum, start horizontal
	FVector Velocity = GetCharacterMovement()->Velocity;

	// Use sprite facing direction - it remembers which way you were last moving
	GlideDirection = SpriteForward;
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

void AIAmSadCharacter::UpdateFlipbook()
{
	if (!CharacterSprite)
	{
		return;
	}

	UPaperFlipbook* DesiredFlipbook = IdleFlipbook;

	if (bIsGliding)
	{
		DesiredFlipbook = GlideFlipbook;
	}
	else if (bGravityReversed)
	{
		DesiredFlipbook = ReverseGravityFlipbook;
	}
	else if (GetCharacterMovement()->IsFalling())
	{
		DesiredFlipbook = JumpFlipbook;
	}
	else if (FMath::Abs(GetCharacterMovement()->Velocity.Y) > 10.0f)
	{
		DesiredFlipbook = RunFlipbook;
	}

	// Only change if different to avoid resetting animation
	if (DesiredFlipbook && CharacterSprite->GetFlipbook() != DesiredFlipbook)
	{
		CharacterSprite->SetFlipbook(DesiredFlipbook);
	}
}
