// Fill out your copyright notice in the Description page of Project Settings.

#include "TanksVSZombies.h"
#include "Tank.h"
#include "PaperSpriteComponent.h"

void FTankInput::Sanitize()
{
	MovementInput = RawMovementInput.ClampAxes(-1.0f, 1.0f);
	MovementInput = MovementInput.GetSafeNormal();

	RawMovementInput.Set(0, 0);
}

void FTankInput::MoveX(float AxisValue)
{
	RawMovementInput.X += AxisValue;
}

void FTankInput::MoveY(float AxisValue)
{
	RawMovementInput.Y += AxisValue;
}

void FTankInput::Fire1(bool bPressed)
{
	bFire1 = bPressed;
}

void FTankInput::Fire2(bool bPressed)
{
	bFire2 = bPressed;
}

// Sets default values
ATank::ATank()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (!RootComponent)
	{
		RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	}

	TankDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("TankDirection"));
	TankDirection->SetupAttachment(RootComponent);
	TankSprite = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("TankSprite"));
	TankSprite->SetupAttachment(TankDirection);

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(GetRootComponent());
	Capsule->SetCollisionProfileName(TEXT("Pawn"));

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->TargetArmLength = 500.0f;
	SpringArm->bEnableCameraLag = true;
	SpringArm->bEnableCameraRotationLag = false;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->CameraLagSpeed = 2.0f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->SetWorldRotation(FRotator(-90, 0, -90));
	SpringArm->SetupAttachment(RootComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->bUsePawnControlRotation = false;
	CameraComponent->ProjectionMode = ECameraProjectionMode::Orthographic;
	CameraComponent->OrthoWidth = 1024.0f;
	CameraComponent->AspectRatio = 4.0f / 3.0f;
	CameraComponent->bConstrainAspectRatio = true;
	CameraComponent->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	ChildTurret = CreateDefaultSubobject<UChildActorComponent>(TEXT("ChildTurret"));
	ChildTurret->SetupAttachment(TankDirection);
}

// Called when the game starts or when spawned
void ATank::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATank::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	TankInput.Sanitize();

	// Move the tank!
	{
		FVector DesiredMovementDirection = FVector(TankInput.MovementInput.X, TankInput.MovementInput.Y, 0.0f);
		if (!DesiredMovementDirection.IsNearlyZero())
		{
			// Rotate the tank! Note that we rotate the TankDirection Component, 
			// not the RootComponent
			FRotator MovementAngle = DesiredMovementDirection.Rotation();
			float DeltaYaw = UTankStatics::FindDeltaAngleDegrees(TankDirection->GetComponentRotation().Yaw, MovementAngle.Yaw);
			bool bReverse = false;
			if (DeltaYaw != 0.0f)
			{
				float AdjustedDeltaYaw = DeltaYaw;

				if (AdjustedDeltaYaw < -90.0f)
				{
					AdjustedDeltaYaw += 180.0f;
					bReverse = true;
				}
				else if (AdjustedDeltaYaw > 90.0f)
				{
					AdjustedDeltaYaw -= 180.0f;
					bReverse = true;
				}

				// Turn toward the desired angle. Stop if we can get there in one frame.
				float MaxYawThisFrame = YawSpeed * DeltaTime;
				if (MaxYawThisFrame >= FMath::Abs(AdjustedDeltaYaw))
				{
					if (bReverse)
					{
						// Move backward
						FRotator FacingAngle = MovementAngle;
						FacingAngle.Yaw = MovementAngle.Yaw + 180.0f;
						TankDirection->SetWorldRotation(FacingAngle);
					}
					else
					{
						TankDirection->SetWorldRotation(MovementAngle);
					}
				}
				else
				{
					// Can't reach our desired angle this frame, rotate part way.
					TankDirection->AddLocalRotation(FRotator(0, FMath::Sign(AdjustedDeltaYaw) * MaxYawThisFrame, 0));
				}
			}


			// Move the tank
			{
				FVector MovementDirection = TankDirection->GetForwardVector() * (bReverse ? -1.0f : 1.0f);
				FVector Pos = GetActorLocation();
				Pos.X += MovementDirection.X * MoveSpeed * DeltaTime;
				Pos.Y += MovementDirection.Y * MoveSpeed * DeltaTime;
				SetActorLocation(Pos);
			}
		}
	}
}

// Called to bind functionality to input
void ATank::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	InputComponent->BindAxis(TEXT("MoveX"), this, &ATank::MoveX);
	InputComponent->BindAxis(TEXT("MoveY"), this, &ATank::MoveY);

	InputComponent->BindAction(TEXT("Fire1"), IE_Pressed, this, &ATank::Fire1Pressed);
	InputComponent->BindAction(TEXT("Fire1"), IE_Released, this, &ATank::Fire1Released);
	InputComponent->BindAction(TEXT("Fire2"), IE_Pressed, this, &ATank::Fire2Pressed);
	InputComponent->BindAction(TEXT("Fire2"), IE_Released, this, &ATank::Fire2Released);
}

void ATank::MoveX(float Value)
{
	TankInput.MoveX(Value);
}

void ATank::MoveY(float Value)
{
	TankInput.MoveY(Value);
}

void ATank::Fire1Pressed()
{
	TankInput.Fire1(true);
}

void ATank::Fire1Released()
{
	TankInput.Fire1(false);
}

void ATank::Fire2Pressed()
{
	TankInput.Fire2(true);
}

void ATank::Fire2Released()
{
	TankInput.Fire2(false);
}

