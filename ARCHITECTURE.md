# Audio Sandbox - Architecture & Design Documentation

## Overview

The Audio Sandbox is a comprehensive framework for creating rich, evolving soundscapes through the integration of physics simulation and procedural audio synthesis. It enables the creation of naturalistic, dynamic audio that responds to physical interactions in real-time.

## Core Design Principles

### 1. **Separation of Concerns**
- **Physics Module**: Handles rigid body dynamics independently
- **Audio Module**: Manages all audio synthesis and generation
- **Integration Layer**: Maps physical events to audio parameters
- **Procedural Layer**: Generates autonomous parameter evolution

### 2. **Real-Time Performance**
- Deterministic update cycles for predictable performance
- Efficient data structures (preallocated buffers, object pooling)
- Lockfree processing where possible
- Minimal allocations in critical paths

### 3. **Modularity and Extensibility**
- Pure virtual interfaces for custom implementations
- Plugin architecture for additional synthesis algorithms
- Material property system for varied sonic characteristics
- Configurable integration mapping

### 4. **Bidirectional Feedback**
- Physics informs audio (collision → sound)
- Audio characteristics can influence physics parameters (optional)
- Procedural generation adapts based on audio content

## Detailed Component Architecture

### Audio Module (`Audio/`)

#### FOscillator
- **Purpose**: Core sound generation
- **Features**:
  - Multiple waveforms (sine, square, sawtooth, triangle, noise)
  - Wavetable synthesis for efficient processing
  - Real-time frequency and amplitude modulation
- **Performance**: O(1) per sample generation

```
┌─────────────────┐
│ FOscillator     │
├─────────────────┤
│ - CurrentPhase  │
│ - Frequency     │
│ - Amplitude     │
│ - Waveform      │
└─────────────────┘
         ▲
         │ Modulation
         │
    ┌─────────────────┐
    │ FEnvelopeGen    │
    │ (ADSR)          │
    └─────────────────┘
```

#### FEnvelopeGenerator
- **Purpose**: Time-domain amplitude and parameter shaping
- **Implementation**: State machine with ADSR stages
  - Attack: Linear ramp to peak
  - Decay: Linear ramp to sustain level
  - Sustain: Constant level
  - Release: Linear decay to zero
- **Smoothing**: Prevents clicks and pops

#### FAudioMixer
- **Purpose**: Combine multiple audio sources
- **Algorithm**: 
  1. Request samples from each source
  2. Sum with level scaling
  3. Soft clipping to prevent distortion
- **Complexity**: O(n·m) where n = sources, m = samples

### Physics Module (`Physics/`)

#### FVector3
- Custom 3D vector implementation
- Optimized dot product, magnitude, normalization
- No external dependencies (no UE4 FVector required)

#### FPhysicsObject
- **Base class** for all rigid bodies
- **State Variables**:
  - Position, Velocity, Acceleration
  - Mass, Damping
- **Integration**: Simple Euler method
  ```
  a = F / m
  v = v + a * dt
  p = p + v * dt
  ```

#### FPhysicsSphere
- **Derived class** for spherical colliders
- **Collision Detection**:
  - Sphere-sphere distance tests
  - Early rejection based on radii
- **Impact Calculation**:
  ```
  ImpactForce = RelativeVelocity * (m1 * m2) / (m1 + m2)
  ```

#### FPhysicsWorld
- **Role**: Container and coordinator
- **Responsibilities**:
  - Object lifecycle management
  - Collision detection (all-pairs O(n²))
  - Force application (gravity, constraints)
  - Step simulation
- **Constraints**: Ground plane with coefficient of restitution

```
Physics Update Loop
├─ Apply Gravity
├─ UpdateObjects (pos, vel integration)
├─ DetectCollisions
│  ├─ Sphere-sphere tests
│  ├─ Impulse-based response
│  └─ Overlap separation
└─ ApplyConstraints
   └─ Ground plane collision
```

### Integration Layer (`Integration/`)

#### FAudioPhysicsMapper
- **Core Mapping Function**:
  ```
  Physics Impact → Audio Parameters
  ├─ ImpactForce → Amplitude
  ├─ Material Properties → Frequency
  └─ Duration → Sound Length
  ```

- **Frequency Mapping**:
  ```
  f = f_base * (0.2 + 0.8 * Hardness) * sqrt(ImpactForce)
  ```
  - Hardness ∈ [0,1]: 0=soft(foam), 1=hard(ceramic)
  - ImpactForce ∈ [0,1]: Normalized impact magnitude

#### FImpactSynthesizer
- **Architecture**: Oscillator + Envelope + Frequency Decay
- **Sound Design**:
  - Fast attack (5ms) for percussive transients
  - Exponential frequency decay: f(t) = f₀ * r^t
  - ADSR envelope for energy shaping

#### FResonanceSynthesizer
- **Purpose**: Sustained vibrations and resonances
- **Mechanism**: 
  - Accumulates excitation energy
  - Exponential decay over time
  - Maintains oscillation at resonant frequency
