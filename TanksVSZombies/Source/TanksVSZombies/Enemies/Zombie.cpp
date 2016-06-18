// Fill out your copyright notice in the Description page of Project Settings.

#include "TanksVSZombies.h"
#include "Zombie.h"
#include "Tank.h"
#include "PaperFlipbookComponent.h"


// Sets default values
AZombie::AZombie()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (!RootComponent)
	{
		RootComponent = (CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot")));
		RootComponent->Mobility = EComponentMobility::Movable;
	}

	PaperFlipbookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("PaperFlipbook"));
	PaperFlipbookComponent->SetupAttachment(RootComponent);
	PaperFlipbookComponent->RelativeRotation = FRotator(0, 0, -90);

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(RootComponent);
	Capsule->SetCollisionProfileName(TEXT("ZombiesGeneral"));

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	Health = 100.0f;
	SightDistance = 500.0f;
	SightAngle = 60.0f;
	YawSpeed = 90.0f;
	WalkSpeed = 25.0f;
	RunSpeed = 45.0f;
	AttackDistance = 100.0f;
	AttackAngle = 30.0f;
	AttackCooldown = 1.0f;
}

// Called when the game starts or when spawned
void AZombie::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AZombie::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	ZombieAI(DeltaTime);

	UpdateZombieFlipbookAnim();
}

void AZombie::SetTarget(AActor* InTarget)
{
	TargetActor = InTarget;

	TargetTank = Cast<ATank>(InTarget);
}

void AZombie::ZombieAI_Implementation(float DeltaSeconds)
{
	// The zombie always moves unless attacking. If moving, it moves between WalkSpeed and RunSpeed.
	FVector DesiredMovement = GetAttackInput() ? FVector::ZeroVector : (FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 1.0f), FVector2D(WalkSpeed, RunSpeed), GetPendingMovementInputVector().X)) * DeltaSeconds * GetActorForwardVector();
	FVector OriginalLocation = GetActorLocation();
	FVector DesiredLoc = OriginalLocation + DesiredMovement;
	float MaxYawThisFrame = YawSpeed * DeltaSeconds;
	FRotator DeltaRot = FRotator(0.0f, FMath::Clamp(GetRotationInput(), -MaxYawThisFrame, MaxYawThisFrame), 0.0f);
	if (DeltaRot.IsNearlyZero())
	{
		DeltaRot.Yaw = FMath::RandRange(-MaxYawThisFrame, MaxYawThisFrame);
	}
	FRotator DesiredRot = GetActorRotation() + DeltaRot;

	SetActorLocationAndRotation(DesiredLoc, DesiredRot.Quaternion(), true);
	FVector DistanceWalked = GetActorLocation() - OriginalLocation;
	if (!DistanceWalked.IsNearlyZero())
	{
		ZombieWalk(DeltaSeconds, DistanceWalked);
	}

	// See if we have a target and deal with it if we do. Find a target if we don't.
	if (AActor* Target = GetTarget())
	{
		// We do have a target. See if we're attacking, since our attacks always hit.
		FVector OurLocation = GetActorLocation();
		FVector DirectionToTarget = (Target->GetActorLocation() - OurLocation).GetSafeNormal2D();
		float DotToTarget = FVector::DotProduct(DirectionToTarget, GetActorForwardVector());

		// Check to see if we should attack based on button status and attack cooldown.
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (GetAttackInput() && (AttackAvailableTime <= CurrentTime))
		{
			AttackAvailableTime = CurrentTime + AttackCooldown;
			ZombieAttack(DeltaSeconds);
			if (DotToTarget >= FMath::Cos(FMath::DegreesToRadians(AttackAngle)))
			{
				float DistSqXY = FVector::DistSquaredXY(Target->GetActorLocation(), OurLocation);
				if (DistSqXY <= (AttackDistance * AttackDistance))
				{
					if (ATank* TankTarget = GetTargetAsTank())
					{
						//TankTarget->DamageHealth(10.0f);
						if (APlayerController* PC = Cast<APlayerController>(TankTarget->GetController()))
						{
							PC->ClientPlayCameraShake(HitShake, 1.0f);
						}
					}
					else
					{
						SetTarget(nullptr);
					}
				}
			}
		}
	}
	else
	{
		// Look for a target. We might not do this every single frame, but for now it's OK.
		// TODO: Make this use a list of registered targets so we can handle multiplayer or add decoys.
		Target = UGameplayStatics::GetPlayerPawn(this, 0);
		float DistSqXY = FVector::DistSquaredXY(Target->GetActorLocation(), GetActorLocation());
		if (DistSqXY <= (SightDistance * SightDistance))
		{
			FVector DirectionToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
			if (FVector::DotProduct(DirectionToTarget, GetActorForwardVector()) >= FMath::Cos(FMath::DegreesToRadians(SightAngle)))
			{
				SetTarget(Target);
			}
		}
	}
}

bool AZombie::ZombieAIShouldAttack_Implementation()
{
	if (AActor* Target = GetTarget())
	{
		// Attack our target if we're in range (distance and angle). For now, we'll use our unmodified attack variables for this.
		FVector OurLocation = GetActorLocation();
		FVector DirectionToTarget = (Target->GetActorLocation() - OurLocation).GetSafeNormal2D();
		float DotToTarget = FVector::DotProduct(DirectionToTarget, GetActorForwardVector());
		if (DotToTarget >= FMath::Cos(FMath::DegreesToRadians(AttackAngle)))
		{
			float DistSqXY = FVector::DistSquaredXY(Target->GetActorLocation(), OurLocation);
			if (DistSqXY <= (AttackDistance * AttackDistance))
			{
				// Note that attacking cooldown isn't checked. We don't want this kind of zombie to move between attacks or think it's not attacking while waiting for the cooldown.
				return true;
			}
		}
	}
	return false;
}

void AZombie::ZombieWalk_Implementation(float DeltaSeconds, FVector DistanceWalkedThisFrame)
{
	ConsumeMovementInputVector();
	ConsumeRotationInput();

	ChangeZombieAnimState(EZombieAnimState::EZAS_Walk);
}

void AZombie::ZombieAttack_Implementation(float DeltaSeconds)
{
	ConsumeAttackInput();

	ChangeZombieAnimState(EZombieAnimState::EZAS_Attack);
}

void AZombie::ChangeZombieAnimState(EZombieAnimState InAnimState)
{
	ZombieAnimState = InAnimState;
}

void AZombie::UpdateZombieFlipbookAnim()
{
	switch (ZombieAnimState)
	{
	case EZombieAnimState::EZAS_Stand:
		{
			PaperFlipbookComponent->SetFlipbook(StandFlipbook);
		}
		break;

	case EZombieAnimState::EZAS_Walk:
		{
			PaperFlipbookComponent->SetFlipbook(WalkFlipbook);
		}
		break;

	case EZombieAnimState::EZAS_Attack:
		{
			PaperFlipbookComponent->SetFlipbook(AttackFlipbook);
		}
		break;
	}
}

void AZombie::AddRotationInput(float DeltaYawDegrees)
{
	YawInput += DeltaYawDegrees;
}

float AZombie::GetRotationInput()
{
	return YawInput;
}

float AZombie::ConsumeRotationInput()
{
	float RetYawInput = YawInput;
	YawInput = 0.0f;
	return RetYawInput;
}

void AZombie::AddAttackInput()
{
	bAttackInput = true;
}

bool AZombie::GetAttackInput()
{
	return bAttackInput;
}

bool AZombie::ConsumeAttackInput()
{
	bool bRetVal = bAttackInput;
	bAttackInput = false;
	return bRetVal;
}
