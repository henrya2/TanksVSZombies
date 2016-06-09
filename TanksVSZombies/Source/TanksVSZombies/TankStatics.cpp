#include "TanksVSZombies.h"
#include "TankStatics.h"

UTankStatics::UTankStatics()
{

}

float UTankStatics::FindDeltaAngleDegree(float A1, float A2)
{
	// Find the difference
	float Delta = A2 - A1;

	if (Delta > 180.0f)
	{
		Delta = Delta - 360.0f;
	}
	else if (Delta < -180.0f)
	{
		Delta = Delta + 360.0f;
	}

	return Delta;
}

bool UTankStatics::FindLookAtAngle2D(const FVector2D& Start, const FVector2D& Target, float& Angle)
{
	FVector2D Normal = (Target - Start).GetSafeNormal();
	if (!Normal.IsNearlyZero())
	{
		Angle = FMath::RadiansToDegrees(FMath::Atan2(Normal.Y, Normal.X));
		return true;
	}

	return false;
}
