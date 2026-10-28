#pragma once

#include <JuceHeader.h>
#include "AfterglowSequencer.h"
#include <array>
#include <optional>

class AfterglowAudioProcessor final : public juce::AudioProcessor
{
public:
    AfterglowAudioProcessor();
    ~AfterglowAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    using juce::AudioProcessor::processBlock;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    juce::AudioProcessorParameter* getBypassParameter() const override { return parameters.getParameter("bypass"); }
    double getTailLengthSeconds() const override;
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    static juce::File getDataDirectory();
    static juce::File getSettingsDirectory();
    static juce::File getPresetsDirectory();
    static juce::File getSequencesDirectory();
    juce::Result savePresetToFile(const juce::File&);
    juce::Result loadPresetFromFile(const juce::File&);
    juce::File getCurrentPresetFile() const;
    std::optional<float> getSavedParameterValue(
        const juce::String& parameterId) const;
    int getActiveSequencerStep() const noexcept
    {
        return activeSequencerStep.load(std::memory_order_relaxed);
    }

private:
    static constexpr int stateSchemaVersion = 36;
    static constexpr int delayChannelCount = 2;
    static constexpr int delayModeCount = 18;
    static constexpr int distortionTypeCount = 16;
    static constexpr size_t multiTapCount = 4;
    static constexpr size_t distortionCombBufferSize = 96;
    static constexpr int diffusionStageCount = 4;
    static constexpr int diffusionBankCount = 4;
    static constexpr size_t ghostHilbertStages = 4;
    static constexpr size_t phaserStageCount = 12;
    static constexpr double maximumDelaySeconds = 8.0;
    static constexpr double maximumDiffusionSeconds = maximumDelaySeconds;
    static constexpr double maximumFreezeSeconds = maximumDelaySeconds * 2.0;
    static constexpr float maximumFeedback = 2.0f;

    using LinearSmoother = juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>;

    struct DigitalDelayState
    {
        float previousDelaySamples = 1.0f;
        float targetDelaySamples = 1.0f;
        float requestedDelaySamples = 1.0f;
        float crossfade = 1.0f;
    };

    struct TapeTransportState
    {
        float delaySamples = 1.0f;
        float velocitySamplesPerSecond = 0.0f;
    };

    struct AllpassState
    {
        float previousInput = 0.0f;
        float previousOutput = 0.0f;
    };

    struct RandomModulator
    {
        uint32_t noiseState = 1u;
        float value = 0.0f;
        float target = 0.0f;
        float step = 0.0f;
        int samplesRemaining = 0;
    };

    using HilbertPath = std::array<AllpassState, ghostHilbertStages>;

    struct ParameterPointers
    {
        std::atomic<float>* timeL = nullptr;
        std::atomic<float>* timeR = nullptr;
        std::atomic<float>* link = nullptr;
        std::atomic<float>* syncL = nullptr;
        std::atomic<float>* syncR = nullptr;
        std::atomic<float>* pingPong = nullptr;
        std::atomic<float>* delayStyle = nullptr;
        std::atomic<float>* divisionL = nullptr;
        std::atomic<float>* divisionR = nullptr;
        std::atomic<float>* multiTap = nullptr;
        std::atomic<float>* multiTapSync = nullptr;
        std::array<std::atomic<float>*, multiTapCount> tapTime {};
        std::array<std::atomic<float>*, multiTapCount> tapDivision {};
        std::array<std::atomic<float>*, multiTapCount> tapPan {};
        std::array<std::atomic<float>*, multiTapCount> tapLevel {};
        std::array<std::atomic<float>*, multiTapCount> tapEnabled {};
        std::atomic<float>* feedback = nullptr;
        std::atomic<float>* cross = nullptr;
        std::atomic<float>* spread = nullptr;
        std::atomic<float>* duck = nullptr;
        std::atomic<float>* duckAttack = nullptr;
        std::atomic<float>* duckRelease = nullptr;
        std::atomic<float>* diffusionAmount = nullptr;
        std::atomic<float>* diffusionSize = nullptr;
        std::atomic<float>* modulationAmount = nullptr;
        std::atomic<float>* modulationRate = nullptr;
        std::atomic<float>* pitchShift = nullptr;
        std::atomic<float>* pitchDetune = nullptr;
        std::atomic<float>* ghostShift = nullptr;
        std::atomic<float>* ghostDetune = nullptr;
        std::atomic<float>* phaserResonance = nullptr;
        std::atomic<float>* phaserStages = nullptr;
        std::atomic<float>* pitchMix = nullptr;
        std::atomic<float>* mix = nullptr;
        std::atomic<float>* lowCut = nullptr;
        std::atomic<float>* highCut = nullptr;
        std::atomic<float>* output = nullptr;
        std::atomic<float>* bypass = nullptr;
        std::atomic<float>* freeze = nullptr;
        std::atomic<float>* freezeToggle = nullptr;
        std::atomic<float>* mode = nullptr;
        std::atomic<float>* distortionType = nullptr;
        std::atomic<float>* drive = nullptr;
        std::atomic<float>* age = nullptr;
        std::atomic<float>* wow = nullptr;
        std::atomic<float>* flutter = nullptr;
        std::atomic<float>* sequencerEnabled = nullptr;
        std::atomic<float>* sequencerHostSync = nullptr;
        std::atomic<float>* sequencerRate = nullptr;
        std::atomic<float>* sequencerStart = nullptr;
        std::atomic<float>* sequencerEnd = nullptr;
        std::atomic<float>* sequencerDirection = nullptr;
        std::atomic<float>* sequencerBipolar = nullptr;
        std::atomic<float>* sequencerAttack = nullptr;
        std::atomic<float>* sequencerRelease = nullptr;
        std::atomic<float>* sequencerPeak = nullptr;
        std::atomic<float>* sequencerMove = nullptr;
        std::array<std::atomic<float>*, afterglow::sequencer::stepCount>
            sequencerSteps {};
        std::array<std::atomic<float>*, afterglow::sequencer::targetSlotCount>
            sequencerTargets {};
        std::array<std::atomic<float>*, afterglow::sequencer::targetSlotCount>
            sequencerTargetEnabled {};
    } raw;

