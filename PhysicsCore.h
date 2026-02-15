#pragma once

#include "CoreMinimal.h"
#include "Containers/List.h"

/**
 * 3D vector structure for physics calculations
 */
struct FVector3
{
	float X, Y, Z;

	FVector3(float InX = 0.0f, float InY = 0.0f, float InZ = 0.0f)
		: X(InX), Y(InY), Z(InZ) {}

	FVector3 operator+(const FVector3& Other) const
	{
		return FVector3(X + Other.X, Y + Other.Y, Z + Other.Z);
	}

	FVector3 operator-(const FVector3& Other) const
	{
		return FVector3(X - Other.X, Y - Other.Y, Z - Other.Z);
	}

	FVector3 operator*(float Scalar) const
	{
		return FVector3(X * Scalar, Y * Scalar, Z * Scalar);
	}

	float Dot(const FVector3& Other) const
	{
		return X * Other.X + Y * Other.Y + Z * Other.Z;
	}

	float Magnitude() const
	{
		return FMath::Sqrt(X * X + Y * Y + Z * Z);
	}

	FVector3 Normalize() const
	{
		float Mag = Magnitude();
		if (Mag > 1e-6f)
			return FVector3(X / Mag, Y / Mag, Z / Mag);
		return *this;
	}
};

/**
 * Base physics object with rigid body dynamics
 */
class FPhysicsObject
{
public:
	FPhysicsObject(float InMass = 1.0f);
	virtual ~FPhysicsObject() = default;

	/**
	 * Update physics simulation
	 * @param DeltaTime Time step in seconds
	 */
	virtual void Update(float DeltaTime);

	// Position and velocity
	void SetPosition(const FVector3& NewPos) { Position = NewPos; }
	void SetVelocity(const FVector3& NewVel) { Velocity = NewVel; }
	void ApplyForce(const FVector3& Force);
	void ApplyImpulse(const FVector3& Impulse);

	// Getters
	const FVector3& GetPosition() const { return Position; }
	const FVector3& GetVelocity() const { return Velocity; }
	const FVector3& GetAcceleration() const { return Acceleration; }
	float GetSpeed() const { return Velocity.Magnitude(); }
	float GetMass() const { return Mass; }

	// Damping for energy dissipation
	void SetDamping(float NewDamping) { Damping = FMath::Clamp(NewDamping, 0.0f, 1.0f); }
	float GetDamping() const { return Damping; }

protected:
	FVector3 Position;
	FVector3 Velocity;
	FVector3 Acceleration;
	FVector3 TotalForce;
	float Mass;
	float Damping;
};

/**
 * Rigid sphere for collision and impact detection
 */
class FPhysicsSphere : public FPhysicsObject
{
public:
	FPhysicsSphere(float InRadius = 1.0f, float InMass = 1.0f);

	float GetRadius() const { return Radius; }
	void SetRadius(float NewRadius) { Radius = FMath::Max(NewRadius, 0.1f); }

	/**
	 * Check collision with another sphere
	 * @param Other Another sphere
	 * @param OutImpactForce Force magnitude at impact
	 * @return true if collision detected
	 */
	bool CheckCollision(const FPhysicsSphere& Other, float& OutImpactForce) const;

	/**
	 * Calculate impact force based on velocity and mass
	 */
	float CalculateImpactForce() const;

private:
	float Radius;
};

/**
 * Physics world manager handling object interactions
 */
class FPhysicsWorld
{
public:
	FPhysicsWorld();

	void AddObject(TSharedPtr<FPhysicsObject> Object);
	void RemoveObject(TSharedPtr<FPhysicsObject> Object);
	void SimulateStep(float DeltaTime);

	// Physics parameters
	void SetGravity(const FVector3& NewGravity) { Gravity = NewGravity; }
	const FVector3& GetGravity() const { return Gravity; }

	// Access objects for impact detection
	const TArray<TSharedPtr<FPhysicsObject>>& GetObjects() const { return PhysicsObjects; }

private:
	TArray<TSharedPtr<FPhysicsObject>> PhysicsObjects;
	FVector3 Gravity;

	void DetectCollisions();
	void ApplyConstraints(float DeltaTime);
};

/**
 * Impact event data for audio generation
 */
struct FImpactEvent
{
	FVector3 Position;
	FVector3 ImpactNormal;
	float ImpactForce;     // Normalized 0-1
	float ImpactFrequency; // Suggested frequency for audio synthesis
	float Duration;        // How long to sustain the impact sound
	uint32 ObjectID;

	FImpactEvent()
		: ImpactForce(0.0f), ImpactFrequency(200.0f), Duration(0.5f), ObjectID(0) {}
};
