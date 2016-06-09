#pragma once

#include "Turret.generated.h"

class ATank;
class AMissile;

UCLASS()
class ATurret : public AActor
{
	GENERATED_BODY()
		
public:
	ATurret();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	FORCEINLINE UArrowComponent* GetTurretDirection() { return TurretDirection; }

protected:
	// Max turn rate in degrees/second for the turret.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret")
	float YawSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret")
	ATank* Tank;

	// Time to delay between Fire1 commands.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret", meta = (AllowPrivateAccess = "true"))
	float Fire1Cooldown;

	// If this value is greater than the current game time, Fire1 is ignored because it has been fired too recently.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret", meta = (AllowPrivateAccess = "true"))
	float Fire1ReadyTime;
private:
	// Helpful debug tool - which way is the turret facing?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret", meta = (AllowPrivateAccess = "true"))
	UArrowComponent* TurretDirection;

	// Sprite for the turret.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret", meta = (AllowPrivateAccess = "true"))
	class UPaperSpriteComponent* TurretSprite;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AMissile> ProjectileActorClass;
};