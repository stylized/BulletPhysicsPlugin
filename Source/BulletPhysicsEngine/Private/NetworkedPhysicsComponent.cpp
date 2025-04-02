#include "NetworkedPhysicsComponent.h"
#include "BulletPlayerController.h"
#include "NetHelper.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/EngineTypes.h"
#include "Engine/NetConnection.h"

DEFINE_LOG_CATEGORY_STATIC(LogNetworkedPhysics, Log, All);

namespace NetworkedPhysicsCVars
{
	static int32 NetPackedSnapshotMaxBits = 4096;
	FAutoConsoleVariableRef CVarNetPackedSnapshotMaxBits(
		TEXT("bullet.NetPackedSnapshotMaxBits"),
		NetPackedSnapshotMaxBits,
		TEXT("Max number of bits allowed in each object snapshot. Used to protect against bad data causing the client to allocate too much memory.\n"),
		ECVF_Default);
}

UNetworkedPhysicsComponent::UNetworkedPhysicsComponent() : Super()
{
	PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

	SnapshotBitWriter.SetAllowResize(true);
}

void UNetworkedPhysicsComponent::BeginPlay()
{
	Super::BeginPlay();

	BulletSubsystem->AddToSnapshotDelegate.AddUObject(this, &UNetworkedPhysicsComponent::AddToSnapshot);
}

void UNetworkedPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	// Ack cmds per frame after receiving all RPCs
	if (GetOwner()->HasAuthority() && LatestUserCmd.TickCount > LatestAckedCmdTick)
	{
		ClientAckUserCmd(LatestUserCmd.TickCount, BulletSubsystem->GetTickCount());
		LatestAckedCmdTick = LatestUserCmd.TickCount;
	}
}

void UNetworkedPhysicsComponent::TickPhysics(float DeltaTime)
{
	Super::TickPhysics(DeltaTime);

	const ENetRole Role = GetOwner()->GetLocalRole();

	if (Role == ROLE_AutonomousProxy && !BulletSubsystem->IsRollback())
	{
		FinalizeLatestUserCmd();
	}

	if (Role == ROLE_SimulatedProxy)
	{
		NetworkTickPhysics(LatestUserCmd, DeltaTime);
	}
	else
	{
		const FUserCmd& UserCmd = GetUserCmd(BulletSubsystem->GetTickCount());
		NetworkTickPhysics(UserCmd, DeltaTime);
	}
}

void UNetworkedPhysicsComponent::PostPhysicsFrame()
{
	if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		SendUnacknowledgedUserCmds();
	}
}

void UNetworkedPhysicsComponent::SerializeSnapshot(FArchive& Ar)
{
	FVector3f Location;
	FQuat4f Rotation;
	FVector3f LinearVelocity;
	FVector3f AngularVelocity;
	FUserCmd UserCmd;

	const bool bIsPawn = GetOwner()->IsA<APawn>();

	if (Ar.IsSaving())
	{
		const FTransform Transform = GetCenterOfMassTransform();
		Location = DoubleToFloat(Transform.GetLocation());
		Rotation = DoubleToFloat(Transform.GetRotation());
		LinearVelocity = DoubleToFloat(GetLinearVelocity());
		AngularVelocity = DoubleToFloat(GetAngularVelocity());

		if (bIsPawn)
		{
			UserCmd = LatestUserCmd;
		}
	}

	Ar << Location;
	Ar << Rotation;
	Ar << LinearVelocity;
	Ar << AngularVelocity;

	if (bIsPawn)
	{
		Ar << UserCmd;
	}

	if (Ar.IsLoading())
	{
		SetCenterOfMassTransform(FTransform(FloatToDouble(Rotation), FloatToDouble(Location)));
		SetLinearVelocity(FloatToDouble(LinearVelocity));
		SetAngularVelocity(FloatToDouble(AngularVelocity));

		if (bIsPawn && GetOwner()->GetLocalRole() == ROLE_SimulatedProxy)
		{
			LatestUserCmd = UserCmd;
		}
	}
}

