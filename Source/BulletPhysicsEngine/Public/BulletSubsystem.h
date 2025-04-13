// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PhysicsEngine/BodySetup.h"
#include "BulletPhysicsEngine/bthelper.h"
#include "BulletPhysicsEngine/motionstate.h"
#include "BulletPhysicsEngine/BulletMain.h"
#include "BulletPhysicsEngine/btdebug.h"
#include "NetSnapshot.h"
#include "Components/ShapeComponent.h"
#include <functional>
#include "GameFramework/Actor.h"
#include "Subsystems/SubsystemCollection.h"
#include "Templates/Function.h"

#include "BulletSubsystem.generated.h"

class ULandscapeComponent;
class USplineMeshComponent;

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FRayTestSingleCallback, const FVector&, To, const FVector&, From, bool&, HasHit);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPhysicsTick, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPostPhysicsTick, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPostPhysicsFrame, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPostFrame, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FAddToSnapshot, FPhysicsSceneSnapshot&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnRollbackStart, float);

struct FClosestRayResultWithExclude : btCollisionWorld::ClosestRayResultCallback
{
	FClosestRayResultWithExclude(const btVector3& rayFromWorld, const btVector3& rayToWorld, const btCollisionObject* excludeObject)
		: btCollisionWorld::ClosestRayResultCallback(rayFromWorld, rayToWorld)
		, m_excludeObject(excludeObject)
	{
	}

	virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) override
	{
		if (rayResult.m_collisionObject == m_excludeObject)
		{
			return 1.0;
		}
		return btCollisionWorld::ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
	}

	const btCollisionObject* m_excludeObject;
};

struct FClosestConvexResultWithExclude : btCollisionWorld::ClosestConvexResultCallback
{
	FClosestConvexResultWithExclude(const btVector3& convexFromWorld, const btVector3& convexToWorld, const btCollisionObject* excludeObject)
		: btCollisionWorld::ClosestConvexResultCallback(convexFromWorld, convexToWorld)
		, m_excludeObject(excludeObject)
	{
	}

	virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace) override
	{
		if (convexResult.m_hitCollisionObject == m_excludeObject)
		{
			return 1.0;
		}
		return btCollisionWorld::ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
	}

	const btCollisionObject* m_excludeObject;
};

struct FContactPoint
{
	FVector LocalPointOnSelf;
	FVector LocalPointOnOther;
	FVector PointOnSelf;
	FVector PointOnOther;
	FVector Normal;
	btScalar AppliedImpulse;
	const btCollisionObject* OtherObject;
};

class CustomBulletWorld : public btDiscreteDynamicsWorld
{
public:
	CustomBulletWorld(btDispatcher* dispatcher, btBroadphaseInterface* pairCache, btConstraintSolver* constraintSolver, btCollisionConfiguration* collisionConfiguration)
		: btDiscreteDynamicsWorld(dispatcher, pairCache, constraintSolver, collisionConfiguration)
	{
	}

	virtual int stepSimulation(btScalar timeStep, int maxSubSteps = 1, btScalar fixedTimeStep = btScalar(1.) / btScalar(60.)) override;

	virtual void stepTicks(int subSteps = 1, btScalar fixedTimeStep = btScalar(1.) / btScalar(60.));

	btScalar getLocalTime() const
	{
		return m_localTime;
	}

	btScalar getTimeScale() const
	{
		return m_timeScale;
	}

	void setTimeScale(btScalar newTimeScale)
	{
		m_timeScale = newTimeScale;
	}

	int getExtraSimulationTicks() const
	{
		return m_extraSimulationTicks;
	}

	void addExtraSimulationTicks(int ticks)
	{
		m_extraSimulationTicks += ticks;
	}

private:
	btScalar m_timeScale = 1.;
	int m_extraSimulationTicks = 0;
};