    void bindParameterPointers();
    void setSavedParameterState(const juce::ValueTree&);
    void initialiseSmoother(LinearSmoother&, float value);
    void setDigitalDelayTarget(int channel, float delaySamples);
    float advanceTapeIITransport(TapeTransportState&, float targetDelaySamples) noexcept;
    void resetTapeIIState() noexcept;
    void resetProcessingState() noexcept;
    float readDelaySample(int channel, float delaySamples) const;
    float readDigitalDelay(int channel, float modulationSamples);
    float readPitchDelay(int channel, float delaySamples,
                         float pitchRatio) noexcept;
    float readPitchDelayWithPhase(int channel, float delaySamples,
                                  float pitchRatio,
                                  std::array<float, delayChannelCount>& phaseState) noexcept;
    float readReversePitchDelay(int channel, float spliceSamples,
                                float pitchRatio) noexcept;
    float readReversePitchDelayWithPhase(
        int channel, float spliceSamples, float pitchRatio,
        std::array<float, delayChannelCount>& phaseState,
        bool emphasiseShortSplices = false) noexcept;
    float processDiffusion(float sample, int channel, float amount,
                           float spanSamples, int bank) noexcept;
    float processGhostFrequencyShift(float sample, int channel) noexcept;
    float processGhostFrequencyShift(float sample, int channel,
                                     HilbertPath&, HilbertPath&) noexcept;
    void updateGhostOscillatorIncrements(float shiftHz,
                                         float detuneHz) noexcept;
    void advanceGhostOscillators() noexcept;
    void resetGhostState() noexcept;
    void resetLoFiState() noexcept;
    void resetCrystalState() noexcept;
    void resetPhaserState() noexcept;
    void resetGranularModeState() noexcept;
    float nextRandomModulation(RandomModulator&, float rateHz) noexcept;
    static float nextNoiseSample(uint32_t& state) noexcept;
    static float processAllpass(float sample, float coefficient,
                                AllpassState& state) noexcept;

