#include "BulletPlayerController.h"
#include "BulletSubsystem.h"
#include "NetworkedPhysicsComponent.h"

void ABulletPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BulletSubsystem = GetWorld()->GetSubsystem<UBulletSubsystem>();
}

void ABulletPlayerController::ClientReceiveSnapshot_Implementation(const FPhysicsSceneSnapshot& Snapshot)
{
	if (BulletSubsystem == nullptr)
	{
		return;
	}

	for (const FPhysicsObjectSnapshotPackedBits& ObjectSnapshot : Snapshot.Objects)
	{
		if (ObjectSnapshot.Object != nullptr)
		{
			ObjectSnapshot.Object->ClientReadSnapshot(ObjectSnapshot);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Object from physics snapshot could not be resolved on client"));
		}
	}

	BulletSubsystem->StartRollback(Snapshot.TickCount);
}