UCLASS()
	class BULLETPHYSICSENGINE_API UBulletSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

	public:
		/// Called before each Bullet physics tick, used to add forces
		FOnPhysicsTick OnPhysicsTickDelegate;
		/// Called after each Bullet physics tick and after collision notify
		FOnPhysicsTick OnPostPhysicsTickDelegate;
		/// Called after Bullet simulation has run for a frame (one or more ticks)
		FOnPostPhysicsFrame OnPostPhysicsFrameDelegate;
		/// Called after Bullet subsystem frame, regardless of whether there was a simulation tick
		FOnPostFrame OnPostFrameDelegate;
		/// Used for objects to add themself to the physics scene snapshots sent from the server
		FAddToSnapshot AddToSnapshotDelegate;
		/// Called before the first simulation tick of a rollback
		FOnRollbackStart OnRollbackStartDelegate;

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|Objects")
			void AddStaticBody(AActor* Body, float Friction, float Restitution,int &ID);

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|Objects")
			void AddProcBody(AActor* Body,  float Friction, TArray<FVector> a, TArray<FVector> b, TArray<FVector> c, TArray<FVector> d, float Restitution, int& ID);

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|Objects")
			void UpdateProcBody(AActor* Body, float Friction, TArray<FVector> a, TArray<FVector> b, TArray<FVector> c, TArray<FVector> d, float Restitution, int& ID, int PrevID);

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|Objects")
			void AddRigidBody(AActor* Body, float Friction, float Restitution, int& ID,float mass);

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|Objects")
			void UpdatePlayertransform(AActor* player, int ID);

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|Objects")
			void AddImpulse(int ID, FVector Impulse, FVector Location);

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|Objects")
			void AddForce(int ID, FVector Impulse, FVector Location);

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|Objects")
			void StepPhysics(float DeltaSeconds, float FixedTimeStep = 0.0078125f);

		void NotifyCollisions();

		void ServerBroadcastSnapshot();

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|Objects")
			void SetPhysicsState(int ID, FTransform transforms, FVector Velocity, FVector AngularVelocity,FVector& Force);

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|Objects")
			void GetPhysicsState(int ID, FTransform& transforms, FVector& Velocity, FVector& AngularVelocity, FVector& Force);

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|Objects")
			void ResetSim();

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Physics|Objects")
			bool DebugEnabled=true;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Physics|Objects")
			FVector Gravity=FVector(0, 0, -9.8);

		// Input the fixed frame rate to calculate physics
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Physics|Objects")
			float PhysicsRefreshRate = 128.0f;

		// This is independent of the frame rate in UE
		UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Bullet Physics|Objects")
			float PhysicsDeltaTime;

		// Maximum number of Bullet simulation ticks per frame
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Physics|Objects")
			int MaxTicksPerFrame = 64;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Physics|Objects")
			int SubSteps=1;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Physics|Objects")
			float RandVar;

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|Skeletal Mesh")
			float GetGravity(){ return Gravity.Z; };

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|RayCast")
			void RayTestSingle(FVector start, FVector end, int CheckObjectID, const FRayTestSingleCallback HitCallback);

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|Settings")
			void EnableDebugDrawer();

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|Time")
		int32 GetTickCount() const { return TickCount; }

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|Time")
		float GetTimeSeconds() const { return TickCount * PhysicsDeltaTime; }

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|Time")
		float GetInterpolatedTimeSeconds() const { return GetTimeSeconds() + BtWorld->getLocalTime() * SavedTimeScale; }

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|Networking")
		bool IsRollback() const { return bIsRollback; }

		void StartRollback(int32 RollbackTick)
		{
			bIsRollback = true;
			RollbackStartTick = RollbackTick;
		}

		void SkipToTick(int32 SkipTick)
		{
			bIsSkipping = true;
			TickToSkipTo = SkipTick;
		}

		/// Additional ticks to advance the simulation by, bypassing max ticks per frame
		int GetExtraSimulationTicks()
		{
			return BtWorld->getExtraSimulationTicks();
		}

		void AddExtraSimulationTicks(int Ticks)
		{
			BtWorld->addExtraSimulationTicks(Ticks);
		}

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|Time")
		float GetTimeScale() const { return BtWorld->getTimeScale(); }

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|Time")
		void SetTimeScale(float NewTimeScale) { BtWorld->setTimeScale(NewTimeScale); }

		UFUNCTION(BlueprintCallable, Category = "Bullet Physics|Time")
		float GetRealDeltaTime() const { return PhysicsDeltaTime / GetTimeScale(); }

		void RayTestSingle(FVector Start, FVector End, int CheckObjectID, std::function<void(const FVector&, const FVector&, const bool&)> HitCallback);

		void RayTest(FVector Start, FVector End,std::function<void(const FVector&, const FVector&, const bool&)> HitCallback);

		btCollisionWorld::AllHitsRayResultCallback RayTestAll(FVector Start, FVector End);

		FClosestRayResultWithExclude RayTest(FVector Start, FVector End, const btCollisionObject* ExcludeObject);

		btCollisionWorld::ClosestRayResultCallback RayTest(FVector Start, FVector End);

		FClosestConvexResultWithExclude ConvexSweepTest(const btConvexShape* CastShape, FTransform Start, FTransform End, const btCollisionObject* ExcludeObject);

		virtual void Initialize(FSubsystemCollectionBase& Collection) override;

		virtual void OnWorldBeginPlay(UWorld& InWorld) override;

		virtual void Tick(float deltaTime) override;

		virtual TStatId GetStatId() const override;