    static float load(const std::atomic<float>*) noexcept;
    static float onePoleCoefficient(float cutoffHz, double sampleRate) noexcept;
    static float envelopeCoefficient(float timeMs, double sampleRate) noexcept;
    static float applyInputDrive(float sample, float amount,
                                 int distortionType) noexcept;
    float processDistortionType(float sample, float amount, int channel,
                                int distortionType) noexcept;
    float processRectifyDrive(float sample, float amount, int channel) noexcept;
    float processCrackleDrive(float sample, float amount, int channel) noexcept;
    float processLightningDrive(float sample, float amount, int channel) noexcept;
    float processAmpDrive(float sample, float amount, int channel) noexcept;
    float processBumpyDrive(float sample, float amount, int channel) noexcept;
    float processAsymmetricalDrive(float sample, float amount,
                                   int channel) noexcept;
    float processRazorDrive(float sample, float amount, int channel) noexcept;
    float processPixelatedDrive(float sample, float amount,
                                int channel) noexcept;
    float processCombDrive(float sample, float amount, int channel) noexcept;
    float processStressedDrive(float sample, float amount,
                               int channel) noexcept;
    float processSamuraiDrive(float sample, float amount, int channel) noexcept;
    float processFreshDrive(float sample, float amount, int channel) noexcept;
    static float processHardClipDrive(float sample, float amount) noexcept;
    void resetDistortionState() noexcept;
    static float applyFeedbackSaturation(float sample, float headroom) noexcept;
    static float applyOutputSafety(float sample) noexcept;
    static float sanitiseSample(float sample, float limit) noexcept;
    static bool migrateStateToCurrentSchema(juce::ValueTree&);
    bool isSequencerTargetAssigned(int targetChoice) const noexcept;
    float sequencerModulatedValue(int targetChoice, float baseValue,
                                  float canonicalSequenceValue,
                                  bool bipolar,
                                  juce::RangedAudioParameter* valueParameter
                                      = nullptr) const noexcept;

    juce::AudioBuffer<float> delayBuffer;
    std::array<juce::AudioBuffer<float>, diffusionStageCount>
        diffusionStageBuffer;
    juce::AudioBuffer<float> freezeBuffer;
    int writePosition = 0;
    std::array<int, diffusionStageCount> diffusionWritePosition {};
    int freezeCaptureWritePosition = 0;
    int freezeCaptureValidSamples = 0;
    std::array<int, delayChannelCount> freezeLoopStart {};
    std::array<int, delayChannelCount> freezeReadPosition {};
    std::array<int, delayChannelCount> freezeLoopLength { 1, 1 };
    bool freezePlaybackActive = false;
    std::atomic<int> activeSequencerStep { -1 };
    double sequencerFreePhase = 0.0;
    int sequencerHeldNotes = 0;
    bool sequencerPhraseTriggered = false;
    float previousSequencerCanonical = 1.0f;
    std::array<juce::RangedAudioParameter*, afterglow::sequencer::targets.size()>
        sequencerTargetParameters {};
    std::array<juce::RangedAudioParameter*, 2 + multiTapCount>
        sequencerTimeDivisionParameters {};
    int currentMode = 0;
    int currentDistortionType = 0;
    int previousDistortionType = 0;
    double currentSampleRate = 44100.0;

    std::array<DigitalDelayState, delayChannelCount> digitalDelay;
    std::array<TapeTransportState, delayChannelCount> tapeIITransport;
    std::array<TapeTransportState, multiTapCount> multiTapTapeIITransport;
    float digitalCrossfadeStep = 1.0f;
    LinearSmoother tapeDelayL;
    LinearSmoother tapeDelayR;
    LinearSmoother multiTapBlend;
    std::array<LinearSmoother, multiTapCount> multiTapDelayValue;
    std::array<LinearSmoother, multiTapCount> multiTapPanValue;
    std::array<LinearSmoother, multiTapCount> multiTapLevelValue;
    std::array<LinearSmoother, multiTapCount> multiTapEnabledValue;
    LinearSmoother multiTapLowpassCoefficientValue;
    LinearSmoother multiTapHighpassCoefficientValue;
    LinearSmoother multiTapHeadroomValue;
    LinearSmoother digitalModeBlend;
    LinearSmoother tapeModeBlend;
    LinearSmoother bbdModeBlend;
    LinearSmoother hifiModeBlend;
    LinearSmoother ghostModeBlend;
    LinearSmoother pitchModeBlend;
    LinearSmoother reversePitchModeBlend;
    LinearSmoother lofiModeBlend;
    LinearSmoother crystalModeBlend;
    LinearSmoother phaserModeBlend;
    LinearSmoother lushPitchModeBlend;
    LinearSmoother chromeTapeModeBlend;
    LinearSmoother analogModeBlend;
    LinearSmoother chaosModeBlend;
    LinearSmoother pureReverseModeBlend;
    LinearSmoother reverseChaosModeBlend;
    LinearSmoother reverseForwardModeBlend;
    LinearSmoother tapeIIModeBlend;

