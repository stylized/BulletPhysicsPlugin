#include "BulletPhysicsComponent.h"

void UBulletPhysicsComponent::BeginPlay()
{
	Super::BeginPlay();

	BulletSubsystem = GetWorld()->GetSubsystem<UBulletSubsystem>();
	BulletSubsystem->OnPhysicsTickDelegate.AddUObject(this, &UBulletPhysicsComponent::TickPhysics);
	BulletSubsystem->OnPostPhysicsTickDelegate.AddUObject(this, &UBulletPhysicsComponent::PostTickPhysics);
	BulletSubsystem->OnPostPhysicsFrameDelegate.AddUObject(this, &UBulletPhysicsComponent::PostPhysicsFrame);
	BulletSubsystem->OnPostFrameDelegate.AddUObject(this, &UBulletPhysicsComponent::PostFrame);
	BulletSubsystem->OnRollbackStartDelegate.AddUObject(this, &UBulletPhysicsComponent::OnRollbackStart);

	RigidBody = BulletSubsystem->AddRigidBody(GetOwner(), Friction, Restitution, Mass);
	RigidBody->setUserPointer(this);
	RigidBody->setFriction(Friction);
	RigidBody->setRollingFriction(RollingFriction);
	RigidBody->setSpinningFriction(SpinningFriction);
	RigidBody->setRestitution(Restitution);
	RigidBody->setContactStiffnessAndDamping(ContactStiffness, ContactDamping);

	if (bIsKinematic)
	{
		RigidBody->setCollisionFlags(RigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
	}
}

void UBulletPhysicsComponent::AddForce(const FVector& Force, const FVector& Offset)
{
	RigidBody->applyForce(BulletHelpers::ToBtDir(Force), BulletHelpers::ToBtPos(Offset));
}

void UBulletPhysicsComponent::AddTorque(const FVector& Torque)
{
	RigidBody->applyTorque(BulletHelpers::ToBtDir(Torque));
}

void UBulletPhysicsComponent::AddImpulse(const FVector& Impulse, const FVector& Offset)
{
	RigidBody->applyImpulse(BulletHelpers::ToBtDir(Impulse), BulletHelpers::ToBtPos(Offset));
}

void UBulletPhysicsComponent::AddCentralForce(const FVector& Force)
{
	RigidBody->applyCentralForce(BulletHelpers::ToBtDir(Force));
}

void UBulletPhysicsComponent::AddCentralImpulse(const FVector& Impulse)
{
	RigidBody->applyCentralImpulse(BulletHelpers::ToBtDir(Impulse));
}

FTransform UBulletPhysicsComponent::GetCenterOfMassTransform() const
{
	if (bIsKinematic)
	{
		return GetOwner()->GetActorTransform();
	}
	else
	{
		return BulletHelpers::ToUE(RigidBody->getCenterOfMassTransform());
	}
}

FVector UBulletPhysicsComponent::GetLinearVelocity() const
{
	return BulletHelpers::ToUEDir(RigidBody->getLinearVelocity());
}

FVector UBulletPhysicsComponent::GetAngularVelocity() const
{
	return BulletHelpers::ToUEDir(RigidBody->getAngularVelocity());
}

FVector UBulletPhysicsComponent::GetVelocityAtOffset(const FVector& Offset) const
{
	return BulletHelpers::ToUEDir(RigidBody->getVelocityInLocalPoint(BulletHelpers::ToBtPos(Offset)));
}

FVector UBulletPhysicsComponent::GetLocation() const
{
	return BulletHelpers::ToUEPos(RigidBody->getCenterOfMassPosition());
}

FQuat UBulletPhysicsComponent::GetQuaternion() const
{
	return BulletHelpers::ToUE(RigidBody->getOrientation());
}

FRotator UBulletPhysicsComponent::GetRotation() const
{
	return GetQuaternion().Rotator();
}

FVector UBulletPhysicsComponent::GetFinalLinearVelocity() const
{
	const btVector3 ForceImpulse = RigidBody->getTotalForce() * RigidBody->getInvMass() * BulletSubsystem->PhysicsDeltaTime;
	return BulletHelpers::ToUEDir(RigidBody->getLinearVelocity() + ForceImpulse);
}

FVector UBulletPhysicsComponent::GetFinalAngularVelocity() const
{
	const btVector3 TorqueImpulse = RigidBody->getTotalTorque() * RigidBody->getInvInertiaTensorWorld() * BulletSubsystem->PhysicsDeltaTime;
	return BulletHelpers::ToUEDir(RigidBody->getAngularVelocity() + TorqueImpulse);
}

FVector UBulletPhysicsComponent::GetFinalVelocityAtOffset(const FVector& Offset) const
{
	const btVector3 ForceImpulse = RigidBody->getTotalForce() * RigidBody->getInvMass() * BulletSubsystem->PhysicsDeltaTime;
	const btVector3 TorqueImpulse = RigidBody->getTotalTorque() * RigidBody->getInvInertiaTensorWorld() * BulletSubsystem->PhysicsDeltaTime;
	const btVector3 FinalLinearVelocity = RigidBody->getLinearVelocity() + ForceImpulse;
	const btVector3 FinalAngularVelocity = RigidBody->getAngularVelocity() + TorqueImpulse;
	return BulletHelpers::ToUEDir(FinalLinearVelocity + FinalAngularVelocity.cross(BulletHelpers::ToBtPos(Offset)));
}

void UBulletPhysicsComponent::SetCenterOfMassTransform(const FTransform& Transform)
{
	if (bIsKinematic)
	{
		GetOwner()->SetActorTransform(Transform);
	}
	else
	{
		RigidBody->setCenterOfMassTransform(BulletHelpers::ToBt(Transform));
	}
}

void UBulletPhysicsComponent::SetLinearVelocity(const FVector& Velocity)
{
	RigidBody->setLinearVelocity(BulletHelpers::ToBtDir(Velocity));
}

void UBulletPhysicsComponent::SetAngularVelocity(const FVector& Velocity)
{
	RigidBody->setAngularVelocity(BulletHelpers::ToBtDir(Velocity));
}

void UBulletPhysicsComponent::SaveKinematicState(float DeltaTime)
{
	RigidBody->saveKinematicState(DeltaTime);
}

void UBulletPhysicsComponent::SetGravity(const FVector& Gravity)
{
	RigidBody->setGravity(BulletHelpers::ToBtDir(Gravity));
}

void UBulletPhysicsComponent::ResetGravity()
{
	RigidBody->setGravity(BulletSubsystem->GetBulletWorld()->getGravity());
}

void UBulletPhysicsComponent::ClearForces()
{
	RigidBody->clearForces();
}

void UBulletPhysicsComponent::SetKinematic(bool bKinematic)
{
	bIsKinematic = bKinematic;

	if (!HasBegunPlay())
	{
		return;
	}

	if (bKinematic)
	{
		RigidBody->setCollisionFlags(RigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
	}
	else
	{
		RigidBody->setCollisionFlags(RigidBody->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
	}
}

void UBulletPhysicsComponent::SetMass(float NewMass)
{
	Mass = NewMass;

	if (!HasBegunPlay())
	{
		return;
	}

	btVector3 Inertia;
	RigidBody->getCollisionShape()->calculateLocalInertia(NewMass, Inertia);
	RigidBody->setMassProps(NewMass, Inertia);
}

void UBulletPhysicsComponent::TickPhysics(float DeltaTime)
{
	OnTickPhysics.Broadcast(DeltaTime);

	if (bIsKinematic)
	{
		UpdateKinematic(BulletSubsystem->GetTimeSeconds());
		SaveKinematicState(DeltaTime);
	}
}

void UBulletPhysicsComponent::PostTickPhysics(float DeltaTime)
{
	OnPostTickPhysics.Broadcast(DeltaTime);
}

void UBulletPhysicsComponent::OnRollbackStart(float RollbackTime)
{
	if (bIsKinematic)
	{
		// Save kinematic transform so we can get the right velocities on the first tick of the rollback
		UpdateKinematic(RollbackTime);
		SaveKinematicState(BulletSubsystem->PhysicsDeltaTime);
	}
}

void UBulletPhysicsComponent::UpdateKinematic(float CurrentTime)
{
	OnUpdateKinematic.Broadcast(CurrentTime);
}

void UBulletPhysicsComponent::PostFrame()
{
	UpdateKinematic(BulletSubsystem->GetInterpolatedTimeSeconds());
}