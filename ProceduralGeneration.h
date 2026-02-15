#pragma once

#include "CoreMinimal.h"
#include "Containers/List.h"

/**
 * Base interface for procedural parameter generation
 * Uses various algorithms to create evolving audio parameters
 */
class FProceduralGenerator
{
public:
	virtual ~FProceduralGenerator() = default;

	/**
	 * Generate next parameter value(s)
	 * @return Value in range [0, 1]
	 */
	virtual float GetNextValue() = 0;

	virtual void Reset() = 0;
	virtual void SetSeed(uint32 InSeed) = 0;
};

/**
 * Perlin noise-based parameter generation
 * Creates smooth, organic parameter evolution
 */
class FPerlinNoiseGenerator : public FProceduralGenerator
{
public:
	FPerlinNoiseGenerator(uint32 InSeed = 12345);

	virtual float GetNextValue() override;
	virtual void Reset() override;
	virtual void SetSeed(uint32 InSeed) override;

	void SetOctaves(int32 NumOctaves) { Octaves = FMath::Max(1, NumOctaves); }
	void SetPersistence(float InPersistence) { Persistence = FMath::Clamp(InPersistence, 0.0f, 1.0f); }
	void SetScale(float InScale) { Scale = FMath::Max(InScale, 0.1f); }

private:
	uint32 Seed;
	float CurrentTime;
	int32 Octaves;
	float Persistence;
	float Scale;

	float PerlinNoise1D(float x);
	float Interpolate(float a, float b, float t);
};

/**
 * Chaotic/deterministic synthesis parameter generator
 * Creates rich, evolving parameter sequences
 */
class FChaoticGenerator : public FProceduralGenerator
{
public:
	enum class EChaosType : uint8
	{
		Logistic,    // Logistic map chaos
		Henon,       // Henon map (2D)
		Lorenz       // Lorenz attractor characteristics
	};

	FChaoticGenerator(EChaosType InType = EChaosType::Logistic);

	virtual float GetNextValue() override;
	virtual void Reset() override;
	virtual void SetSeed(uint32 InSeed) override;

	void SetChaosParameter(float Value) { ChaosParam = FMath::Clamp(Value, 0.0f, 4.0f); }
	EChaosType GetType() const { return ChaoticType; }

private:
	EChaosType ChaoticType;
	float ChaosParam;
	float X;      // Current state
	float Y;      // For 2D maps

	float LogisticMap();
	float HenonMap();
};

/**
 * Spectral analysis-inspired parameter generator
 * Creates harmonic variations based on spectral patterns
 */
class FSpectralGenerator : public FProceduralGenerator
{
public:
	FSpectralGenerator();

	virtual float GetNextValue() override;
	virtual void Reset() override;
	virtual void SetSeed(uint32 InSeed) override;

	/**
	 * Add a harmonic component
	 * @param Frequency Relative frequency (cycles per sample batch)
	 * @param Amplitude Contribution weight
	 */
	void AddHarmonic(float Frequency, float Amplitude);
	void ClearHarmonics();

	int32 GetHarmonicCount() const { return Harmonics.Num(); }

private:
	struct FHarmonic
	{
		float Frequency;
		float Amplitude;
		float Phase;
	};

	TArray<FHarmonic> Harmonics;
	uint32 SampleCount;
	float CurrentTime;
};

/**
 * Markov chain-based parameter generator
 * Creates sequential patterns with probabilistic state transitions
 */
class FMarkovGenerator : public FProceduralGenerator
{
public:
	FMarkovGenerator(int32 Order = 1);

	virtual float GetNextValue() override;
	virtual void Reset() override;
	virtual void SetSeed(uint32 InSeed) override;

	/**
	 * Add state transition
	 * @param FromState Previous value (quantized)
	 * @param ToState Next value (quantized)
	 * @param Probability Likelihood of transition (0-1)
	 */
	void AddTransition(float FromState, float ToState, float Probability);

private:
	struct FStateTransition
	{
		float FromState;
		float ToState;
		float Probability;
	};

	int32 Order;
	float CurrentState;
	uint32 RandomSeed;
	TArray<FStateTransition> Transitions;

	float SelectNextState();
};

/**
 * Procedural parameter controller
 * Manages multiple generators for complex audio parameter evolution
 */
class FProceduralController
{
public:
	FProceduralController();

	/**
	 * Generate next set of parameters
	 * @param OutFrequency Fundamental frequency (Hz)
	 * @param OutAmplitude Amplitude envelope (0-1)
	 * @param OutSpectralRichness Harmonic content (0-1)
	 * @param OutDuration Duration estimate (seconds)
	 */
	void GenerateParameters(
		float& OutFrequency,
		float& OutAmplitude,
		float& OutSpectralRichness,
		float& OutDuration);

	// Register generators for different parameters
	void SetFrequencyGenerator(TUniquePtr<FProceduralGenerator> Gen);
	void SetAmplitudeGenerator(TUniquePtr<FProceduralGenerator> Gen);
	void SetSpectralGenerator(TUniquePtr<FProceduralGenerator> Gen);
	void SetDurationGenerator(TUniquePtr<FProceduralGenerator> Gen);

	void SetFrequencyRange(float MinHz, float MaxHz);
	void SetAmplitudeRange(float MinAmp, float MaxAmp);
	void SetDurationRange(float MinSec, float MaxSec);

	void SetSeed(uint32 NewSeed);
	void Reset();

private:
	TUniquePtr<FProceduralGenerator> FrequencyGen;
	TUniquePtr<FProceduralGenerator> AmplitudeGen;
	TUniquePtr<FProceduralGenerator> SpectralGen;
	TUniquePtr<FProceduralGenerator> DurationGen;

	float FreqMin, FreqMax;
	float AmpMin, AmpMax;
	float DurMin, DurMax;

	float MapRange(float Value, float OutMin, float OutMax);
};

/**
 * Adaptive procedural generator that evolves based on feedback
 * Creates feedback loops where audio characteristics influence future generation
 */
class FAdaptiveProceduralSystem
{
public:
	FAdaptiveProceduralSystem();

	/**
	 * Update system with audio metrics and generate next parameters
	 * @param AudioMetrics Current audio characteristics (spectrum, energy, etc.)
	 * @param OutParameters Generated procedural parameters
	 */
	void Update(const TArray<float>& AudioMetrics, TArray<float>& OutParameters);

	void SetAdaptationRate(float Rate) { AdaptationRate = FMath::Clamp(Rate, 0.0f, 1.0f); }
	float GetAdaptationRate() const { return AdaptationRate; }

private:
	FProceduralController ProcController;
	TArray<float> PreviousMetrics;
	TArray<float> MetricHistory;
	float AdaptationRate;

	void AnalyzeMetrics(const TArray<float>& Metrics, TArray<float>& OutAnalysis);
};
