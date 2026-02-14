# Audio Sandbox - Quick Start Guide

## Installation & Setup

### Prerequisites
- C++17 compatible compiler (MSVC 2017+, GCC 7.0+, Clang 5.0+)
- CMake 3.18 or later
- (Optional) Unreal Engine 5.0+ for engine integration

### Building from Source

#### Windows (Visual Studio)
```powershell
# Clone or download the repository
cd AudioSandbox

# Create build directory
mkdir build
cd build

# Generate Visual Studio project
cmake -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release ..

# Build
cmake --build . --config Release

# Run examples
.\bin\Release\AudioSandboxExamples.exe
```

#### Linux/Mac (GCC/Clang)
```bash
cd AudioSandbox
mkdir build
cd build

cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .

./bin/AudioSandboxExamples
```

### Unreal Engine Integration

1. **Create Plugin Structure**
   ```
   Your Project/
   └── Plugins/
       └── AudioSandbox/
           ├── Binaries/
           ├── Resources/
           └── Source/
               ├── AudioSandbox/
               │   ├── Public/
               │   │   ├── Audio/
               │   │   ├── Physics/
               │   │   ├── Integration/
               │   │   ├── Procedural/
               │   │   └── SandboxManager.h
               │   └── Private/
               │       └── *.cpp files
               └── AudioSandbox.Build.cs
   ```

2. **Configure Build.cs**
   ```csharp
   PublicDependencyModuleNames.AddRange(new string[] { 
       "Core", 
       "Engine", 
       "AudioMixer" 
   });
   
   PublicIncludePaths.Add(ModuleDirectory + "/Public");
   ```

3. **Create Blueprint Wrapper**
   ```cpp
   UCLASS()
   class UAudioSandboxComponent : public UActorComponent
   {
       GENERATED_BODY()
   public:
       UFUNCTION(BlueprintCallable)
       void PlayImpactSound(float Frequency, float Amplitude, float Duration);
       
   private:
       FSandboxManager SandboxManager;
   };
   ```

## Basic Usage

### Example 1: Simple Impact Sound
```cpp
#include "SandboxManager.h"

int main()
{
    // Create sandbox (48kHz, 2048 sample buffer)
    FSandboxManager Sandbox(48000.0f, 2048);

    // Create a sphere
    auto Sphere = MakeShared<FPhysicsSphere>(0.5f, 2.0f);  // radius, mass
    Sphere->SetPosition(FVector3(0, 5, 0));
    Sandbox.AddPhysicsObject(Sphere);

    // Apply initial force
    Sphere->ApplyImpulse(FVector3(0, -10, 0));

    // Generate audio buffer
    TArray<float> AudioBuffer;
    for (int frame = 0; frame < 60; ++frame)  // 1 second at 48kHz
    {
        Sandbox.Update(1.0f/48000.0f, AudioBuffer);
        // Write AudioBuffer to audio device
    }

    return 0;
}
```

### Example 2: Controlled Synthesis
```cpp
FOscillator Osc(48000.0f);
Osc.SetFrequency(440.0f);      // A4
Osc.SetAmplitude(0.3f);         // 30% volume
Osc.SetWaveform(FOscillator::EWaveform::Sine);

TArray<float> Buffer;
Osc.GenerateSamples(Buffer, 2048);
// Play 'Buffer' through audio device
```

### Example 3: Procedural Generation
```cpp
FProceduralController ProcGen;

// Use Perlin noise for evolving frequency
auto FreqGen = MakeUnique<FPerlinNoiseGenerator>();
FreqGen->SetOctaves(3);
ProcGen.SetFrequencyGenerator(MoveTemp(FreqGen));

// Configure range
ProcGen.SetFrequencyRange(200.0f, 1000.0f);
ProcGen.SetAmplitudeRange(0.1f, 0.8f);

// Generate parameters
float Frequency, Amplitude, Richness, Duration;
ProcGen.GenerateParameters(Frequency, Amplitude, Richness, Duration);

// Use parameters for synthesis...
```

