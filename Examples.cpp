/**
 * Audio Sandbox - Basic Example
 * Demonstrates how to use the procedural audio/physics system
 */

#include "SandboxManager.h"
#include "Audio/AudioSynthesizer.h"
#include "Physics/PhysicsCore.h"
#include <iostream>
#include <vector>

/**
 * Example 1: Basic Percussion Impact
 * Drop a sphere and listen to the impact sound
 */
void Example_PercussionImpact()
{
	std::cout << "=== Example 1: Percussion Impact ===" << std::endl;

	FPercussionSandbox Sandbox(48000.0f);

	// Drop object from 5 meters
	Sandbox.DropObject(5.0f, 0.5f, 0.8f);

	// Simulate for 2 seconds
	TArray<float> AudioBuffer;
	for (int i = 0; i < 96000; i += 2048) // 96000 samples = 2 sec at 48kHz
	{
		Sandbox.Update(0.0426667f, AudioBuffer); // ~43ms per frame
		// Write AudioBuffer to file or audio device
	}

	std::cout << "Generated percussion sound: " << AudioBuffer.Num() << " samples" << std::endl;
}

/**
 * Example 2: Resonant Surface
 * Multiple resonators vibrating and producing harmonic content
 */
void Example_ResonantSurface()
{
	std::cout << "=== Example 2: Resonant Surface ===" << std::endl;

	FResonantSurfaceSandbox Sandbox(48000.0f);

	// Create a set of resonators with different frequencies
	Sandbox.AddResonator(220.0f, 8.0f, 1.0f); // A3
	Sandbox.AddResonator(330.0f, 8.0f, 1.0f); // A4
	Sandbox.AddResonator(440.0f, 8.0f, 1.0f); // A5

	// Set low damping for sustained resonance
	Sandbox.SetSurfaceDamping(0.05f);

	// Excite resonators
	Sandbox.ExciteResonator(0, 0.8f);
	Sandbox.ExciteResonator(1, 0.7f);
	Sandbox.ExciteResonator(2, 0.6f);

	// Generate audio
	TArray<float> AudioBuffer;
	for (int i = 0; i < 192000; i += 2048) // 4 seconds
	{
		Sandbox.Update(0.0426667f, AudioBuffer);
	}

	std::cout << "Generated resonant surface: " << AudioBuffer.Num() << " samples" << std::endl;
}

/**
 * Example 3: Physics Simulation with Procedural Generation
 * Complex system with physics objects and procedurally evolved audio
 */
void Example_ComplexPhysicsAudio()
{
	std::cout << "=== Example 3: Complex Physics with Procedural Audio ===" << std::endl;

	FSandboxManager Sandbox(48000.0f, 2048);

	// Create multiple objects
	for (int i = 0; i < 3; ++i)
	{
		auto Sphere = MakeShared<FPhysicsSphere>(0.3f + i * 0.2f, 1.0f + i * 0.5f);
		Sphere->SetPosition(FVector3(-2.0f + i * 2.0f, 5.0f - i, 0));
		Sphere->ApplyImpulse(FVector3(5.0f, -3.0f, 0));
		Sandbox.AddPhysicsObject(Sphere);
	}

	// Configure procedural generation
	auto FreqGen = MakeUnique<FPerlinNoiseGenerator>();
	FreqGen->SetOctaves(4);
	FreqGen->SetPersistence(0.6f);
	Sandbox.GetProceduralController()->SetFrequencyGenerator(MoveTemp(FreqGen));
	Sandbox.GetProceduralController()->SetFrequencyRange(200.0f, 1000.0f);

	// Enable all features
	Sandbox.EnableProceduralGeneration(true);
	Sandbox.EnablePhysicsAudio(true);
	Sandbox.EnableResonance(true);

	// Simulate
	TArray<float> AudioBuffer;
	for (int frame = 0; frame < 240; ++frame) // 10 seconds at 24 FPS
	{
		Sandbox.Update(1.0f / 24.0f, AudioBuffer);
	}

	// Get stats
	FSandboxManager::FSandboxStats Stats;
	Sandbox.GetStats(Stats);
	std::cout << "Active objects: " << Stats.ActivePhysicsObjects << std::endl;
	std::cout << "Queued impacts: " << Stats.QueuedImpacts << std::endl;
	std::cout << "Generated samples: " << AudioBuffer.Num() << std::endl;
}

/**
 * Example 4: Custom Procedural Generation
 * Using chaotic maps for complex parameter evolution
 */