- **Applications**: Vibrating plates, strings, bells

#### FAudioPhysicsSandbox
- **Orchestrator** for entire audio/physics integration
- **Update Sequence**:
  1. Read physics object states
  2. Detect high-velocity objects → generate FImpactEvent
  3. Queue impacts in circular buffer
  4. Process queued impacts → trigger synthesizers
  5. Mix audio output
  6. Apply master volume and clipping

### Procedural Layer (`Procedural/`)

#### FProceduralGenerator Interface
All generators inherit from this abstract base:

```cpp
class FProceduralGenerator
{
    virtual float GetNextValue() = 0;  // Returns [0,1]
    virtual void Reset() = 0;
    virtual void SetSeed(uint32) = 0;
};
```

#### FPerlinNoiseGenerator
- **Algorithm**: Gradient Persistence Noise
- **Characteristics**:
  - Smooth, organic parameter evolution
  - Octave-based harmonics
  - Configurable persistence for detail control
- **Use Cases**: 
  - Frequency sweeps
  - Amplitude envelopes
  - Subtle parameter modulation

#### FChaoticGenerator
Deterministic nonlinear systems for rich, evolving sounds:

**Logistic Map**:
```
x[n+1] = r * x[n] * (1 - x[n])
```
- Bifurcation behavior at different r values
- Transition from periodic to chaotic

**Henon Map** (2D):
```
x[n+1] = 1 - a * x[n]² + y[n]
y[n+1] = b * x[n]
```
- Creates strange attractor
- Rich, unpredictable sequences

**Lorenz-like Attractor**:
- Approximated behavior from Lorenz system
- Butterfly-like 3D trajectory

#### FSpectralGenerator
- **Method**: Harmonic summation
- **Formula**:
  ```
  y(t) = Σ A_i * sin(2π * f_i * t + φ_i)
  ```
- **Use Cases**:
  - Complex timbral evolution
  - Additive synthesis parameters
  - Frequency domain processing

#### FMarkovGenerator
- **Model**: First-order Markov chain
- **States**: Quantized parameter values [0, 0.1, 0.2, ..., 1.0]
- **Transitions**: Probabilistic state transitions
- **Use Cases**:
  - Melodic parameter sequences
  - Constrained randomization
  - Learning-based generation (can be trained)

#### FProceduralController
- **Manager** for multiple generators
- **Features**:
  - Independent generator per parameter
  - Range mapping (normalized → Hz, dB, duration)
  - Synchronized seeding across generators
- **Parameters Controlled**:
  1. Frequency (Hz)
  2. Amplitude (linear)
  3. Spectral Richness (0-1)
  4. Duration (seconds)

#### FAdaptiveProceduralSystem
- **Feedback Loop**: Audio metrics → parameter adaptation
- **Metrics**:
  - Overall energy
  - Spectral centroid (estimated)
  - Brightness (high-frequency content)
- **Adaptation**: Parameters adjust based on recent output

### Sandbox Manager (`SandboxManager.h`)

#### FSandboxManager
- **High-Level API** coordinating all components
- **Responsibilities**:
  - Manage physics world
  - Coordinate audio generation
  - Control procedural evolution
  - Performance monitoring

**Update Sequence** (per frame):
```
1. SimulatePhysics(DeltaTime)
2. GeneratePhysicsAudio()
3. GenerateProceduralAudio()
4. MixAudio()
5. ApplyEffects(volume, clipping)
6. Return stereo buffer
```

#### Specialized Sandboxes

**FPercussionSandbox**
- Optimized for impact sounds
- Default parameters for drums, kicks, impacts
- Convenient API for dropping/striking objects

**FResonantSurfaceSandbox**
- Multiple resonators with independent frequencies
- Collective damping control
- Bell-like, vibraphone-type sounds

**FGranularPhysicsSandbox**
- Each impact triggers grain envelope
- Granular synthesis from discrete collisions
- Overlapping grain voices for texture

## Data Flow Diagram

```
Physics Simulation
│
├─ Objects movement
├─ Collision detection
└─ Impact events
         │
         ▼
   FImpactEvent
   ├─ Position
   ├─ Momentum
   ├─ Time
   └─ Duration
         │
         ▼
FAudioPhysicsMapper
├─ Material properties
├─ Hardness curve
└─ Frequency mapping
         │
         ▼
   Audio Parameters
   ├─ Frequency (Hz)
   ├─ Amplitude (0-1)
   └─ Duration (seconds)
         │
    ┌────┴────┐
    │          │
    ▼          ▼
ImpactSynth  ResonanceSynth
    │          │
    └────┬─────┘
         │
         ▼
    FAudioMixer
         │
         ▼
    Soft Clipping
    Master Volume
         │
         ▼
   Stereo Buffer
   (to output)
```

## Performance Characteristics

### Computational Complexity