## Key Concepts

### Physics Objects
- **Position**: Where in 3D space
- **Velocity**: Direction and speed of movement
- **Mass**: Affects acceleration and impact force
- **Damping**: Energy loss over time (0 = no loss, 1 = complete loss)

### Audio Synthesis
- **Frequency**: Pitch in Hz (20-20000 Hz for human hearing)
- **Amplitude**: Volume (0 = silent, 1 = max)
- **Waveform**: Shape (sine, square, sawtooth, etc.)
- **Envelope**: Time-domain amplitude shaping (ADSR)

### Impact-to-Audio Mapping
```
Physics Impact Event
├─ ImpactForce (0-1)    →  Amplitude
├─ Position            →  Panning (when 3D enabled)
├─ Material Hardness   →  Frequency
└─ Impact Duration     →  Sound Duration
```

### Procedural Generation
Instead of static audio, parameters evolve over time using:
- **Perlin Noise**: Smooth, organic changes
- **Chaos Maps**: Complex, unpredictable evolution
- **Spectral**: Harmonic-based generation
- **Markov**: Probabilistic sequences

## Common Tasks

### Playing a Drum Hit
```cpp
FPercussionSandbox Drums(48000.0f);

// Drop sphere from height
Drums.DropObject(
    5.0f,    // height
    0.3f,    // radius
    0.8f     // material hardness
);

TArray<float> Buffer;
Drums.Update(1.0f/48.0f, Buffer);
```

### Creating a Resonant Surface
```cpp
FResonantSurfaceSandbox Resonator(48000.0f);

// Add resonators at different frequencies
Resonator.AddResonator(220.0f, 8.0f, 1.0f);  // A3, Q=8, mass=1
Resonator.AddResonator(330.0f, 8.0f, 1.0f);  // A4
Resonator.AddResonator(440.0f, 8.0f, 1.0f);  // A5

// Excite them
Resonator.ExciteResonator(0, 0.8f);  // Index, energy
Resonator.ExciteResonator(1, 0.6f);
Resonator.ExciteResonator(2, 0.4f);

TArray<float> Buffer;
Resonator.Update(0.016f, Buffer);  // 60 FPS
```

### Chaotic Parameter Evolution
```cpp
auto ChaoticGen = MakeUnique<FChaoticGenerator>(
    FChaoticGenerator::EChaosType::Henon
);
ChaoticGen->SetChaosParameter(1.4f);

FProceduralController Controller;
Controller.SetFrequencyGenerator(MoveTemp(ChaoticGen));
Controller.SetFrequencyRange(100.0f, 2000.0f);

// Each call generates different frequency
float Freq;
for (int i = 0; i < 10; ++i)
{
    Controller.GenerateParameters(Freq, ...);
    printf("Frequency %d: %.1f Hz\n", i, Freq);
}
```

## Configuration Parameters

### Physics World
```cpp
FPhysicsWorld* World = Sandbox.GetPhysicsWorld();

// Set gravity (default: 9.81 m/s² downward)
World->SetGravity(FVector3(0, -9.81f, 0));
```

### Object Properties
```cpp
auto Sphere = MakeShared<FPhysicsSphere>(radius, mass);

// Damping: 0 = no loss, 1 = complete loss
Sphere->SetDamping(0.99f);

// Position and velocity
Sphere->SetPosition(FVector3(0, 5, 0));
Sphere->SetVelocity(FVector3(10, 0, 0));
```

### Audio Synthesis
```cpp
// Frequency range for impact mapping
AudioPhysicsIntegration.GetMapper()->SetFrequencyRange(
    50.0f,    // Min Hz
    5000.0f   // Max Hz
);

// Master volume (0-1)
Sandbox.SetMasterVolume(0.8f);
```

