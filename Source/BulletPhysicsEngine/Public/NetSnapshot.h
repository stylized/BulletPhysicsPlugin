#pragma once

#include "CoreMinimal.h"
#include "Serialization/Archive.h"
#include "NetSnapshot.generated.h"

class UNetworkedPhysicsComponent;

constexpr size_t OBJECT_SNAPSHOT_PACKEDBITS_RESERVED_SIZE = 1024;

USTRUCT()
struct FPhysicsObjectSnapshotPackedBits
{
	GENERATED_BODY()

	TObjectPtr<UNetworkedPhysicsComponent> Object = nullptr;

	// TInlineAllocator used with TBitArray takes the number of 32-bit dwords, but the define is in number of bits, so convert here by dividing by 32.
	TBitArray<TInlineAllocator<OBJECT_SNAPSHOT_PACKEDBITS_RESERVED_SIZE / NumBitsPerDWORD>> DataBits;
};

USTRUCT()
struct FPhysicsSceneSnapshot
{
	GENERATED_BODY()

	int32 TickCount;
	TArray<FPhysicsObjectSnapshotPackedBits> Objects;
};