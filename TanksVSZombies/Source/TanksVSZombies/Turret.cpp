#include "TanksVSZombies.h"
#include "Turret.h"
#include "Tank.h"
#include "Projectiles/Missile.h"
#include "PaperSpriteComponent.h"

ATurret::ATurret()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = TurretDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("TurretDirection"));

	TurretSprite = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("TurretSprite"));
	TurretSprite->SetupAttachment(RootComponent);

	YawSpeed = 270.0f;
	Fire1Cooldown = 1.0f;
	Fire1ReadyTime = 0.0f;
}

void ATurret::BeginPlay()
{
	Super::BeginPlay();

	// Be sure to aim only after the tank has turned and moved so that turret doesn't lag one frame behind
	UChildActorComponent* ChildActorComponent = GetParentComponent();
	if (ChildActorComponent)
	{
		Tank = Cast<ATank>(ChildActorComponent->GetOwner());
		AddTickPrerequisiteActor(Tank);
	}

	check(Tank);
}

void ATurret::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (Tank)
	{
		APlayerController* PC = Cast<APlayerController>(Tank->GetController());
		if (PC)
		{
			FVector2D AimLocation;
			if (PC->GetMousePosition(AimLocation.X, AimLocation.Y))
			{
				FVector2D TurretLocation = FVector2D::ZeroVector;
				PC->ProjectWorldLocationToScreen(TurretDirection->GetComponentLocation(), TurretLocation);

				float DesiredYaw;
				if (UTankStatics::FindLookAtAngle2D(TurretLocation, AimLocation, DesiredYaw))
				{
					FRotator CurrentRotation = TurretDirection->GetComponentRotation();
					float DeltaYaw = UTankStatics::FindDeltaAngleDegrees(CurrentRotation.Yaw, DesiredYaw);
					float MaxDeltaYawThisFrame = YawSpeed * DeltaSeconds;
					if (MaxDeltaYawThisFrame >= FMath::Abs(DeltaYaw))
					{
						CurrentRotation.Yaw = DesiredYaw;
					}
					else
					{
						CurrentRotation.Yaw += FMath::Sign(DeltaYaw) * MaxDeltaYawThisFrame;
					}

					TurretDirection->SetWorldRotation(CurrentRotation);
				}
			}
		}

		const FTankInput& CurrentInput = Tank->GetCurrentInput();
		if (CurrentInput.bFire1 && ProjectileActorClass)
		{
			if (UWorld* World = GetWorld())
			{
				//UE_LOG(LogTemp, Warning, TEXT("Fire1!"));
				float CurrentTime = World->GetTimeSeconds();
				if (CurrentTime >= Fire1ReadyTime)
				{
					FVector Loc = TurretSprite->GetSocketLocation(TEXT("Muzzle"));
					FRotator Rot(0, TurretSprite->GetComponentRotation().Yaw, 0); // Missile sprite already rotate -90 in yaw
					AMissile* NewProjectile = World->SpawnActor<AMissile>(ProjectileActorClass, Loc, Rot);

					Fire1ReadyTime = CurrentTime + Fire1Cooldown;
				}
			}
		}
	}
}