void Example_ChaoticGeneration()
{
	std::cout << "=== Example 4: Chaotic Parameter Generation ===" << std::endl;

	FSandboxManager Sandbox(48000.0f);

	// Use chaotic map for frequency evolution
	auto ChaoticGen = MakeUnique<FChaoticGenerator>(FChaoticGenerator::EChaosType::Henon);
	ChaoticGen->SetChaosParameter(1.4f);

	Sandbox.GetProceduralController()->SetFrequencyGenerator(MoveTemp(ChaoticGen));
	Sandbox.GetProceduralController()->SetFrequencyRange(100.0f, 3000.0f);

	// Generate 5 sounds
	for (int sound = 0; sound < 5; ++sound)
	{
		float Frequency, Amplitude, Richness, Duration;
		Sandbox.GetProceduralController()->GenerateParameters(
			Frequency, Amplitude, Richness, Duration);

		std::cout << "Sound " << sound << ": "
			<< "Freq=" << Frequency << "Hz, "
			<< "Amp=" << Amplitude << ", "
			<< "Rich=" << Richness << ", "
			<< "Dur=" << Duration << "s" << std::endl;
	}
}

/**
 * Example 5: Direct Synthesis Control
 * Low-level API for fine-grained audio synthesis
 */
void Example_DirectSynthesis()
{
	std::cout << "=== Example 5: Direct Synthesis Control ===" << std::endl;

	const float SampleRate = 48000.0f;
	const int32 NumSamples = 48000; // 1 second

	// Create oscillator
	FOscillator Osc(SampleRate);
	Osc.SetFrequency(440.0f); // A4
	Osc.SetAmplitude(0.3f);
	Osc.SetWaveform(FOscillator::EWaveform::Sine);

	// Create envelope
	FEnvelopeGenerator::FEnvelopeParams EnvParams;
	EnvParams.AttackTime = 0.1f;
	EnvParams.DecayTime = 0.2f;
	EnvParams.SustainLevel = 0.6f;
	EnvParams.ReleaseTime = 0.3f;

	FEnvelopeGenerator Envelope(SampleRate);
	Envelope.SetParameters(EnvParams);
	Envelope.NoteOn();

	// Generate audio with envelope modulation
	TArray<float> AudioBuffer;
	Osc.GenerateSamples(AudioBuffer, NumSamples);

	// Apply envelope
	for (int32 i = 0; i < NumSamples; ++i)
	{
		float EnvValue = Envelope.GetNextValue();
		AudioBuffer[i * 2] *= EnvValue;
		AudioBuffer[i * 2 + 1] *= EnvValue;
	}

	std::cout << "Generated enveloped sine wave: " << AudioBuffer.Num() << " samples" << std::endl;
}

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * Helper to save audio buffer to WAV file (simplified)
 */
void SaveAudioToFile(const TArray<float>& AudioBuffer, const char* Filename)
{
	// TODO: Implement WAV file writing
	std::cout << "Would save " << AudioBuffer.Num() << " samples to " << Filename << std::endl;
}

/**
 * Helper to analyze audio buffer for peak level
 */
float AnalyzePeakLevel(const TArray<float>& AudioBuffer)
{
	float PeakLevel = 0.0f;
	for (float Sample : AudioBuffer)
	{
		float AbsSample = FMath::Abs(Sample);
		if (AbsSample > PeakLevel)
		{
			PeakLevel = AbsSample;
		}
	}
	return PeakLevel;
}

// ============================================================================
// Main Entry Point
// ============================================================================

int main()
{
	std::cout << "Audio Sandbox - Procedural Audio/Physics Integration Examples" << std::endl;
	std::cout << "=============================================================" << std::endl;
	std::cout << std::endl;

	try
	{
		// Run examples
		Example_PercussionImpact();
		std::cout << std::endl;

		Example_ResonantSurface();
		std::cout << std::endl;

		Example_ComplexPhysicsAudio();
		std::cout << std::endl;

		Example_ChaoticGeneration();
		std::cout << std::endl;

		Example_DirectSynthesis();
		std::cout << std::endl;

		std::cout << "All examples completed successfully!" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}

// ============================================================================
// Notes for Integration with Unreal Engine
// ============================================================================
/*
To use this with Unreal Engine:

1. Create a plugin module:
   - Plugins/AudioSandbox/Source/AudioSandbox/

2. Add to .uplugin file:
   {
     "FileVersion": 3,
     "Version": 1,
     "VersionName": "1.0.0",
     "FriendlyName": "Audio Sandbox - Procedural Audio Physics",
     "Description": "Real-time procedural audio generation driven by physics simulation",
     "Category": "Audio",
     "CreatedBy": "Your Name",
     ...
   }

3. Create UObject wrappers:
   UCLASS()
   class AUDIOSANDBOX_API UAudioSandboxComponent : public UActorComponent
   {
       GENERATED_BODY()
   private:
       FSandboxManager SandboxManager;
       ...
   };

4. Register with audio subsystem for hardware output

5. Use in Blueprint or C++ code:
   AActor->AddComponentByClass(UAudioSandboxComponent::StaticClass(), ...);
*/