    LinearSmoother feedbackValue;
    LinearSmoother crossValue;
    LinearSmoother pingPongValue;
    LinearSmoother ratioStyleValue;
    LinearSmoother spreadValue;
    LinearSmoother duckAmountValue;
    LinearSmoother duckAttackCoefficientValue;
    LinearSmoother duckReleaseCoefficientValue;
    LinearSmoother diffusionAmountValue;
    LinearSmoother diffusionSizeValue;
    LinearSmoother modulationAmountValue;
    LinearSmoother modulationRateValue;
    LinearSmoother pitchRatioL;
    LinearSmoother pitchRatioR;
    LinearSmoother detuneRatioL;
    LinearSmoother detuneRatioR;
    LinearSmoother phaserResonanceValue;
    LinearSmoother phaserStageValue;
    LinearSmoother pitchMixValue;
    LinearSmoother mixValue;
    LinearSmoother outputGainValue;
    LinearSmoother bypassValue;
    LinearSmoother freezeValue;
    LinearSmoother driveValue;
    LinearSmoother distortionTypeCrossfade;
    LinearSmoother ageValue;
    LinearSmoother wowDepthValue;
    LinearSmoother flutterDepthValue;
    LinearSmoother lowpassCoefficientValue;
    LinearSmoother highpassCoefficientValue;
    LinearSmoother tapeLowpassCoefficientValue;
    LinearSmoother tapeHighpassCoefficientValue;
    LinearSmoother tapeIILowpassCoefficientValue;
    LinearSmoother tapeIIHighpassCoefficientValue;
    LinearSmoother chromeTapeLowpassCoefficientValue;
    LinearSmoother chromeTapeHighpassCoefficientValue;
    LinearSmoother hifiLowpassCoefficientValue;
    LinearSmoother hifiHighpassCoefficientValue;
    LinearSmoother ghostLowpassCoefficientValue;
    LinearSmoother ghostHighpassCoefficientValue;
    LinearSmoother lofiLowpassCoefficientValue;
    LinearSmoother lofiHighpassCoefficientValue;
    LinearSmoother crystalAgeLowpassCoefficientValue;
    std::array<LinearSmoother, delayChannelCount> bbdLowpassCoefficientValue;
    std::array<LinearSmoother, delayChannelCount> bbdHighpassCoefficientValue;
    std::array<LinearSmoother, delayChannelCount> analogLowpassCoefficientValue;
    std::array<LinearSmoother, delayChannelCount> analogHighpassCoefficientValue;

