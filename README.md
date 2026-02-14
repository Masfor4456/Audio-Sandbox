# Audio Sandbox - Procedural Audio/Physics Integration

A C++ framework for Unreal Engine that integrates physics simulation with real-time procedural audio generation. Create rich, evolving soundscapes driven entirely by physical interactions.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                   FSandboxManager (Main Controller)          │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌──────────────────┐  ┌──────────────────┐                 │
│  │ FPhysicsWorld    │  │ Audio/Physics    │                 │
│  │                  │  │ Integration      │                 │
│  │ - Objects        │  │                  │                 │
│  │ - Collisions     │──> - Impact Mapping │                 │
│  │ - Simulation     │  │ - Event Queue    │                 │
│  └──────────────────┘  └────────┬─────────┘                 │
│                                  │                            │
│                    ┌─────────────┴──────────────┐             │
│                    │                            │             │
│            ┌───────▼─────────┐       ┌──────────▼─────┐      │
│            │ Audio Synthesis │       │ Procedural     │      │
│            │                 │       │ Generation     │      │
│            │ - Impact Synth  │       │                │      │
│            │ - Resonance     │       │ - Perlin Noise │      │
│            │ - Oscillators   │       │ - Chaos Maps   │      │
│            │ - Mixer         │       │ - Spectral     │      │
│            └────────┬────────┘       └────────┬───────┘      │
│                     │                         │              │
│                     └────────────┬────────────┘             │
│                                  │                           │
│                          ┌───────▼────────┐                  │
│                          │  Audio Buffer  │                  │
│                          │  (Output)      │                  │
│                          └────────────────┘                  │
└─────────────────────────────────────────────────────────────┘
```

## Core Components

### 1. **Audio Synthesis Module** (`Audio/AudioSynthesizer.h`)
Procedural audio generation engine:
- **FOscillator**: Wavetable-based oscillator (sine, square, sawtooth, triangle, noise)
- **FEnvelopeGenerator**: ADSR envelope for amplitude/frequency modulation
- **FAudioMixer**: Combines multiple synthesis sources
- **FBaseSynthesizer**: Interface for custom synthesis algorithms

### 2. **Physics Module** (`Physics/PhysicsCore.h`)
Rigid body dynamics simulation:
- **FPhysicsObject**: Base rigid body with position, velocity, forces
- **FPhysicsSphere**: Sphere collider with collision detection
- **FPhysicsWorld**: Physics world manager and simulator
- **FImpactEvent**: Data structure for collision events

### 3. **Audio/Physics Integration** (`Integration/AudioPhysicsIntegration.h`)
Bridges physical simulation and audio:
- **FAudioPhysicsMapper**: Converts impact events to audio parameters
- **FImpactSynthesizer**: Generates transient sounds from collisions
- **FResonanceSynthesizer**: Creates sustained vibrations from physics
- **FAudioPhysicsSandbox**: Master orchestrator

### 4. **Procedural Generation** (`Procedural/ProceduralGeneration.h`)
Algorithmic parameter generation:
- **FPerlinNoiseGenerator**: Smooth, organic parameter evolution
- **FChaoticGenerator**: Deterministic chaos (logistic, Henon, Lorenz maps)
- **FSpectralGenerator**: Harmonic-based parameter generation
- **FMarkovGenerator**: Probabilistic state transitions
- **FProceduralController**: Manages multiple generators
- **FAdaptiveProceduralSystem**: Feedback-driven generation

### 5. **Sandbox Manager** (`SandboxManager.h`)
High-level API and example implementations:
- **FSandboxManager**: Main controller class
- **FPercussionSandbox**: Percussion impact examples
- **FResonantSurfaceSandbox**: Vibrating surfaces
- **FGranularPhysicsSandbox**: Granular synthesis from collisions

## Quick Start

### Basic Usage

```cpp
// Create sandbox
FSandboxManager Sandbox(48000.0f, 2048); // 48kHz, 2048 sample buffer

// Add physics objects
auto Sphere = MakeShared<FPhysicsSphere>(1.0f, 2.0f); // radius=1, mass=2
Sphere->SetPosition(FVector3(0, 5, 0));
Sandbox.AddPhysicsObject(Sphere);

// Apply force for collision
Sphere->ApplyImpulse(FVector3(0, -10, 0));

