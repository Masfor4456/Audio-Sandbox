#include "SandboxManager.h"
#include <chrono>

// ============================================================================
// FSandboxManager Implementation
// ============================================================================

FSandboxManager::FSandboxManager(float InSampleRate, int32 InBufferSize)
	: SampleRate(InSampleRate)
	, BufferSize(InBufferSize)
	, SimulationSpeed(1.0f)
	, bUseProceduralGeneration(true)
	, bUsePhysicsAudio(true)
	, bUseResonanceSynthesis(true)
	, bInitialized(false)
	, LastFrameTime(0.0f)
	, AudioPhysicsIntegration(InSampleRate)
	, ProceduralController()
{
	Initialize();
}

FSandboxManager::~FSandboxManager()
{
}

void FSandboxManager::Initialize()
{
	// Set up default physics world
	PhysicsWorld.SetGravity(FVector3(0, -9.81f, 0));

	// Configure procedural system
	ProceduralController.SetFrequencyRange(100.0f, 2000.0f);
	ProceduralController.SetAmplitudeRange(0.1f, 0.8f);
	ProceduralController.SetDurationRange(0.1f, 1.0f);

	bInitialized = true;
}

int32 FSandboxManager::Update(float DeltaTime, TArray<float>& OutAudioBuffer)
{
	if (!bInitialized)
	{
		OutAudioBuffer.SetNum(BufferSize * 2);
		for (int32 i = 0; i < OutAudioBuffer.Num(); ++i)
		{
			OutAudioBuffer[i] = 0.0f;
		}
		return BufferSize;
	}

	// Apply simulation speed
	float AdjustedDeltaTime = DeltaTime * SimulationSpeed;

	// Simulate physics
	PhysicsWorld.SimulateStep(AdjustedDeltaTime);

	// Initialize output buffer
	TArray<float> PhysicsAudioBuffer;
	TArray<float> ProceduralAudioBuffer;

	// Generate physics-driven audio
	if (bUsePhysicsAudio)
	{
		AudioPhysicsIntegration.Update(&PhysicsWorld, AdjustedDeltaTime, PhysicsAudioBuffer, BufferSize);
	}
	else
	{
		PhysicsAudioBuffer.SetNum(BufferSize * 2);
		for (int32 i = 0; i < PhysicsAudioBuffer.Num(); ++i)
		{
			PhysicsAudioBuffer[i] = 0.0f;
		}
	}

	// Generate procedural audio
	if (bUseProceduralGeneration)
	{
		ProcessProceduralAudio(ProceduralAudioBuffer);
	}
	else
	{
		ProceduralAudioBuffer.SetNum(BufferSize * 2);
		for (int32 i = 0; i < ProceduralAudioBuffer.Num(); ++i)
		{
			ProceduralAudioBuffer[i] = 0.0f;
		}
	}

	// Mix both audio streams
	OutAudioBuffer.SetNum(BufferSize * 2);
	for (int32 i = 0; i < OutAudioBuffer.Num(); ++i)
	{
		OutAudioBuffer[i] = (PhysicsAudioBuffer[i] * 0.6f + ProceduralAudioBuffer[i] * 0.4f) * 0.9f;
		// Soft clipping
		OutAudioBuffer[i] = FMath::Clamp(OutAudioBuffer[i], -1.0f, 1.0f);
	}

	// Track performance
	FrameTimeHistory.Add(LastFrameTime);
	if (FrameTimeHistory.Num() > 100)
	{
		FrameTimeHistory.RemoveAt(0);
	}

	return BufferSize;
}

void FSandboxManager::AddPhysicsObject(TSharedPtr<FPhysicsObject> Object)
{
	PhysicsWorld.AddObject(Object);
	AudioPhysicsIntegration.RegisterPhysicsObject(Object);
}

void FSandboxManager::RemovePhysicsObject(TSharedPtr<FPhysicsObject> Object)
{
	PhysicsWorld.RemoveObject(Object);
	AudioPhysicsIntegration.UnregisterPhysicsObject(Object);
}

void FSandboxManager::SetMasterVolume(float Volume)
{
	AudioPhysicsIntegration.SetMasterVolume(FMath::Clamp(Volume, 0.0f, 1.0f));
}

bool FSandboxManager::GetStats(FSandboxStats& OutStats) const
{
	OutStats.ActivePhysicsObjects = PhysicsWorld.GetObjects().Num();
	OutStats.QueuedImpacts = AudioPhysicsIntegration.GetImpactQueue()->GetQueueSize();
	OutStats.SimulationFrameTime = LastFrameTime;

	// Calculate average audio level (simplified)
	OutStats.AverageAudioLevel = 0.5f;

	return true;
}

