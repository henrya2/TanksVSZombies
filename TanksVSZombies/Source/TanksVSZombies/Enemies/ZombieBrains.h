// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "ZombieBrains.generated.h"

class AZombie;

/**
 * 
 */
UCLASS()
class TANKSVSZOMBIES_API AZombieBrains : public AAIController
{
	GENERATED_BODY()

public:
	AZombieBrains();

	virtual void Tick(float DeltaTime) override;

	virtual void Possess(APawn* InPawn) override;
	virtual void UnPossess() override;

protected:
	// Our pawn, pre-cast to a Zombie. Will be NULL if the pawn is NULL or is not a Zombie.
	UPROPERTY(BlueprintReadOnly, Category = "Zombie")
	AZombie* PawnAsZombie;
};
