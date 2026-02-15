#pragma once

#include "CoreMinimal.h"
#include "Audio/AudioSynthesizer.h"
#include "Physics/PhysicsCore.h"

/**
 * Maps physics impact events to audio synthesis parameters
 * Core integration point between physical simulation and procedural audio
 */
class FAudioPhysicsMapper
{
public:
	FAudioPhysicsMapper();

	/**
	 * Convert impact event to audio synthesis parameters
	 * @param ImpactEvent Physics impact data
	 * @param OutFrequency Generated frequency (Hz)
	 * @param OutAmplitude Generated amplitude (0-1)
	 * @param OutDuration Generated duration (seconds)
	 */
	void MapImpactToAudio(
		const FImpactEvent& ImpactEvent,
		float& OutFrequency,
		float& OutAmplitude,
		float& OutDuration);

	/**
	 * Generate impact frequency based on material properties and impact force
	 * Materials with higher hardness produce higher frequencies
	 * @param MaterialHardness 0-1, higher = harder material
	 * @param ImpactForce 0-1 normalized
	 * @return Frequency in Hz
	 */
	float GenerateImpactFrequency(float MaterialHardness, float ImpactForce);

	/**
	 * Set frequency range for impact sounds
	 */
	void SetFrequencyRange(float MinHz, float MaxHz);

private:
	float MinFrequency;   // Hz
	float MaxFrequency;   // Hz
	float FrequencyScale; // Multiplier for frequency generation
};

/**
 * Manages impact events from physics and routes to audio
 */
class FImpactEventQueue
{
public:
	FImpactEventQueue(int32 MaxQueueSize = 256);

	void QueueImpact(const FImpactEvent& Impact);
	bool DequeueImpact(FImpactEvent& OutImpact);
	int32 GetQueueSize() const { return ImpactQueue.Num(); }
	bool HasEvents() const { return ImpactQueue.Num() > 0; }

private:
	TArray<FImpactEvent> ImpactQueue;
	int32 MaxSize;
};

/**
 * Synthesizer for impact/collision sounds
 * Generates transient audio from physical collisions
 */
class FImpactSynthesizer : public FBaseSynthesizer
{
public:
	FImpactSynthesizer(float InSampleRate = 48000.0f);

	virtual void GenerateSamples(TArray<float>& OutBuffer, int32 NumSamples) override;
	virtual void SetParameter(const FName& ParamName, float Value) override;

	/**
	 * Trigger an impact sound
	 * @param Frequency Fundamental frequency (Hz)
	 * @param Amplitude Peak amplitude (0-1)
	 * @param Duration Sound duration (seconds)
	 */
	void TriggerImpact(float Frequency, float Amplitude, float Duration);

	bool IsPlaying() const { return bIsPlaying; }

private:
	FEnvelopeGenerator Envelope;
	FOscillator Oscillator;
	float ImpactDuration;
	float RemainingDuration;
	bool bIsPlaying;

	// Frequency decay over time (pitch drop for impact)
	float FrequencyDecay;
	float InitialFrequency;
};

/**
 * Continuous/resonance synthesis from sustained physics interactions
 * For vibrations, friction, etc.
 */
class FResonanceSynthesizer : public FBaseSynthesizer
{
public:
	FResonanceSynthesizer(float InSampleRate = 48000.0f);

	virtual void GenerateSamples(TArray<float>& OutBuffer, int32 NumSamples) override;
	virtual void SetParameter(const FName& ParamName, float Value) override;

	/**
	 * Set resonance properties
	 * @param Frequency Resonant frequency (Hz)
	 * @param Quality Q factor (higher = sharper resonance)
	 * @param Damping Energy loss (0-1)
	 */
	void SetResonance(float Frequency, float Quality, float Damping);

	/**
	 * Excite the resonance with input energy
	 * @param Energy Input amount (0-1)
	 */
	void ExciteResonance(float Energy);

private:
	float Quality;
	float ResonanceDamping;
	float AccumulatedEnergy;

	// Simple resonator state
	float FilterState1;
	float FilterState2;
};

/**
 * Master audio/physics integration controller
 * Orchestrates all audio generation from physics simulation
 */
class FAudioPhysicsSandbox
{
public:
	FAudioPhysicsSandbox(float InSampleRate = 48000.0f);

	/**
	 * Update sandbox with physics and generate audio
	 * @param PhysicsWorld The physics simulation to read from
	 * @param DeltaTime Time step
	 * @param OutAudioBuffer Generated audio samples
	 * @param NumSamples Samples to generate
	 */
	void Update(FPhysicsWorld* PhysicsWorld, float DeltaTime, 
		TArray<float>& OutAudioBuffer, int32 NumSamples);

	// Add/remove objects from audio monitoring
	void RegisterPhysicsObject(TSharedPtr<FPhysicsObject> Object);
	void UnregisterPhysicsObject(TSharedPtr<FPhysicsObject> Object);

	// Access components
	FAudioMixer* GetMixer() { return &AudioMixer; }
	FAudioPhysicsMapper* GetMapper() { return &PhysicsMapper; }
	FImpactEventQueue* GetImpactQueue() { return &ImpactQueue; }

	void SetMasterVolume(float Volume) { MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f); }
	float GetMasterVolume() const { return MasterVolume; }

private:
	FAudioMixer AudioMixer;
	FAudioPhysicsMapper PhysicsMapper;
	FImpactEventQueue ImpactQueue;

	TSharedPtr<FImpactSynthesizer> ImpactSynth;
	TSharedPtr<FResonanceSynthesizer> ResonanceSynth;

	TArray<TSharedPtr<FPhysicsObject>> MonitoredObjects;
	float MasterVolume;
	float SampleRate;

	void ProcessPhysicsImpacts(FPhysicsWorld* PhysicsWorld);
};