    std::array<float, delayChannelCount> lowpassState {};
    std::array<float, delayChannelCount> highpassState {};
    std::array<float, delayChannelCount> highpassInput {};
    std::array<float, delayChannelCount> tapeLowpassState {};
    std::array<float, delayChannelCount> tapeLowpassState2 {};
    std::array<float, delayChannelCount> tapeHighpassState {};
    std::array<float, delayChannelCount> tapeHighpassInput {};
    std::array<float, delayChannelCount> tapeIILowpassState {};
    std::array<float, delayChannelCount> tapeIILowpassState2 {};
    std::array<float, delayChannelCount> tapeIIHighpassState {};
    std::array<float, delayChannelCount> tapeIIHighpassInput {};
    std::array<float, delayChannelCount> chromeTapeLowpassState {};
    std::array<float, delayChannelCount> chromeTapeLowpassState2 {};
    std::array<float, delayChannelCount> chromeTapeHighpassState {};
    std::array<float, delayChannelCount> chromeTapeHighpassInput {};
    std::array<float, delayChannelCount> bbdLowpassState {};
    std::array<float, delayChannelCount> bbdLowpassState2 {};
    std::array<float, delayChannelCount> bbdHighpassState {};
    std::array<float, delayChannelCount> bbdHighpassInput {};
    std::array<float, delayChannelCount> analogLowpassState {};
    std::array<float, delayChannelCount> analogLowpassState2 {};
    std::array<float, delayChannelCount> analogHighpassState {};
    std::array<float, delayChannelCount> analogHighpassInput {};
    std::array<float, delayChannelCount> hifiLowpassState {};
    std::array<float, delayChannelCount> hifiHighpassState {};
    std::array<float, delayChannelCount> hifiHighpassInput {};
    std::array<float, delayChannelCount> ghostLowpassState {};
    std::array<float, delayChannelCount> ghostHighpassState {};
    std::array<float, delayChannelCount> ghostHighpassInput {};
    std::array<float, delayChannelCount> lofiLowpassState {};
    std::array<float, delayChannelCount> lofiLowpassState2 {};
    std::array<float, delayChannelCount> lofiHighpassState {};
    std::array<float, delayChannelCount> lofiHighpassInput {};
    std::array<float, delayChannelCount> crystalLowpassState {};
    std::array<float, delayChannelCount> crystalHighpassState {};
    std::array<float, delayChannelCount> crystalHighpassInput {};
    std::array<std::array<float, 4>, delayChannelCount> crystalAgeLowpassState {};
    std::array<float, delayChannelCount> digitalTriangleState {};
    std::array<float, delayChannelCount> crystalTriangleState {};
    std::array<std::array<float, delayChannelCount>, multiTapCount>
        multiTapDigitalTriangleState {};
    std::array<std::array<float, delayChannelCount>, multiTapCount>
        multiTapCrystalTriangleState {};
    std::array<std::array<float, delayChannelCount>, 2>
        reverseForwardLowpassState {};
    std::array<std::array<float, delayChannelCount>, 2>
        reverseForwardHighpassState {};
    std::array<std::array<float, delayChannelCount>, 2>
        reverseForwardHighpassInput {};
    std::array<std::array<AllpassState, phaserStageCount>, delayChannelCount>
        phaserAllpassState {};
    std::array<float, delayChannelCount> phaserFeedbackState {};
    std::array<float, delayChannelCount> rectifyPreviousInput {};
    std::array<float, delayChannelCount> rectifyPreviousOutput {};
    std::array<float, delayChannelCount> crackleBurst {};
    std::array<float, delayChannelCount> cracklePreviousInput {};
    std::array<float, delayChannelCount> cracklePreviousOutput {};
    std::array<float, delayChannelCount> lightningPreviousInput {};
    std::array<float, delayChannelCount> lightningPreviousOutput {};
    std::array<float, delayChannelCount> ampLowpassState {};
    std::array<float, delayChannelCount> ampPreviousInput {};
    std::array<float, delayChannelCount> ampPreviousOutput {};
    std::array<float, delayChannelCount> bumpyToneState {};
    std::array<float, delayChannelCount> bumpyPreviousInput {};
    std::array<float, delayChannelCount> bumpyPreviousOutput {};
    std::array<float, delayChannelCount> asymmetricalPreviousInput {};
    std::array<float, delayChannelCount> asymmetricalPreviousOutput {};
    std::array<float, delayChannelCount> razorToneState {};
    std::array<float, delayChannelCount> razorPreviousInput {};
    std::array<float, delayChannelCount> razorPreviousOutput {};
    std::array<float, delayChannelCount> pixelatedHeldSample {};
    std::array<float, delayChannelCount> pixelatedPhase {};
    std::array<std::array<float, distortionCombBufferSize>, delayChannelCount>
        distortionCombBuffer {};
    std::array<size_t, delayChannelCount> distortionCombWritePosition {};
    std::array<float, delayChannelCount> distortionCombDampingState {};
    std::array<float, delayChannelCount> stressedPreviousInput {};
    std::array<float, delayChannelCount> stressedPreviousOutput {};
    std::array<float, delayChannelCount> samuraiToneState {};
    std::array<float, delayChannelCount> samuraiPreviousInput {};
    std::array<float, delayChannelCount> samuraiPreviousOutput {};
    std::array<float, delayChannelCount> freshToneState {};
    std::array<float, multiTapCount> multiTapLowpassState {};
    std::array<float, multiTapCount> multiTapLowpassState2 {};
    std::array<float, multiTapCount> multiTapHighpassState {};
    std::array<float, multiTapCount> multiTapHighpassInput {};
    std::array<HilbertPath, delayChannelCount> ghostHilbertA {};
    std::array<HilbertPath, delayChannelCount> ghostHilbertB {};
    std::array<std::array<HilbertPath, delayChannelCount>, multiTapCount>
        multiTapGhostHilbertA {};
    std::array<std::array<HilbertPath, delayChannelCount>, multiTapCount>
        multiTapGhostHilbertB {};
    std::array<uint32_t, delayChannelCount> bbdNoiseState { 0x9e3779b9u, 0x243f6a88u };
    std::array<uint32_t, delayChannelCount> hifiNoiseState { 0xb7e15162u, 0x8aed2a6bu };
    std::array<uint32_t, delayChannelCount> lofiNoiseState { 0x85ebca6bu, 0xc2b2ae35u };
    std::array<uint32_t, delayChannelCount> crackleNoiseState { 0xa511e9b3u, 0x63d83595u };
    std::array<uint32_t, delayChannelCount> lightningNoiseState { 0x27d4eb2du, 0x165667b1u };
    std::array<uint32_t, delayChannelCount> pixelatedNoiseState { 0x94d049bbu, 0x369dea0fu };
    std::array<uint32_t, delayChannelCount> analogNoiseState { 0xd3a2646cu, 0xfd7046c5u };
    std::array<uint32_t, multiTapCount> multiTapNoiseState
    {
        0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au
    };
    std::array<RandomModulator, delayChannelCount> lofiSlowModulation;
    std::array<RandomModulator, delayChannelCount> lofiScrapeModulation;
    std::array<RandomModulator, delayChannelCount> chaosTimeModulation;
    std::array<RandomModulator, delayChannelCount> chaosPitchModulation;

