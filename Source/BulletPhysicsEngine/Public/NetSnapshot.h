#pragma once

#include "Net/Core/NetToken/NetTokenExportContext.h"
#include "Serialization/Archive.h"
#include "NetSnapshot.generated.h"

class UNetworkedPhysicsComponent;

constexpr size_t OBJECT_SNAPSHOT_PACKEDBITS_RESERVED_SIZE = 1024;

USTRUCT()
struct FPhysicsObjectSnapshotPackedBits
{
	GENERATED_BODY()

	BULLETPHYSICSENGINE_API bool NetSerialize(FArchive& Ar, UPackageMap* PackageMap, bool& bOutSuccess);
	UPackageMap* GetPackageMap() const { return SavedPackageMap; }

	TObjectPtr<UNetworkedPhysicsComponent> Object = nullptr;

	// TInlineAllocator used with TBitArray takes the number of 32-bit dwords, but the define is in number of bits, so convert here by dividing by 32.
	TBitArray<TInlineAllocator<OBJECT_SNAPSHOT_PACKEDBITS_RESERVED_SIZE / NumBitsPerDWORD>> DataBits;

	// Since we capturing data outside of the normal serialization path we also need to store exports to inject when actually sending the data.
	UE::Net::FNetTokenExportContext::FNetTokenExports NetTokensPendingExport;

private:
	UPackageMap* SavedPackageMap;
};

template<>
struct TStructOpsTypeTraits<FPhysicsObjectSnapshotPackedBits> : public TStructOpsTypeTraitsBase2<FPhysicsObjectSnapshotPackedBits>
{
	enum
	{
		WithNetSerializer = true,
	};
};

USTRUCT()
struct FPhysicsSceneSnapshot
{
	GENERATED_BODY()

	UPROPERTY()
	int32 TickCount;
	UPROPERTY()
	TArray<FPhysicsObjectSnapshotPackedBits> Objects;
};