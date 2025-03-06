#pragma once

#include "GameFramework/GameState.h"
#include "NetSnapshot.h"
#include "BulletGameState.generated.h"

class UBulletSubsystem;

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class BULLETPHYSICSENGINE_API ABulletGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastReceiveSnapshot(const FPhysicsSceneSnapshot& Snapshot);

protected:
	UBulletSubsystem* BulletSubsystem;
};