private:
		// Bullet section
		// Global objects

		btCollisionConfiguration* BtCollisionConfig;
		btCollisionDispatcher* BtCollisionDispatcher;
		btBroadphaseInterface* BtBroadphase;
		btConstraintSolver* BtConstraintSolver;
		CustomBulletWorld* BtWorld;
		BulletHelpers* BulletHelpers;
		BulletDebugDraw* btdebugdraw;
		btStaticPlaneShape* plane;
		// Custom debug interface
		btIDebugDraw* BtDebugDraw;
		// Dynamic bodies
		// Static colliders
		TArray<btCollisionObject*> BtStaticObjects;
		btCollisionObject* procbody;
		// Re-usable collision shapes
		TArray<btBoxShape*> BtBoxCollisionShapes;
		TArray<btSphereShape*> BtSphereCollisionShapes;
		TArray<btCapsuleShape*> BtCapsuleCollisionShapes;
		TArray<btCylinderShape*> BtCylinderCollisionShapes;
		TArray<btCylinderShape*> BtCylinderCollisionShapesX;
		TArray<btCylinderShape*> BtCylinderCollisionShapesZ;
		btSequentialImpulseConstraintSolver* mt;
		// Structure to hold re-usable ConvexHull shapes based on origin BodySetup / subindex / scale
		struct ConvexHullShapeHolder
		{
			UBodySetup* BodySetup;
			int HullIndex;
			FVector Scale;
			btConvexHullShape* Shape;
		};
		TArray<ConvexHullShapeHolder> BtConvexHullCollisionShapes;
		// These shapes are for *potentially* compound rigid body shapes
		struct CachedDynamicShapeData
		{
			FName ClassName; // class name for cache
			btCollisionShape* Shape;
			bool bIsCompound; // if true, this is a compound shape and so must be deleted
			btScalar Mass;
			btVector3 Inertia; // because we like to precalc this
		};
		TArray<CachedDynamicShapeData> CachedDynamicShapes;

		TArray<btRigidBody*> BtRigidBodies;

		int32 TickCount;

		/// We save timescale so that changes to timescale don't affect GetInterpolatedTimeSeconds until the next tick
		float SavedTimeScale = 1.f;

		int32 RollbackStartTick;

		int32 TickToSkipTo;

		bool bIsRollback;

		bool bIsSkipping;

	public:
		CustomBulletWorld* GetBulletWorld() const { return BtWorld; }

		void RayTestSingle(FVector Start, FVector End, int CheckObjectID, void (*HitCallback)(const FVector&, const FVector&));

		btBoxShape* GetBoxCollisionShape(const FVector& Dimensions);

		btSphereShape* GetSphereCollisionShape(float Radius);

		btCapsuleShape* GetCapsuleCollisionShape(float Radius, float Height);

		btCylinderShape* GetCylinderCollisionShape(float Radius, float Height);

		btCylinderShape* GetCylinderCollisionShapeX(float Radius, float Height);

		btCylinderShape* GetCylinderCollisionShapeZ(float Radius, float Height);

		btBvhTriangleMeshShape* GetTriangleMeshShape(TArray<FVector> a, TArray<FVector> b, TArray<FVector> c, TArray<FVector> d);

		btConvexHullShape* GetConvexHullCollisionShape(UBodySetup* BodySetup, int ConvexIndex, const FVector& Scale);

		btRigidBody* AddRigidBody(AActor* Body, float Friction, float Restitution, float Mass);

		btRigidBody* AddRigidBody(AActor* Actor, const UBulletSubsystem::CachedDynamicShapeData& ShapeData, float Friction, float Restitution);

		btRigidBody* AddRigidBody(AActor* Actor, btCollisionShape* CollisionShape, btVector3 Inertia, float Mass, float Friction, float Restitution);

		btRigidBody* AddRigidBody(USkeletalMeshComponent* skel, const FTransform& localTransform, btCollisionShape* collisionShape, float Mass, float Friction, float Restitution);

		btCollisionObject* GetStaticObject(int ID);

	private:
		typedef const std::function<void(btCollisionShape* /*SingleShape*/, const FTransform& /*RelativeXform*/)>& PhysicsGeometryCallback;

		void SetupStaticGeometryPhysics(TArray<AActor*> Actors, float Friction, float Restitution);

		void ExtractPhysicsGeometry(AActor* Actor, PhysicsGeometryCallback CB);

		btCollisionObject* AddStaticCollision(btCollisionShape* Shape, const FTransform& Transform, float Friction, float Restitution, AActor* Actor);

		void ExtractPhysicsGeometry(ULandscapeComponent* LandscapeComponent, const FTransform& InvActorXform, PhysicsGeometryCallback CB);

		void ExtractPhysicsGeometry(UStaticMeshComponent* SMC, const FTransform& InvActorXform, PhysicsGeometryCallback CB);

		void ExtractPhysicsGeometry(USkeletalMeshComponent* SMC, const FTransform& InvActorXform, PhysicsGeometryCallback CB);

		void ExtractPhysicsGeometry(USplineMeshComponent* SMC, const FTransform& InvActorXform, PhysicsGeometryCallback CB);

		void ExtractPhysicsGeometry(UShapeComponent* Sc, const FTransform& InvActorXform, PhysicsGeometryCallback CB);

		void ExtractPhysicsGeometry(const FTransform& XformSoFar, UBodySetup* BodySetup, PhysicsGeometryCallback CB);

		const UBulletSubsystem::CachedDynamicShapeData& GetCachedDynamicShapeData(AActor* Actor, float Mass);
};