void UNetworkedPhysicsComponent::FinalizeLatestUserCmd()
{
	const uint32 TickCount = BulletSubsystem->GetTickCount();
	LatestUserCmd.TickCount = TickCount;
	UserCmdBuffer[TickCount % UserCmdBufferSize] = LatestUserCmd;
}

void UNetworkedPhysicsComponent::SendUnacknowledgedUserCmds()
{
	TArray<FUserCmd> CmdsToSend;

	for (const FUserCmd& UserCmd : UserCmdBuffer)
	{
		if (UserCmd.TickCount > LatestAckedCmdTick)
		{
			CmdsToSend.Add(UserCmd);
		}
	}

	ServerReceiveUserCmds(CmdsToSend);
}

const FUserCmd& UNetworkedPhysicsComponent::GetUserCmd(int32 TickCount)
{
	if (GetWorld()->GetNetMode() == NM_Standalone)
	{
		// No usercmd networking
		return LatestUserCmd;
	}

	const FUserCmd& UserCmd = UserCmdBuffer[TickCount % UserCmdBufferSize];

	if (UserCmd.TickCount == TickCount)
	{
		return UserCmd;
	}
	else
	{
		// Use last received usercmd if the server runs out
		if (GetOwner()->GetRemoteRole() == ROLE_AutonomousProxy)
		{
			UE_LOG(LogNetworkedPhysics, Warning, TEXT("UserCmd not received in time for tick %d latest was %d"), TickCount, LatestUserCmd.TickCount);
		}

		return LatestUserCmd;
	}
}

void UNetworkedPhysicsComponent::SetInputForward(float Forward)
{
	LatestUserCmd.Throttle = Forward;
}

void UNetworkedPhysicsComponent::SetInputSide(float Side)
{
	LatestUserCmd.Steering = Side;
}

void UNetworkedPhysicsComponent::SetInputHandbrake(bool bHandbrake)
{
	LatestUserCmd.bHandbrake = bHandbrake;
}

void UNetworkedPhysicsComponent::SetInputDashLeft(bool bDashLeft)
{
	LatestUserCmd.bDashLeft = bDashLeft;
}

void UNetworkedPhysicsComponent::SetInputDashRight(bool bDashRight)
{
	LatestUserCmd.bDashRight = bDashRight;
}

void UNetworkedPhysicsComponent::ServerReceiveSingleUserCmd(const FUserCmd &UserCmd)
{
	UserCmdBuffer[UserCmd.TickCount % UserCmdBufferSize] = UserCmd;

	if (UserCmd.TickCount > LatestUserCmd.TickCount)
	{
		LatestUserCmd = UserCmd;
	}
}

void UNetworkedPhysicsComponent::ServerReceiveUserCmds_Implementation(const TArray<FUserCmd> &UserCmds)
{
	for (const FUserCmd& UserCmd : UserCmds)
	{
		ServerReceiveSingleUserCmd(UserCmd);
	}
}

void UNetworkedPhysicsComponent::ClientAckUserCmd_Implementation(int32 CmdTickCount, int32 ServerTickCount)
{
	if (CmdTickCount > LatestAckedCmdTick)
	{
		LatestAckedCmdTick = CmdTickCount;
	}

	if (CmdTickCount < ServerTickCount + 4)
	{
		BulletSubsystem->SkipToTick(ServerTickCount + 6);
	}
}

void UNetworkedPhysicsComponent::ClientReadSnapshot(const FPhysicsObjectSnapshotPackedBits& PackedBits)
{
	if (!HasBegunPlay())
	{
		return;
	}

	const int32 NumBits = PackedBits.DataBits.Num();
	if (NumBits > NetworkedPhysicsCVars::NetPackedSnapshotMaxBits)
	{
		// Protect against bad data that could cause client to allocate way too much memory.
		UE_LOG(LogNetworkedPhysics, Error, TEXT("ClientReadSnapshot (%s): Dropping snapshot due to NumBits (%d) exceeding allowable limit (%d). See NetPackedSnapshotMaxBits."), *GetNameSafe(GetOwner()), NumBits, NetworkedPhysicsCVars::NetPackedSnapshotMaxBits);
		return;
	}

	SnapshotBitReader.PackageMap = PackedBits.GetPackageMap();

	if (SnapshotBitReader.PackageMap == nullptr)
	{
		UE_LOG(LogNetworkedPhysics, Error, TEXT("ClientReadSnapshot: Failed to find PackageMap for data serialization!"));
		return;
	}

	// Reuse bit reader to avoid allocating memory each time
	SnapshotBitReader.SetData((uint8*)PackedBits.DataBits.GetData(), NumBits);

	// Deserialize bits and apply snapshot
	SerializeSnapshot(SnapshotBitReader);
}

