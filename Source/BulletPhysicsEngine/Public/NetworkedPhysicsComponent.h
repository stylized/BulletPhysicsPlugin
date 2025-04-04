#pragma once

#include "BulletPhysicsComponent.h"
#include "NetworkedPhysicsComponent.generated.h"

struct FPhysicsObjectSnapshotPackedBits;

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class BULLETPHYSICSENGINE_API UNetworkedPhysicsComponent : public UBulletPhysicsComponent
{
	GENERATED_BODY()

public:
	UNetworkedPhysicsComponent();

	virtual void BeginPlay() override;

	virtual void SerializeSnapshot(FArchive& Ar);
	void ClientReadSnapshot(const FPhysicsObjectSnapshotPackedBits& PackedBits);
	void AddToSnapshot(FPhysicsSceneSnapshot& Snapshot);

private:
	/// Used for writing snapshot RPC bits
	FNetBitWriter SnapshotBitWriter;

	/// Used for reading snapshot RPC bits
	FNetBitReader SnapshotBitReader;
};