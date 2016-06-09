#pragma once

#include "Missile.generated.h"

UCLASS()
class AMissile : public AActor
{
	GENERATED_BODY()

public:
	AMissile();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
protected:
	/** What to do when the projectile explodes. The base version just destroys the projectile. */
	UFUNCTION(BlueprintNativeEvent, Category = "Projectile")
	void Explode();

	virtual void Explode_Implementation();

	UFUNCTION()
	void HandleExplosionFlipbookFinished();
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Missile")
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Missile")
	float LifeTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Missile")
	uint32 bExploding : 1;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Missile")
	class UPaperSpriteComponent* MissileSprite;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Missile")
	class UPaperFlipbookComponent* ExplosionFlipbook;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Missile")
	class UPaperFlipbook* ExplosionAsset;
};