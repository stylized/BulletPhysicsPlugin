#include "BulletPhysicsComponent.h"

void UBulletPhysicsComponent::BeginPlay()
{
	Super::BeginPlay();

	BulletSubsystem = GetWorld()->GetSubsystem<UBulletSubsystem>();
	UBulletSubsystem::OnPhysicsTickDelegate.AddUObject(this, &UBulletPhysicsComponent::TickPhysics);

	RigidBody = BulletSubsystem->AddRigidBody(GetOwner(), Friction, Restitution, Mass);
	RigidBody->setFriction(Friction);
	RigidBody->setRollingFriction(RollingFriction);
	RigidBody->setSpinningFriction(SpinningFriction);
	RigidBody->setRestitution(Restitution);
	RigidBody->setContactStiffnessAndDamping(ContactStiffness, ContactDamping);
}

void UBulletPhysicsComponent::AddForce(FVector Force, FVector Offset)
{
	RigidBody->applyForce(BulletHelpers::ToBtDir(Force), BulletHelpers::ToBtPos(Offset, FVector::ZeroVector));
}

void UBulletPhysicsComponent::AddTorque(FVector Torque)
{
	RigidBody->applyTorque(BulletHelpers::ToBtDir(Torque));
}

void UBulletPhysicsComponent::AddImpulse(FVector Impulse, FVector Offset)
{
	RigidBody->applyImpulse(BulletHelpers::ToBtDir(Impulse), BulletHelpers::ToBtPos(Offset, FVector::ZeroVector));
}

void UBulletPhysicsComponent::AddCentralForce(FVector Force)
{
	RigidBody->applyCentralForce(BulletHelpers::ToBtDir(Force));
}

void UBulletPhysicsComponent::AddCentralImpulse(FVector Impulse)
{
	RigidBody->applyCentralImpulse(BulletHelpers::ToBtDir(Impulse));
}

FVector UBulletPhysicsComponent::GetLinearVelocity() const
{
	return BulletHelpers::ToUEDir(RigidBody->getLinearVelocity());
}

FVector UBulletPhysicsComponent::GetVelocityAtOffset(FVector Offset) const
{
	return BulletHelpers::ToUEDir(RigidBody->getVelocityInLocalPoint(BulletHelpers::ToBtPos(Offset, FVector::ZeroVector)));
}