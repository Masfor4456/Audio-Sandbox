#pragma once

#include "CoreMinimal.h"
#include "Containers/List.h"

/**
 * Core audio synthesis interface for procedural audio generation
 * Supports real-time synthesis with parameter modulation
 */
class FBaseSynthesizer
{
public:
	FBaseSynthesizer(float InSampleRate = 48000.0f);
	virtual ~FBaseSynthesizer() = default;

	/**
	 * Generate audio samples
	 * @param OutBuffer Output audio buffer (interleaved stereo)
	 * @param NumSamples Number of samples to generate
	 */
	virtual void GenerateSamples(TArray<float>& OutBuffer, int32 NumSamples) = 0;

	/**
	 * Set synthesis parameters
	 * @param ParamName Parameter identifier
	 * @param Value Parameter value (normalized 0.0-1.0 unless otherwise specified)
	 */
	virtual void SetParameter(const FName& ParamName, float Value) = 0;

	// Getters
	float GetSampleRate() const { return SampleRate; }
	float GetCurrentPhase() const { return CurrentPhase; }

protected:
	float SampleRate;
	float CurrentPhase;
	float CurrentFrequency;
	float CurrentAmplitude;
};

/**
 * Simple wavetable oscillator - foundation for procedural synthesis
 */
class FOscillator : public FBaseSynthesizer
{
public:
	enum class EWaveform : uint8
	{
		Sine,
		Square,
		Sawtooth,
		Triangle,
		Noise
	};

	FOscillator(float InSampleRate = 48000.0f);

	virtual void GenerateSamples(TArray<float>& OutBuffer, int32 NumSamples) override;
	virtual void SetParameter(const FName& ParamName, float Value) override;

	void SetFrequency(float InFrequency);
	void SetAmplitude(float InAmplitude);
	void SetWaveform(EWaveform NewWaveform);

private:
	EWaveform CurrentWaveform;
	TArray<float> SineTable;

	float GenerateSample();
	void BuildWavetables();
};

/**
 * Envelope generator for amplitude and frequency modulation
 */
class FEnvelopeGenerator
{
public:
	struct FEnvelopeParams
	{
		float AttackTime;  // seconds
		float DecayTime;   // seconds
		float SustainLevel; // 0-1
		float ReleaseTime; // seconds
	};

	FEnvelopeGenerator(float InSampleRate = 48000.0f);

	void SetParameters(const FEnvelopeParams& NewParams);
	float GetNextValue();

	void NoteOn();
	void NoteOff();
	bool IsActive() const { return bIsActive; }

private:
	enum class EEnvelopeStage : uint8
	{
		Idle,
		Attack,
		Decay,
		Sustain,
		Release
	};

	float SampleRate;
	FEnvelopeParams Params;
	EEnvelopeStage CurrentStage;
	float EnvelopeValue;
	float SampleCount;
	bool bIsActive;
};

/**
 * Audio mixer for combining multiple synthesis sources
 */
class FAudioMixer
{
public:
	FAudioMixer();

	void AddSource(TSharedPtr<FBaseSynthesizer> Source);
	void RemoveSource(TSharedPtr<FBaseSynthesizer> Source);
	void MixAudio(TArray<float>& OutBuffer, int32 NumSamples);

private:
	TList<TSharedPtr<FBaseSynthesizer>> SynthSources;
	int32 SourceCount;
};
