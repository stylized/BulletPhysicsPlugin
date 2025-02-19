#pragma once

#include "BulletSubsystem.h"
#include "Components/ActorComponent.h"
#include "BulletPhysicsComponent.generated.h"

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class BULLETPHYSICSENGINE_API UBulletPhysicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	void AddForce(const FVector& Force, const FVector& Offset);
	void AddTorque(const FVector& Torque);
	void AddImpulse(const FVector& Impulse, const FVector& Offset);
	void AddCentralForce(const FVector& Force);
	void AddCentralImpulse(const FVector& Impulse);

	FTransform GetCenterOfMassTransform() const;
	FVector GetLinearVelocity() const;
	FVector GetAngularVelocity() const;
	FVector GetVelocityAtOffset(const FVector& Offset) const;

	void SetCenterOfMassTransform(const FTransform& Transform) const;
	void SetLinearVelocity(const FVector& Velocity) const;
	void SetAngularVelocity(const FVector& Velocity) const;

	virtual void TickPhysics(float DeltaTime)
	{
	}

	virtual void PostPhysicsFrame()
	{
	}

	UPROPERTY(EditAnywhere)
	float Friction = .5f;
	UPROPERTY(EditAnywhere)
	float RollingFriction = .5f;
	UPROPERTY(EditAnywhere)
	float SpinningFriction = .5f;
	UPROPERTY(EditAnywhere)
	float Restitution = 0.f;
	UPROPERTY(EditAnywhere)
	float ContactDamping = .1f;
	UPROPERTY(EditAnywhere)
	float ContactStiffness = BT_LARGE_FLOAT;
	UPROPERTY(EditAnywhere)
	float Mass = 10.f;

protected:
	btRigidBody* RigidBody;
	UBulletSubsystem* BulletSubsystem;
};