### Procedural Parameters
```cpp
FProceduralController* Controller = Sandbox.GetProceduralController();

// Frequency range (Hz)
Controller->SetFrequencyRange(100.0f, 2000.0f);

// Amplitude range (0-1)
Controller->SetAmplitudeRange(0.1f, 0.8f);

// Duration range (seconds)
Controller->SetDurationRange(0.1f, 1.0f);
```

## Debugging & Analysis

### Check Peak Audio Level
```cpp
float PeakLevel = 0.0f;
for (float Sample : AudioBuffer)
{
    PeakLevel = FMath::Max(PeakLevel, FMath::Abs(Sample));
}
printf("Peak level: %.3f\n", PeakLevel);  // Should be < 1.0
```

### Monitor Physics Objects
```cpp
FSandboxManager::FSandboxStats Stats;
Sandbox.GetStats(Stats);

printf("Active objects: %d\n", Stats.ActivePhysicsObjects);
printf("Queued impacts: %d\n", Stats.QueuedImpacts);
printf("Average audio: %.3f\n", Stats.AverageAudioLevel);
```

### Verify Audio Format
- **Sample Rate**: 48000 Hz (standard for real-time audio)
- **Format**: Interleaved stereo (L, R, L, R, ...)
- **Bit Depth**: Float 32-bit IEEE
- **Range**: -1.0 to +1.0 (will clip if exceeded)

## Performance Tips

1. **Reduce Object Count**: Physics simulation is O(n²) for collisions
   - Use spatial hashing or grid for large object counts
   - Limit to <100 active objects for real-time performance

2. **Buffer Size Trade-off**:
   - Larger buffer = lower CPU usage but higher latency
   - 2048 samples = ~42ms latency at 48kHz
   - 512 samples = ~10ms latency (higher CPU)

3. **Generator Selection**:
   - Perlin Noise: Smooth, ~4-5 octaves
   - Chaos Maps: Fast computation, O(1)
   - Spectral: Heavier, consider fewer harmonics

4. **Synthesis Complexity**:
   - Sine wave: Cheapest
   - Wavetables (square, sawtooth): Medium
   - Noise: Medium
   - Complex filters: Most expensive

## Troubleshooting

### Audio Distortion
- Reduce master volume
- Check peak audio levels (should be < 1.0)
- Use soft clipping: `Sample = tanh(Sample)`

### Physics Objects Vibrating Abnormally
- Increase damping coefficient
- Reduce time step or use fixed timestep integration
- Check object masses and sizes are reasonable

### No Sound Generated
- Verify impact events are queued: `GetStats().QueuedImpacts`
- Check master volume is > 0
- Verify oscillator frequency is in human hearing range (20-20000 Hz)

### Memory Leaks
- Ensure `SharedPtr` objects are properly deleted
- Check audio buffer allocations in tight loops
- Use profiler to identify retention

## Next Steps

1. **Build the Library**
   - Follow compilation instructions above
   - Run example program to verify

2. **Integrate with Your Project**
   - Copy header files to your project
   - Link against AudioSandbox library
   - Include `<SandboxManager.h>`

3. **Extend Implementation**
   - Add custom synthesis algorithms
   - Implement material property system
   - Connect to audio hardware

4. **Optimize for Your Use Case**
   - Profile CPU usage
   - Tune buffer sizes
   - Optimize collision detection

5. **Create Your Soundscapes**
   - Experiment with procedural generators
   - Combine physics with audio feedback
   - Develop material libraries

## Additional Resources

- **ARCHITECTURE.md**: Detailed system design and component documentation
- **Examples.cpp**: Complete working examples for all major features
- **README.md**: Feature overview and API reference

## Support & Contributing

For issues, questions, or contributions:
1. Check ARCHITECTURE.md for design details
2. Review Examples.cpp for usage patterns
3. Examine source code inline documentation
4. Test with minimal reproducible example

## License

[Specify your license here]
