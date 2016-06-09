#include "TanksVSZombies.h"
#include "Missile.h"
#include "PaperSpriteComponent.h"
#include "PaperFlipbookComponent.h"

AMissile::AMissile()
{
	PrimaryActorTick.bCanEverTick = true;

	Speed = 200.0f;

	LifeTime = 1.0f;

	if (!RootComponent)
	{
		RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	}

	MissileSprite = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("MissileSprite"));
	MissileSprite->RelativeRotation = FRotator(0, 0, -90);
	MissileSprite->RelativeLocation = FVector(0, 0, 5.0f);
	MissileSprite->SetupAttachment(RootComponent);

	ExplosionFlipbook = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("ExplosionFlipbook"));
	ExplosionFlipbook->SetupAttachment(MissileSprite);
	ExplosionFlipbook->bAutoActivate = false;
	ExplosionFlipbook->Stop();
}

void AMissile::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle DummyTimerHandle;
	GetWorldTimerManager().SetTimer(DummyTimerHandle, this, &AMissile::Explode, LifeTime);
}

void AMissile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bExploding)
	{
		FVector Loc = GetActorLocation();
		Loc += (DeltaSeconds * Speed) * GetActorForwardVector();
		SetActorLocation(Loc);
	}
}

void AMissile::Explode_Implementation()
{
	if (ExplosionAsset)
	{
		bExploding = true;

		MissileSprite->SetHiddenInGame(true);
		ExplosionFlipbook->SetFlipbook(ExplosionAsset);
		ExplosionFlipbook->SetLooping(false);
		ExplosionFlipbook->PlayFromStart();
		ExplosionFlipbook->OnFinishedPlaying.AddDynamic(this, &AMissile::HandleExplosionFlipbookFinished);
	}
	else
	{
		Destroy();
	}
}

void AMissile::HandleExplosionFlipbookFinished()
{
	Destroy();
}
