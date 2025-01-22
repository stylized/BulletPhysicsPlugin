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

	void AddForce(FVector Force, FVector Offset);
	void AddImpulse(FVector Impulse, FVector Offset);
	void AddCentralForce(FVector Force);
	void AddCentralImpulse(FVector Impulse);
	FVector GetLinearVelocity() const;
	FVector GetVelocityAtOffset(FVector Offset) const;

	virtual void TickPhysics(float DeltaTime)
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