// Main loop
TArray<float> AudioBuffer;
for (float Time = 0; Time < 10.0f; Time += 0.016f) // 60 FPS
{
    Sandbox.Update(0.016f, AudioBuffer);
    // Output AudioBuffer to audio device
}
```

### Physics Audio Mapping

Impact events are automatically converted to audio:
1. **Collision Detection**: Physics world detects sphere-sphere collisions
2. **Impact Event**: Generates `FImpactEvent` with force, position, normal
3. **Audio Mapping**: `FAudioPhysicsMapper` converts to frequency and amplitude
4. **Synthesis**: `FImpactSynthesizer` generates audio transient
5. **Output**: Audio mixed into master output buffer

### Procedural Generation

Extend parameter generation with procedural algorithms:

```cpp
// Create Perlin noise generator for frequency evolution
auto FreqGen = MakeUnique<FPerlinNoiseGenerator>();
FreqGen->SetOctaves(3);
FreqGen->SetPersistence(0.6f);

FProceduralController* Controller = Sandbox.GetProceduralController();
Controller->SetFrequencyGenerator(MoveTemp(FreqGen));
Controller->SetFrequencyRange(100.0f, 2000.0f);

// Enable procedural generation
Sandbox.EnableProceduralGeneration(true);
```

## Audio/Physics Mapping Details

### Impact Frequency Calculation

```
Frequency = BaseFreq * MaterialHardness * sqrt(ImpactForce)
          * (1 + HarmonicRichness)
```

Material hardness ranges 0-1:
- 0.2: Soft (foam, rubber)
- 0.5: Medium (wood, plastic)
- 0.8: Hard (metal)
- 1.0: Very hard (ceramic, glass)

### Amplitude Mapping

```
Amplitude = ImpactForce * (1.0 - Damping) * MaterialDensity
Duration = InitialDuration * (1.0 - MaterialAbsorption)
```

## Advanced Features

### Resonant Surfaces

```cpp
FResonantSurfaceSandbox ResonantSandbox;
ResonantSandbox.AddResonator(440.0f, 8.0f, 1.0f); // A4, Q=8, mass=1
ResonantSandbox.ExciteResonator(0, 0.8f); // Excite with 80% energy
```

### Granular Synthesis

```cpp
FGranularPhysicsSandbox GranularSandbox;
GranularSandbox.ConfigureGrains(0.05f, 4); // 50ms grains, 4 overlapping
GranularSandbox.SetGrainWaveform(FOscillator::EWaveform::Sawtooth);
```

### Chaotic Parameter Evolution

```cpp
auto ChaoticGen = MakeUnique<FChaoticGenerator>(FChaoticGenerator::EChaosType::Lorenz);
ChaoticGen->SetChaosParameter(2.5f);
```

## Performance Considerations

- **Sample Rate**: 48kHz recommended (44.1kHz also supported)
- **Buffer Size**: 2048 samples (46ms latency at 48kHz)
- **Physics Timestep**: Use fixed timestep for stable simulation
- **Collision Detection**: O(n²) for naive implementation; optimize with spatial hashing
- **Audio Generation**: Lock-free for real-time safety

## Integration with Unreal Engine

To integrate with UE4/UE5:

1. Create `Plugins/AudioSandbox/Source/` directory
2. Copy all headers into the plugin
3. Create C++ binding module with `UObject` wrappers
4. Register with audio subsystem for hardware output
5. Use `FAudioDevice::GetMainAudioDevice()` for platform output

Example UObject wrapper:

```cpp
UCLASS()
class UAudioSandboxComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    FSandboxManager SandboxManager;

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    void OnAudioFinished(USoundWaveProcedural* InSoundWave);
};
```

## File Structure

```
AudioSandbox/
├── Source/
│   ├── Audio/
│   │   └── AudioSynthesizer.h
│   ├── Physics/
│   │   └── PhysicsCore.h
│   ├── Integration/
│   │   └── AudioPhysicsIntegration.h
│   ├── Procedural/
│   │   └── ProceduralGeneration.h
│   ├── SandboxManager.h
│   └── Examples/
│       └── (Implementation files to create)
└── README.md
```

## Future Extensions

- [ ] Implement `.cpp` files with full algorithm implementations
- [ ] Add material property system (damping, resonance frequency curves)
- [ ] Implement spatial audio (3D panning based on physics positions)
- [ ] Add FFT analyzer for real-time spectral feedback
- [ ] Create material library with pre-tuned parameters
- [ ] Implement multi-threaded physics and audio
- [ ] Add recording/playback system for generated audio
- [ ] Create visual debugging tools for audio/physics correlation

## Dependencies

- **Unreal Engine 5.0+** (or UE4.27 with minor modifications)
- **Standard Library**: `<cmath>`, `<algorithm>`
- No external audio libraries required

## License

[Specify your license here]

## Author

Audio Sandbox Framework for Physics-Driven Procedural Audio