void UNetworkedPhysicsComponent::AddToSnapshot(FPhysicsSceneSnapshot& Snapshot)
{
	// Reset bit writer without affecting allocations
	FBitWriterMark BitWriterReset;
	BitWriterReset.Pop(SnapshotBitWriter);

	// 'static' to avoid reallocation each invocation
	static FPhysicsObjectSnapshotPackedBits PackedBits;

	PackedBits.Object = this;

	// Extract the net package map used for serializing object references.
	UNetConnection* NetConnection = Snapshot.Receiver->GetNetConnection();
	SnapshotBitWriter.PackageMap = NetConnection ? ToRawPtr(NetConnection->PackageMap) : nullptr;

	if (SnapshotBitWriter.PackageMap == nullptr)
	{
		UE_LOG(LogNetworkedPhysics, Error, TEXT("AddToSnapshot: Failed to find a NetConnection/PackageMap for data serialization!"));
		return;
	}

	// Reset NetTokensPendingExport as PackedBits is reused
	PackedBits.NetTokensPendingExport.Reset();
	UE::Net::FNetTokenExportScope NetTokenExportScope(SnapshotBitWriter, NetConnection->GetDriver()->GetNetTokenStore(), PackedBits.NetTokensPendingExport, "AddToSnapshot");

	// Serialize snapshot into a bit stream
	SerializeSnapshot(SnapshotBitWriter);

	// Copy bits to our struct that we can NetSerialize to the clients
	PackedBits.DataBits.SetNumUninitialized(SnapshotBitWriter.GetNumBits());

	check(PackedBits.DataBits.Num() >= SnapshotBitWriter.GetNumBits());
	FMemory::Memcpy(PackedBits.DataBits.GetData(), SnapshotBitWriter.GetData(), SnapshotBitWriter.GetNumBytes());

	// Add it to the full scene snapshot
	Snapshot.Objects.Add(PackedBits);
}

bool FPhysicsObjectSnapshotPackedBits::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	SavedPackageMap = Map;

	// Physics object net ID
	Ar << Object;

	// Array size in bits, using minimal number of bytes to write it out.
	uint32 NumBits = DataBits.Num();
	Ar.SerializeIntPacked(NumBits);

	if (NumBits > static_cast<uint32>(NetworkedPhysicsCVars::NetPackedSnapshotMaxBits))
	{
		// Protect against bad data that could cause clients to allocate way too much memory.
		UE_LOG(LogNetworkedPhysics, Error, TEXT("FPhysicsObjectSnapshotPackedBits::NetSerialize: Dropping snapshot due to NumBits (%d) exceeding allowable limit (%d). See NetPackedSnapshotMaxBits."), NumBits, NetworkedPhysicsCVars::NetPackedSnapshotMaxBits);
		return false;
	}

	if (Ar.IsLoading())
	{
		DataBits.Init(0, NumBits);
	}
	else if (Ar.IsSaving() && NetTokensPendingExport.Num())
	{
		// As we now support exporting NetTokens from shared serialization and FPhysicsObjectSnapshotPackedBits serializes data outside of the normal flow
		// we explicitly capture exports which we needs to be inject during actual serialization.
		if (UE::Net::FNetTokenExportContext* ExportContext = UE::Net::FNetTokenExportContext::GetNetTokenExportContext(Ar))
		{
			ExportContext->AppendNetTokensPendingExport(NetTokensPendingExport);
		}
	}

	// Array data
	Ar.SerializeBits(DataBits.GetData(), NumBits);

	bOutSuccess = true;
	return !Ar.IsError();
}