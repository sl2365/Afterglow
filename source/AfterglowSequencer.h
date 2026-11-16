#pragma once

#include <JuceHeader.h>
#include <array>
#include <algorithm>
#include <cmath>

namespace afterglow::sequencer
{
constexpr int stepCount = 32;
constexpr int targetSlotCount = 16;
constexpr int rateChoiceCount = 14;

enum class Direction
{
    loop = 0,
    bounce,
    reverse,
    played,
    random
};

enum class GateStepMode
{
    off = 0,
    shortStep,
    longStep,
    rest
};

enum class Waveform
{
    saw = 0,
    sawDouble,
    sawDown,
    sawDownDouble,
    sine,
    sineDouble,
    triangle,
    triangleDouble,
    pulse25,
    pulse25Double,
    square,
    squareDouble,
    pulse75,
    pulse75Double
};

struct TargetInfo
{
    const char* parameterId;
    const char* displayName;
};

inline constexpr std::array<TargetInfo, 55> targets
{{
    { "timeL", "Left Time" },
    { "timeR", "Right Time" },
    { "tap1Time", "Tap 1 Time" },
    { "tap1Pan", "Tap 1 Pan" },
    { "tap1Level", "Tap 1 Vol" },
    { "tap2Time", "Tap 2 Time" },
    { "tap2Pan", "Tap 2 Pan" },
    { "tap2Level", "Tap 2 Vol" },
    { "tap3Time", "Tap 3 Time" },
    { "tap3Pan", "Tap 3 Pan" },
    { "tap3Level", "Tap 3 Vol" },
    { "tap4Time", "Tap 4 Time" },
    { "tap4Pan", "Tap 4 Pan" },
    { "tap4Level", "Tap 4 Vol" },
    { "feedback", "Feedback" },
    { "cross", "Cross" },
    { "spread", "Spread" },
    { "duck", "Duck" },
    { "duckAttack", "Duck Attack" },
    { "duckRelease", "Duck Release" },
    { "diffusionAmount", "Diff Amount" },
    { "diffusionSize", "Diff Size" },
    { "modulationAmount", "Mod Amount" },
    { "modulationRate", "Mod Rate" },
    { "pitchShift", "Pitch Shift" },
    { "pitchDetune", "Pitch Detune" },
    { "ghostShift", "Ghost Shift" },
    { "ghostDetune", "Ghost Detune" },
    { "phaserResonance", "Phaser Res" },
    { "phaserStages", "Phaser Stage" },
    { "pitchMix", "Pitch Mix" },
    { "mix", "Dry / Wet" },
    { "lowCut", "Lo Cut" },
    { "highCut", "Hi Cut" },
    { "output", "Output" },
    { "drive", "Dist" },
    { "age", "Age" },
    { "wow", "Wow" },
    { "flutter", "Flutter" },
    { "sequencerAttack", "Seq Attack" },
    { "sequencerRelease", "Seq Release" },
    { "sequencerPeak", "Seq Depth" },
    { "sequencerMove", "Seq Offset" },
    { "sequencerStart", "Seq Start" },
    { "sequencerEnd", "Seq End" },
    { "twinTapeSpeed1", "Tape I Speed" },
    { "twinTapeSpeed2", "Tape II Speed" },
    { "twinTapeAge1", "Tape I Age" },
    { "twinTapeShine1", "Tape I Shine" },
    { "twinTapeAge2", "Tape II Age" },
    { "twinTapeShine2", "Tape II Shine" },
    { "twinTapeGain1", "Tape I Gain" },
    { "twinTapeGain2", "Tape II Gain" },
    { "sequencerGateShortLength", "Gate Short" },
    { "sequencerGateLongLength", "Gate Long" }
}};

inline constexpr int sequencerAttackTargetChoice = 40;
inline constexpr int sequencerReleaseTargetChoice = 41;
inline constexpr int sequencerDepthTargetChoice = 42;
inline constexpr int sequencerOffsetTargetChoice = 43;
inline constexpr int sequencerStartTargetChoice = 44;
inline constexpr int sequencerEndTargetChoice = 45;
inline constexpr int twinTapeSpeed1TargetChoice = 46;
inline constexpr int twinTapeSpeed2TargetChoice = 47;
inline constexpr int twinTapeAge1TargetChoice = 48;
inline constexpr int twinTapeShine1TargetChoice = 49;
inline constexpr int twinTapeAge2TargetChoice = 50;
inline constexpr int twinTapeShine2TargetChoice = 51;
inline constexpr int twinTapeGain1TargetChoice = 52;
inline constexpr int twinTapeGain2TargetChoice = 53;
inline constexpr int sequencerGateShortTargetChoice = 54;
inline constexpr int sequencerGateLongTargetChoice = 55;

inline int targetChoiceForParameter(const juce::String& parameterId)
{
    for (size_t index = 0; index < targets.size(); ++index)
        if (parameterId == targets[index].parameterId)
            return static_cast<int>(index) + 1;
    return 0;
}

inline juce::String targetName(int choice)
{
    if (choice <= 0 || choice > static_cast<int>(targets.size()))
        return {};
    return targets[static_cast<size_t>(choice - 1)].displayName;
}

inline juce::String stepParameterId(int step)
{
    return "seqStep" + juce::String(step + 1);
}

inline juce::String gateStepParameterId(int step)
{
    return "sequencerGateStep" + juce::String(step + 1);
}

inline GateStepMode gateStepModeFromChoice(float choice) noexcept
{
    return static_cast<GateStepMode>(juce::jlimit(
        static_cast<int>(GateStepMode::off),
        static_cast<int>(GateStepMode::rest),
        static_cast<int>(std::lround(choice))));
}

inline float gateStepTarget(GateStepMode mode, float position,
                            float shortLength, float longLength) noexcept
{
    position = juce::jlimit(0.0f, 1.0f, position);
    shortLength = juce::jlimit(0.10f, 0.60f, shortLength);
    longLength = juce::jlimit(0.65f, 0.95f, longLength);

    switch (mode)
    {
        case GateStepMode::shortStep:
            return position < shortLength ? 1.0f : 0.0f;
        case GateStepMode::longStep:
            return position < longLength ? 1.0f : 0.0f;
        case GateStepMode::rest:
            return 1.0f;
        case GateStepMode::off:
        default:
            return 0.0f;
    }
}

inline juce::String targetParameterId(int slot)
{
    return "seqTarget" + juce::String(slot + 1);
}

inline juce::String targetEnabledParameterId(int slot)
{
    return "seqTargetEnabled" + juce::String(slot + 1);
}

inline juce::String targetDepthParameterId(int slot)
{
    return targetParameterId(slot) + "_depth";
}

inline float applyTargetDepth(float canonicalSequenceValue, bool,
                              float depth) noexcept
{
    // Both Afterglow sequence displays share one canonical domain: 0.5 is
    // neutral, Unipolar uses 0.5-1.0 and Bipolar uses 0.0-1.0. Moving towards
    // that neutral point makes 0% hold the saved destination and 100% retain
    // the established modulation exactly.
    canonicalSequenceValue = juce::jlimit(
        0.0f, 1.0f, canonicalSequenceValue);
    depth = juce::jlimit(0.0f, 1.0f, depth);
    return 0.5f + (canonicalSequenceValue - 0.5f) * depth;
}

inline double beatsForRate(int rateIndex) noexcept
{
    constexpr std::array<double, rateChoiceCount> beats
    {
        1.0 / 32.0, 1.0 / 24.0, 1.0 / 16.0, 1.0 / 12.0,
        1.0 / 8.0, 1.0 / 6.0, 1.0 / 4.0, 1.0 / 3.0,
        1.0 / 2.0, 2.0 / 3.0, 1.0, 4.0 / 3.0, 2.0, 4.0
    };
    return beats[static_cast<size_t>(juce::jlimit(
        0, rateChoiceCount - 1, rateIndex))];
}

inline unsigned hash(unsigned value) noexcept
{
    value ^= value >> 16;
    value *= 0x7feb352dU;
    value ^= value >> 15;
    value *= 0x846ca68bU;
    value ^= value >> 16;
    return value;
}

inline int greatestCommonDivisor(int left, int right) noexcept
{
    while (right != 0)
    {
        const int remainder = left % right;
        left = right;
        right = remainder;
    }
    return left;
}

inline int randomOffset(int tick, int length) noexcept
{
    if (length <= 1)
        return 0;
    const int shuffle = tick / length;
    const int position = tick % length;
    const unsigned seed = hash(static_cast<unsigned>(length * 131
                                                     + shuffle * 977));
    int stride = 1 + static_cast<int>(seed
        % static_cast<unsigned>(length - 1));
    while (greatestCommonDivisor(stride, length) != 1)
        stride = stride % (length - 1) + 1;
    const int offset = static_cast<int>(
        hash(static_cast<unsigned>(length) ^ 0x9e3779b9U)
        % static_cast<unsigned>(length));
    return (offset + (shuffle & 1) + position * stride) % length;
}

struct Position
{
    int current = 0;
    int previous = 0;
    float fraction = 0.0f;
};

inline Position positionForPhase(double phase, int length,
                                 Direction direction) noexcept
{
    length = juce::jmax(1, length);
    const int cycleLength = direction == Direction::bounce && length > 1
                          ? 2 * (length - 1)
                          : direction == Direction::random ? length * 8
                          : length;
    phase = std::fmod(phase, static_cast<double>(cycleLength));
    if (phase < 0.0)
        phase += static_cast<double>(cycleLength);
    const int tick = juce::jlimit(
        0, cycleLength - 1, static_cast<int>(std::floor(phase)));
    const auto offsetAt = [=](int value)
    {
        value = (value % cycleLength + cycleLength) % cycleLength;
        if (direction == Direction::reverse)
            return length - 1 - value;
        if (direction == Direction::bounce && value >= length)
            return cycleLength - value;
        if (direction == Direction::random)
            return randomOffset(value, length);
        return value;
    };
    return { offsetAt(tick), offsetAt(tick - 1),
             static_cast<float>(phase - std::floor(phase)) };
}

inline float transition(float previous, float current, float position,
                        float attack, float release) noexcept
{
    previous = juce::jlimit(0.0f, 1.0f, previous);
    current = juce::jlimit(0.0f, 1.0f, current);
    if (std::abs(current - previous) <= 0.0001f)
        return current;
    const float amount = juce::jlimit(
        0.0f, 1.0f, current >= previous ? attack : release);
    if (amount <= 0.000001f)
        return current;
    const float progress = juce::jlimit(0.0f, 1.0f, position / amount);
    const float eased = 1.0f - (1.0f - progress) * (1.0f - progress);
    return previous + eased * (current - previous);
}

inline float applyDepthOffset(float value, float minimum, float maximum,
                              float depth, float offsetAmount,
                              bool bipolar) noexcept
{
    value = juce::jlimit(0.0f, 1.0f, value);
    minimum = juce::jlimit(0.0f, 1.0f, minimum);
    maximum = juce::jlimit(minimum, 1.0f, maximum);
    depth = juce::jlimit(-1.0f, 1.0f, depth);
    offsetAmount = juce::jlimit(-1.0f, 1.0f, offsetAmount);
    const float lowerLimit = bipolar ? -1.0f : 0.0f;
    const auto toWorking = [lowerLimit](float input)
    {
        return juce::jlimit(lowerLimit, 1.0f, 2.0f * input - 1.0f);
    };
    const auto fromWorking = [](float input)
    {
        return juce::jlimit(0.0f, 1.0f, 0.5f * (input + 1.0f));
    };
    float workingValue = toWorking(value);
    float workingMinimum = toWorking(minimum);
    float workingMaximum = toWorking(maximum);
    const float excursion = bipolar
        ? juce::jmax(std::abs(workingMinimum),
                     std::abs(workingMaximum))
        : workingMaximum;
    float scale = depth <= 0.0f ? 1.0f + depth : 1.0f;
    if (depth > 0.0f && excursion > 0.000001f)
        scale = 1.0f + depth * (1.0f / excursion - 1.0f);
    workingValue *= scale;
    workingMinimum *= scale;
    workingMaximum *= scale;
    const float offset = offsetAmount >= 0.0f
        ? offsetAmount * (1.0f - workingMaximum)
        : (-offsetAmount) * (lowerLimit - workingMinimum);
    return fromWorking(workingValue + offset);
}

inline float evaluate(
    const std::array<float, stepCount>& steps,
    double phase, int firstStep, int lastStep, Direction direction,
    float attack, float release, int* activeStep = nullptr) noexcept
{
    firstStep = juce::jlimit(0, stepCount - 1, firstStep);
    lastStep = juce::jlimit(firstStep, stepCount - 1, lastStep);
    const auto position = positionForPhase(
        phase, lastStep - firstStep + 1, direction);
    const int currentStep = firstStep + position.current;
    const int previousStep = firstStep + position.previous;
    if (activeStep != nullptr)
        *activeStep = currentStep;
    return transition(steps[static_cast<size_t>(previousStep)],
                      steps[static_cast<size_t>(currentStep)],
                      position.fraction, attack, release);
}

inline std::array<float, stepCount> makeWaveform(Waveform waveform,
                                                 bool bipolar) noexcept
{
    std::array<float, stepCount> result {};
    const int preset = static_cast<int>(waveform);
    const bool doubled = (preset & 1) != 0;
    const int shape = preset / 2;
    const int period = doubled ? stepCount / 2 : stepCount;
    for (int step = 0; step < stepCount; ++step)
    {
        const int local = step % period;
        const float phase = static_cast<float>(local)
                          / static_cast<float>(period);
        float value = 0.0f;
        if (shape == 0)
            value = static_cast<float>(local) / static_cast<float>(period - 1);
        else if (shape == 1)
            value = 1.0f - static_cast<float>(local)
                          / static_cast<float>(period - 1);
        else if (shape == 2)
            value = 0.5f + 0.5f * std::sin(
                juce::MathConstants<float>::twoPi * phase);
        else if (shape == 3)
            value = 1.0f - std::abs(2.0f * phase - 1.0f);
        else if (shape == 4)
            value = phase < 0.25f ? 1.0f : 0.0f;
        else if (shape == 5)
            value = phase < 0.50f ? 1.0f : 0.0f;
        else
            value = phase < 0.75f ? 1.0f : 0.0f;
        result[static_cast<size_t>(step)] = bipolar
            ? juce::jlimit(0.0f, 1.0f, value)
            : 0.5f + 0.5f * juce::jlimit(0.0f, 1.0f, value);
    }
    return result;
}
}
