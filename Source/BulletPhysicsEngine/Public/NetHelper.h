#pragma once

#include "Math/UnrealMath.h"

inline FVector FloatToDouble(const FVector3f& V)
{
	return FVector(V.X, V.Y, V.Z);
}

inline FVector3f DoubleToFloat(const FVector& V)
{
	return FVector3f(V.X, V.Y, V.Z);
}

inline FRotator FloatToDouble(const FRotator3f& R)
{
	return FRotator(R.Pitch, R.Yaw, R.Roll);
}

inline FRotator3f DoubleToFloat(const FRotator& R)
{
	return FRotator3f(R.Pitch, R.Yaw, R.Roll);
}

inline FQuat FloatToDouble(const FQuat4f& Q)
{
	return FQuat(Q.X, Q.Y, Q.Z, Q.W);
}

inline FQuat4f DoubleToFloat(const FQuat& Q)
{
	return FQuat4f(Q.X, Q.Y, Q.Z, Q.W);
}