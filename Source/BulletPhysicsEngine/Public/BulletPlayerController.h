#pragma once

#include "GameFramework/PlayerController.h"
#include "NetSnapshot.h"
#include "BulletPlayerController.generated.h"

class UBulletSubsystem;

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class BULLETPHYSICSENGINE_API ABulletPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(Client, Unreliable)
	void ClientReceiveSnapshot(const FPhysicsSceneSnapshot& Snapshot);

protected:
	UBulletSubsystem* BulletSubsystem;
};