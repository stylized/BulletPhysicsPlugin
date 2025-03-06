#pragma once

#include "BulletPhysicsComponent.h"
#include "NetworkedPhysicsComponent.generated.h"

struct FPhysicsObjectSnapshotPackedBits;

USTRUCT()
struct FUserCmd
{
	GENERATED_BODY()

	inline friend FArchive& operator<<(FArchive& Ar, FUserCmd& UserCmd)
	{
		Ar << UserCmd.TickCount;
		Ar << UserCmd.Throttle;
		Ar << UserCmd.Steering;
		Ar.SerializeBits(&UserCmd.Handbrake, 1);
		return Ar;
	}

	UPROPERTY()
	int32 TickCount = -1;
	UPROPERTY()
	float Throttle = 0.f;
	UPROPERTY()
	float Steering = 0.f;
	UPROPERTY()
	bool Handbrake = false;
};

UCLASS()
class BULLETPHYSICSENGINE_API UNetworkedPhysicsComponent : public UBulletPhysicsComponent
{
	GENERATED_BODY()

public:
	UNetworkedPhysicsComponent();

private:
	static constexpr uint32 UserCmdBufferSize = 256;

public:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void TickPhysics(float DeltaTime) override;

	virtual void PostPhysicsFrame() override;

	virtual void NetworkTickPhysics(const FUserCmd& UserCmd, float DeltaTime)
	{
	}

	virtual void SendSnapshotToClients();

	virtual void SerializeSnapshot(FArchive& Ar);
	void ClientReadSnapshot(const FPhysicsObjectSnapshotPackedBits& PackedBits);
	void AddToSnapshot(FPhysicsSceneSnapshot& Snapshot);

	void FinalizeLatestUserCmd();

	void SendUnacknowledgedUserCmds();

	const FUserCmd& GetUserCmd(int32 TickCount);

	UFUNCTION(BlueprintCallable)
	void SetInputForward(float Forward);
	UFUNCTION(BlueprintCallable)
	void SetInputSide(float Side);
	UFUNCTION(BlueprintCallable)
	void SetInputHandbrake(bool Handbrake);

	void ServerReceiveSingleUserCmd(const FUserCmd &UserCmd);

	UFUNCTION(unreliable, server)
	void ServerReceiveUserCmds(const TArray<FUserCmd> &UserCmds);

	UFUNCTION(unreliable, client)
	void ClientAckUserCmd(int32 CmdTickCount, int32 ServerTickCount);

private:
	FUserCmd LatestUserCmd;
	FUserCmd UserCmdBuffer[UserCmdBufferSize];

	/// Tick of latest ack sent to server on client, or latest ack sent to client on server
	int32 LatestAckedCmdTick = -1;

	/// Used for writing snapshot RPC bits
	FNetBitWriter SnapshotBitWriter;

	/// Used for reading snapshot RPC bits
	FNetBitReader SnapshotBitReader;
};