    double wowPhase = 0.0;
    double flutterPhase = 0.0;
    double modulationPhase = 0.0;
    double crystalPhase = 0.0;
    double diffusionPhaseA = 0.0;
    double diffusionPhaseB = 0.0;
    std::array<float, delayChannelCount> pitchPhase { 0.0f, 0.0f };
    std::array<float, delayChannelCount> reversePitchPhase { 0.0f, 0.0f };
    std::array<float, delayChannelCount> pureReversePhase { 0.0f, 0.0f };
    std::array<float, delayChannelCount> reverseForwardPhase { 0.0f, 0.0f };
    std::array<float, delayChannelCount> chaosPhase { 0.0f, 0.0f };
    std::array<float, delayChannelCount> reverseChaosPhase { 0.0f, 0.0f };
    std::array<float, delayChannelCount> lushPitchShiftPhase { 0.0f, 0.0f };
    std::array<float, delayChannelCount> lushPitchDetunePhase { 0.25f, 0.75f };
    std::array<std::array<float, delayChannelCount>, multiTapCount>
        multiTapPitchPhase {};
    std::array<std::array<float, delayChannelCount>, multiTapCount>
        multiTapReversePitchPhase {};
    std::array<std::array<float, delayChannelCount>, multiTapCount>
        multiTapPureReversePhase {};
    std::array<std::array<float, delayChannelCount>, multiTapCount>
        multiTapReverseForwardPhase {};
    std::array<std::array<float, delayChannelCount>, multiTapCount>
        multiTapChaosPhase {};
    std::array<std::array<float, delayChannelCount>, multiTapCount>
        multiTapReverseChaosPhase {};
    std::array<std::array<float, delayChannelCount>, multiTapCount>
        multiTapLushShiftPhase {};
    std::array<std::array<float, delayChannelCount>, multiTapCount>
        multiTapLushDetunePhase
    {{
        { 0.25f, 0.75f }, { 0.25f, 0.75f },
        { 0.25f, 0.75f }, { 0.25f, 0.75f }
    }};
    float pitchWindowSamples = 64.0f;
    std::array<float, delayChannelCount> ghostOscillatorSin {};
    std::array<float, delayChannelCount> ghostOscillatorCos { 1.0f, 1.0f };
    std::array<float, delayChannelCount> ghostOscillatorSinIncrement {};
    std::array<float, delayChannelCount> ghostOscillatorCosIncrement
    { 1.0f, 1.0f };
    float rectifyDcCoefficient = 0.0f;
    float crackleDcCoefficient = 0.0f;
    float lightningDcCoefficient = 0.0f;
    float ampDcCoefficient = 0.0f;
    float ampToneCoefficient = 0.0f;
    float bumpyDcCoefficient = 0.0f;
    float bumpyToneCoefficient = 0.0f;
    float asymmetricalDcCoefficient = 0.0f;
    float razorToneCoefficient = 0.0f;
    float razorDcCoefficient = 0.0f;
    float distortionCombDampingCoefficient = 0.0f;
    float stressedDcCoefficient = 0.0f;
    float samuraiToneCoefficient = 0.0f;
    float samuraiDcCoefficient = 0.0f;
    float freshToneCoefficient = 0.0f;
    std::array<uint32_t, delayChannelCount> ghostOscillatorSamples {};
    float duckEnvelope = 0.0f;
    mutable juce::CriticalSection savedParameterStateLock;
    juce::ValueTree savedParameterState;
    std::atomic<bool> pendingProcessingReset { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AfterglowAudioProcessor)
};