| Component | Complexity | Notes |
|-----------|-----------|-------|
| FOscillator (per sample) | O(1) | Wavetable lookup |
| FEnvelopeGenerator | O(1) | State machine |
| FAudioMixer | O(n·m) | n sources, m samples |
| FPhysicsWorld::DetectCollisions | O(n²) | Can optimize with spatial hashing |
| FPhysicsWorld::SimulateStep | O(n) | Per-object updates |
| FPerlinNoiseGenerator | O(octaves) | Typically 3-4 octaves |
| FChaoticGenerator | O(1) | Iterative map |
| FProceduralController | O(1) | Delegates to generators |

### Memory Usage

- **Per synthesizer instance**: ~4KB (state variables, buffers)
- **Per physics object**: ~100 bytes (vector3 position/vel/acc, floats)
- **Audio buffer** (2048 samples, stereo): 16KB
- **Physics world**: Minimal allocation until objects added

### Latency

- **Sample rate**: 48kHz (20.8µs per sample)
- **Buffer**: 2048 samples → 42.7ms latency
- **Physics timestep**: Locked to buffer (can subdivide)
- **Impact detection**: ~1-2 frames delay (typical)

## Design Patterns Used

### 1. **Template Method**
- `FBaseSynthesizer` defines generation contract
- Subclasses implement `GenerateSamples()` and `SetParameter()`

### 2. **Strategy Pattern**
- Multiple generator types implementing `FProceduralGenerator`
- Runtime selection of generation algorithm

### 3. **Observer Pattern**
- Physics events trigger audio generation
- Decoupled through event queue

### 4. **Factory Pattern**
- Sandbox manager classes encapsulate setup
- Specialized sandboxes for different use cases

### 5. **State Machine**
- FEnvelopeGenerator stages (Attack, Decay, Sustain, Release)
- FPhysicsWorld collision response logic

## Thread Safety

### Current Implementation
- **Single-threaded by design**
- Audio generation and physics on same thread
- Suitable for game engine integration (single game thread + audio thread)

### For Multi-threaded Use
1. Add atomic< > wrappers for shared state
2. Implement double-buffering for physics results
3. Use lock-free queue for audio update requests
4. Separate audio thread for hardware output

## Extension Points

### Adding New Synthesis Algorithms
```cpp
// 1. Inherit from FBaseSynthesizer
class FCustomSynthesizer : public FBaseSynthesizer
{
public:
    virtual void GenerateSamples(TArray<float>&, int32) override;
    virtual void SetParameter(const FName&, float) override;
};

// 2. Register with mixer
AudioMixer.AddSource(MakeShared<FCustomSynthesizer>());
```

### Adding New Procedural Generators
```cpp
// 1. Inherit from FProceduralGenerator
class FCustomGenerator : public FProceduralGenerator
{
public:
    virtual float GetNextValue() override;
    virtual void Reset() override;
    virtual void SetSeed(uint32) override;
};

// 2. Register with controller
ProceduralController.SetFrequencyGenerator(MakeUnique<FCustomGenerator>());
```

### Custom Physics Objects
```cpp
// 1. Inherit from FPhysicsObject
class FCustomShape : public FPhysicsObject
{
    virtual void Update(float DeltaTime) override;
};

// 2. Add to world
PhysicsWorld.AddObject(MakeShared<FCustomShape>());
```

## Integration with Unreal Engine

### Plugin Integration Points

1. **Audio Subsystem**
   - Connect to UE's `FAudioDevice`
   - Output to hardware speakers/headphones

2. **Blueprint Exposure**
   - Wrap core classes in `UCLASS()` derivatives
   - Expose parameters as `UPROPERTY`
   - Create Blueprint nodes for common operations

3. **Editor Integration**
   - Real-time preview of audio parameters
   - Visual feedback for physics simulation
   - Material editor for impact properties

4. **Content Integration**
   - Save generated audio as USoundWave
   - Store configurations as data assets
   - Implement material-based sound selection

## Future Enhancements

### Short-term
- [ ] FFT analyzer for real-time spectral feedback
- [ ] Material property library
- [ ] Spatial audio (3D panning)
- [ ] Recording/playback system

### Medium-term
- [ ] GPU acceleration for synthesis
- [ ] Machine learning parameter prediction
- [ ] Modal synthesis for realistic vibrations
- [ ] Frequency domain processing

### Long-term
- [ ] Network distributed processing
- [ ] Real-time collaboration features
- [ ] Advanced constraint solver
- [ ] Sound field synthesis

## References

### Physics
- Verlet Integration: Müller et al. "Interactive Virtual Materials"
- Collision Response: Ericson "Real-Time Collision Detection"

### Audio
- Wavetable Synthesis: Serafini et al. "Efficient Oscillator Design"
- Envelope Generation: Huopio "ADSR Envelope Generator"
- Procedural Audio: Farnell "Designing Sound"

### Procedural Generation
- Perlin Noise: Perlin K. "Improving Noise"
- Chaotic Systems: Strogatz "Nonlinear Dynamics and Chaos"
- Markov Chains: Rabiner "A Tutorial on Hidden Markov Models"
