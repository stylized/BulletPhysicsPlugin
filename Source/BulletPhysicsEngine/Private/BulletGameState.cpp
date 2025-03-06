#include "BulletGameState.h"
#include "BulletSubsystem.h"
#include "NetworkedPhysicsComponent.h"

void ABulletGameState::BeginPlay()
{
	Super::BeginPlay();

	BulletSubsystem = GetWorld()->GetSubsystem<UBulletSubsystem>();
}

void ABulletGameState::MulticastReceiveSnapshot_Implementation(const FPhysicsSceneSnapshot& Snapshot)
{
	if (HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("MulticastReceiveSnapshot netmode %d"), GetWorld()->GetNetMode());

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