void FSandboxManager::ProcessProceduralAudio(TArray<float>& OutBuffer)
{
	OutBuffer.SetNum(BufferSize * 2);

	// Generate procedural parameters
	float Frequency, Amplitude, Richness, Duration;
	ProceduralController.GenerateParameters(Frequency, Amplitude, Richness, Duration);

	// Create oscillator with generated parameters
	FOscillator Osc(SampleRate);
	Osc.SetFrequency(Frequency);
	Osc.SetAmplitude(Amplitude * 0.3f); // Reduce volume to avoid clipping

	// Set waveform based on spectral richness
	if (Richness < 0.33f)
	{
		Osc.SetWaveform(FOscillator::EWaveform::Sine);
	}
	else if (Richness < 0.66f)
	{
		Osc.SetWaveform(FOscillator::EWaveform::Triangle);
	}
	else
	{
		Osc.SetWaveform(FOscillator::EWaveform::Sawtooth);
	}

	Osc.GenerateSamples(OutBuffer, BufferSize);
}

void FSandboxManager::ProcessPhysicsAudio(TArray<float>& OutBuffer)
{
	// Already handled by AudioPhysicsIntegration
	OutBuffer.SetNum(BufferSize * 2);
}

void FSandboxManager::MixAudio(TArray<float>& OutBuffer, const TArray<float>& InBuffer, float Volume)
{
	for (int32 i = 0; i < OutBuffer.Num() && i < InBuffer.Num(); ++i)
	{
		OutBuffer[i] = (OutBuffer[i] + InBuffer[i] * Volume) * 0.5f;
	}
}

// ============================================================================
// FPercussionSandbox Implementation
// ============================================================================

FPercussionSandbox::FPercussionSandbox(float InSampleRate)
	: FSandboxManager(InSampleRate, 2048)
	, DefaultMass(2.0f)
	, DefaultRadius(0.5f)
{
}

void FPercussionSandbox::DropObject(float Height, float Radius, float Material)
{
	auto Sphere = MakeShared<FPhysicsSphere>(Radius, DefaultMass);
	Sphere->SetPosition(FVector3(0, Height, 0));
	AddPhysicsObject(Sphere);
}

void FPercussionSandbox::StrikeObject(float Velocity, float ImpactPoint)
{
	auto Sphere = MakeShared<FPhysicsSphere>(DefaultRadius, DefaultMass);
	Sphere->SetPosition(FVector3(0, 2, 0));
	Sphere->SetVelocity(FVector3(Velocity, 0, 0));
	AddPhysicsObject(Sphere);
}

// ============================================================================
// FResonantSurfaceSandbox Implementation
// ============================================================================

FResonantSurfaceSandbox::FResonantSurfaceSandbox(float InSampleRate)
	: FSandboxManager(InSampleRate, 2048)
{
}

void FResonantSurfaceSandbox::AddResonator(float NaturalFrequency, float Quality, float Mass)
{
	auto Sphere = MakeShared<FPhysicsSphere>(0.3f, Mass);
	Sphere->SetPosition(FVector3(Resonators.Num() * 1.0f, 2, 0));
	Resonators.Add(Sphere);
	AddPhysicsObject(Sphere);
}

void FResonantSurfaceSandbox::ExciteResonator(int32 Index, float Energy)
{
	if (Index >= 0 && Index < Resonators.Num())
	{
		Resonators[Index]->ApplyImpulse(FVector3(0, -Energy * 10.0f, 0));
	}
}

void FResonantSurfaceSandbox::SetSurfaceDamping(float Damping)
{
	for (auto& Resonator : Resonators)
	{
		if (Resonator.IsValid())
		{
			Resonator->SetDamping(FMath::Clamp(1.0f - Damping, 0.8f, 1.0f));
		}
	}
}

// ============================================================================
// FGranularPhysicsSandbox Implementation
// ============================================================================

FGranularPhysicsSandbox::FGranularPhysicsSandbox(float InSampleRate)
	: FSandboxManager(InSampleRate, 2048)
	, GrainDuration(0.05f)
	, GrainOverlap(4)
{
	// Create grain voices
	for (int32 i = 0; i < GrainOverlap; ++i)
	{
		auto GrainVoice = MakeShared<FImpactSynthesizer>(InSampleRate);
		GrainVoices.Add(GrainVoice);
	}
}

void FGranularPhysicsSandbox::ConfigureGrains(float InGrainDuration, int32 InGrainOverlap)
{
	GrainDuration = FMath::Max(InGrainDuration, 0.01f);
	GrainOverlap = FMath::Max(InGrainOverlap, 1);

	// Recreate grain voices
	GrainVoices.Clear();
	for (int32 i = 0; i < GrainOverlap; ++i)
	{
		auto GrainVoice = MakeShared<FImpactSynthesizer>(48000.0f);
		GrainVoices.Add(GrainVoice);
	}
}

void FGranularPhysicsSandbox::SetGrainWaveform(FOscillator::EWaveform Form)
{
	// Would be applied when generating grains
}
