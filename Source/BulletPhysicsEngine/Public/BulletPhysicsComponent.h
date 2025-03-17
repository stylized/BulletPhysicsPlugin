#pragma once

#include "BulletSubsystem.h"
#include "Components/ActorComponent.h"
#include "BulletPhysicsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTickPhysics, float, DeltaTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateKinematic, float, CurrentTime);

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class BULLETPHYSICSENGINE_API UBulletPhysicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	static UBulletPhysicsComponent* GetFromBulletObject(const btCollisionObject* Object)
	{
		return reinterpret_cast<UBulletPhysicsComponent*>(Object->getUserPointer());
	}

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Bullet Physics")
	void AddForce(const FVector& Force, const FVector& Offset);
	UFUNCTION(BlueprintCallable, Category = "Bullet Physics")
	void AddTorque(const FVector& Torque);
	UFUNCTION(BlueprintCallable, Category = "Bullet Physics")
	void AddImpulse(const FVector& Impulse, const FVector& Offset);
	UFUNCTION(BlueprintCallable, Category = "Bullet Physics")
	void AddCentralForce(const FVector& Force);
	UFUNCTION(BlueprintCallable, Category = "Bullet Physics")
	void AddCentralImpulse(const FVector& Impulse);

	UFUNCTION(BlueprintCallable, Category = "Bullet Physics")
	FTransform GetCenterOfMassTransform() const;
	UFUNCTION(BlueprintCallable, Category = "Bullet Physics")
	FVector GetLinearVelocity() const;
	UFUNCTION(BlueprintCallable, Category = "Bullet Physics")
	FVector GetAngularVelocity() const;
	UFUNCTION(BlueprintCallable, Category = "Bullet Physics")
	FVector GetVelocityAtOffset(const FVector& Offset) const;
	UFUNCTION(BlueprintCallable, Category = "Bullet Physics")
	FVector GetLocation() const;
	UFUNCTION(BlueprintCallable, Category = "Bullet Physics")
	FQuat GetQuaternion() const;
	UFUNCTION(BlueprintCallable, Category = "Bullet Physics")
	FRotator GetRotation() const;

	UFUNCTION(BlueprintCallable, Category = "Bullet Physics")
	void SetCenterOfMassTransform(const FTransform& Transform);
	UFUNCTION(BlueprintCallable, Category = "Bullet Physics")
	void SetLinearVelocity(const FVector& Velocity);
	UFUNCTION(BlueprintCallable, Category = "Bullet Physics")
	void SetAngularVelocity(const FVector& Velocity);

	UFUNCTION(BlueprintCallable, Category = "Bullet Physics")
	void SaveKinematicState(float DeltaTime);

	virtual void TickPhysics(float DeltaTime);

	virtual void OnRollbackStart(float RollbackTime);

	virtual void UpdateKinematic(float CurrentTime);

	virtual void PostPhysicsFrame()
	{
	}

	virtual void PostFrame();

	virtual void OnNotifyCollision(const btManifoldPoint& Point)
	{
	}

	UPROPERTY(BlueprintAssignable)
	FOnTickPhysics OnTickPhysics;

	UPROPERTY(BlueprintAssignable)
	FOnUpdateKinematic OnUpdateKinematic;

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
	UPROPERTY(EditAnywhere)
	/// Object has collision, but isn't moved by Bullet simulation. Transform is controlled by UE actor. Mass should be 0.
	bool bIsKinematic = false;

protected:
	btRigidBody* RigidBody;
	UBulletSubsystem* BulletSubsystem;
};