#pragma once

#include "CoreMinimal.h"
#include "Audio/AudioSynthesizer.h"
#include "Physics/PhysicsCore.h"
#include "Integration/AudioPhysicsIntegration.h"
#include "Procedural/ProceduralGeneration.h"

/**
 * Main Audio/Physics Sandbox
 * Complete integration system for procedurally-driven audio from physics simulation
 * 
 * Usage:
 *   1. Create FSandboxManager instance
 *   2. Create physics objects and add to sandbox
 *   3. Call Update() each frame with delta time
 *   4. Retrieve audio buffer for output to hardware or file
 */
class FSandboxManager
{
public:
	FSandboxManager(
		float InSampleRate = 48000.0f,
		int32 BufferSize = 2048);

	~FSandboxManager();

	/**
	 * Main update loop - simulate physics, generate audio
	 * @param DeltaTime Time step in seconds
	 * @param OutAudioBuffer Generated stereo audio (interleaved)
	 * @return Number of samples generated
	 */
	int32 Update(float DeltaTime, TArray<float>& OutAudioBuffer);

	// Physics world management
	FPhysicsWorld* GetPhysicsWorld() { return &PhysicsWorld; }
	void AddPhysicsObject(TSharedPtr<FPhysicsObject> Object);
	void RemovePhysicsObject(TSharedPtr<FPhysicsObject> Object);

	// Audio/Physics integration
	FAudioPhysicsSandbox* GetAudioPhysics() { return &AudioPhysicsIntegration; }
	FProceduralController* GetProceduralController() { return &ProceduralController; }

	// Configuration
	void SetMasterVolume(float Volume);
	void EnableProceduralGeneration(bool bEnable) { bUseProceduralGeneration = bEnable; }
	void EnablePhysicsAudio(bool bEnable) { bUsePhysicsAudio = bEnable; }
	void EnableResonance(bool bEnable) { bUseResonanceSynthesis = bEnable; }

	// Runtime parameters
	void SetSimulationSpeed(float Speed) { SimulationSpeed = FMath::Max(Speed, 0.1f); }
	float GetSimulationSpeed() const { return SimulationSpeed; }

	// Statistics
	struct FSandboxStats
	{
		int32 ActivePhysicsObjects;
		int32 QueuedImpacts;
		float AverageAudioLevel;
		float SimulationFrameTime;
	};

	bool GetStats(FSandboxStats& OutStats) const;

private:
	FPhysicsWorld PhysicsWorld;
	FAudioPhysicsSandbox AudioPhysicsIntegration;
	FProceduralController ProceduralController;

	float SampleRate;
	int32 BufferSize;
	float SimulationSpeed;

	// Feature flags
	bool bUseProceduralGeneration;
	bool bUsePhysicsAudio;
	bool bUseResonanceSynthesis;
	bool bInitialized;

	// Performance tracking
	float LastFrameTime;
	TArray<float> FrameTimeHistory;

	void Initialize();
	void ProcessProceduralAudio(TArray<float>& OutBuffer);
	void ProcessPhysicsAudio(TArray<float>& OutBuffer);
	void MixAudio(TArray<float>& OutBuffer, const TArray<float>& InBuffer, float Volume);
};

/**
 * Example: Percussion impact sandbox
 * Demonstrates basic audio-driven physics impacts
 */
class FPercussionSandbox : public FSandboxManager
{
public:
	FPercussionSandbox(float InSampleRate = 48000.0f);

	// Drop object from height, generate impact sound
	void DropObject(float Height, float Radius, float Material);

	// Strike object with given velocity
	void StrikeObject(float Velocity, float ImpactPoint);

private:
	float DefaultMass;
	float DefaultRadius;
};

/**
 * Example: Resonant surface sandbox
 * Objects vibrating and resonating, generating timbral audio
 */
class FResonantSurfaceSandbox : public FSandboxManager
{
public:
	FResonantSurfaceSandbox(float InSampleRate = 48000.0f);

	void AddResonator(float NaturalFrequency, float Quality, float Mass);
	void ExciteResonator(int32 Index, float Energy);
	void SetSurfaceDamping(float Damping);

private:
	TArray<TSharedPtr<FPhysicsSphere>> Resonators;
};

/**
 * Example: Granular synthesis from physics
 * Each physics event triggers a grain of sound
 */
class FGranularPhysicsSandbox : public FSandboxManager
{
public:
	FGranularPhysicsSandbox(float InSampleRate = 48000.0f);

	/**
	 * Configure grain synthesis
	 * @param GrainDuration Duration of each grain (seconds)
	 * @param GrainOverlap Number of overlapping grains
	 */
	void ConfigureGrains(float GrainDuration, int32 GrainOverlap);

	void SetGrainWaveform(FOscillator::EWaveform Form);

private:
	float GrainDuration;
	int32 GrainOverlap;
	TArray<TSharedPtr<FImpactSynthesizer>> GrainVoices;
};
