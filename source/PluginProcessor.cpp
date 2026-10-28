#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

#if JUCE_WINDOWS
 #include <windows.h>
#endif

namespace
{
namespace ParameterIds
{
constexpr auto timeL = "timeL";
constexpr auto timeR = "timeR";
constexpr auto link = "link";
constexpr auto syncL = "syncL";
constexpr auto syncR = "syncR";
constexpr auto pingPong = "pingPong";
constexpr auto delayStyle = "delayStyle";
constexpr auto divisionL = "divisionL";
constexpr auto divisionR = "divisionR";
constexpr auto multiTap = "multiTap";
constexpr auto multiTapSync = "multiTapSync";
constexpr std::array<const char*, 4> tapTime
{
    "tap1Time", "tap2Time", "tap3Time", "tap4Time"
};
constexpr std::array<const char*, 4> tapDivision
{
    "tap1Division", "tap2Division", "tap3Division", "tap4Division"
};
constexpr std::array<const char*, 4> tapPan
{
    "tap1Pan", "tap2Pan", "tap3Pan", "tap4Pan"
};
constexpr std::array<const char*, 4> tapLevel
{
    "tap1Level", "tap2Level", "tap3Level", "tap4Level"
};
constexpr std::array<const char*, 4> tapEnabled
{
    "tap1Enabled", "tap2Enabled", "tap3Enabled", "tap4Enabled"
};
constexpr auto feedback = "feedback";
constexpr auto cross = "cross";
constexpr auto spread = "spread";
constexpr auto duck = "duck";
constexpr auto duckAttack = "duckAttack";
constexpr auto duckRelease = "duckRelease";
constexpr auto diffusionAmount = "diffusionAmount";
constexpr auto diffusionSize = "diffusionSize";
constexpr auto modulationAmount = "modulationAmount";
constexpr auto modulationRate = "modulationRate";
constexpr auto pitchShift = "pitchShift";
constexpr auto pitchDetune = "pitchDetune";
constexpr auto ghostShift = "ghostShift";
constexpr auto ghostDetune = "ghostDetune";
constexpr auto phaserResonance = "phaserResonance";
constexpr auto phaserStages = "phaserStages";
constexpr auto pitchMix = "pitchMix";
constexpr auto mix = "mix";
constexpr auto lowCut = "lowCut";
constexpr auto highCut = "highCut";
constexpr auto output = "output";
constexpr auto bypass = "bypass";
constexpr auto freeze = "freeze";
constexpr auto freezeToggle = "freezeToggle";
constexpr auto mode = "mode";
constexpr auto distortionType = "distortionType";
constexpr auto drive = "drive";
constexpr auto age = "age";
constexpr auto wow = "wow";
constexpr auto flutter = "flutter";
}

namespace ModeIndex
{
constexpr int digital = 0;
constexpr int tape = 1;
constexpr int bbd = 2;
constexpr int hifi = 3;
constexpr int ghost = 4;
constexpr int pitch = 5;
constexpr int reversePitch = 6;
constexpr int lofi = 7;
constexpr int crystal = 8;
constexpr int phaser = 9;
constexpr int lushPitch = 10;
constexpr int chromeTape = 11;
constexpr int analogue = 12;
constexpr int chaos = 13;
constexpr int pureReverse = 14;
constexpr int reverseChaos = 15;
constexpr int reverseForward = 16;
constexpr int tapeII = 17;
}

namespace DelayStyleIndex
{
constexpr int single = 0;
constexpr int dual = 1;
constexpr int ratio = 2;
constexpr int pingPong = 3;
constexpr int quad = 4;
}

constexpr float smoothingSeconds = 0.020f;
constexpr float tapeTimeSmoothingSeconds = 0.080f;
constexpr float maximumModulationDelaySeconds = 0.035f;
constexpr float minimumModulationRangeSeconds = 0.0025f;
constexpr float modulationRangePerDelaySecond = 0.13f;
constexpr float maximumDriveGain = 16.0f;
constexpr float defaultPitchShiftSemitones = 7.0f;
constexpr float maximumPitchShiftSemitones = 24.0f;
constexpr float maximumPitchDetuneSemitones = 1.0f;
constexpr float maximumGhostShiftHz = 1000.0f;
constexpr float maximumGhostDetuneHz = 100.0f;
constexpr float defaultGhostShiftHz = 0.0f;
constexpr float defaultGhostDetuneHz = 14.0f;
constexpr float pitchWindowSeconds = 0.080f;
constexpr float minimumDiffusionSizeFraction = 0.02f;
constexpr float maximumDiffusionModulationSeconds = 0.008f;
constexpr std::array<std::array<float, 4>, 2> diffusionStageFractions
{{
    { 0.109f, 0.193f, 0.301f, 0.397f },
    { 0.131f, 0.211f, 0.277f, 0.381f }
}};
const juce::StringArray tempoDivisionNames
{
    "1/64 T", "1/64", "1/32 T", "1/64 D", "1/32", "1/16 T", "1/32 D",
    "1/16", "1/8 T", "1/16 D", "1/8", "1/4 T", "1/8 D", "1/4", "1/4 D",
    "1/2", "1/1"
};
constexpr std::array<float, 17> tempoDivisionBeats
{
    1.0f / 24.0f, 0.0625f, 1.0f / 12.0f, 0.09375f, 0.125f,
    1.0f / 6.0f, 0.1875f, 0.25f, 1.0f / 3.0f, 0.375f,
    0.5f, 2.0f / 3.0f, 0.75f, 1.0f, 1.5f, 2.0f, 4.0f
};
constexpr int defaultLeftDivision = 13;
constexpr int defaultRightDivision = 10;
constexpr std::array<float, 4> defaultTapTimesMs
{
    250.0f, 375.0f, 500.0f, 750.0f
};
constexpr std::array<int, 4> defaultTapDivisions
{
    10, 12, 13, 14
};
constexpr std::array<float, 4> defaultTapPans
{
    -75.0f, -25.0f, 25.0f, 75.0f
};
constexpr std::array<float, 4> defaultTapLevels
{
    100.0f, 85.0f, 70.0f, 55.0f
};

float pitchRatioForSemitones(float semitones) noexcept
{
    return std::pow(2.0f, semitones / 12.0f);
}

float modulationPhaseOffsetTurns(int delayStyle, int channel) noexcept
{
    if (channel == 0)
        return 0.0f;

    switch (delayStyle)
    {
        case DelayStyleIndex::single:
            return 0.0f;
        case DelayStyleIndex::dual:
            return 0.25f;
        case DelayStyleIndex::ratio:
        case DelayStyleIndex::pingPong:
        case DelayStyleIndex::quad:
            return 0.5f;
        default:
            return 0.25f;
    }
}

float modulationDepthForDelay(float requestedDepthSamples,
                              float delaySamples,
                              float sampleRate) noexcept
{
    const float safeSampleRate = juce::jmax(1.0f, sampleRate);
    const float delaySeconds = juce::jmax(0.0f, delaySamples)
                             / safeSampleRate;
    const float availableRangeSeconds = juce::jmin(
        maximumModulationDelaySeconds,
        minimumModulationRangeSeconds
            + delaySeconds * modulationRangePerDelaySecond);
    return requestedDepthSamples
         * (availableRangeSeconds / maximumModulationDelaySeconds);
}

float floatingConverterLevels(float age) noexcept
{
    const float safeAge = juce::jlimit(0.0f, 1.0f, age);
    // 24 bits when new, 12 bits at 50% Age and 6 bits at 100%.
    const float bitDepth = 24.0f - 30.0f * safeAge
                         + 12.0f * safeAge * safeAge;
    return juce::jmax(31.0f, std::exp2(bitDepth - 1.0f) - 1.0f);
}

float quantiseFloatingConverter(float sample, float levels) noexcept
{
    return std::round(sample * levels) / levels;
}

juce::NormalisableRange<float> makeGhostShiftRange()
{
    return {
        -maximumGhostShiftHz, maximumGhostShiftHz,
        [] (float, float, float proportion)
        {
            const float bipolar = juce::jlimit(0.0f, 1.0f, proportion) * 2.0f
                                 - 1.0f;
            return std::copysign(maximumGhostShiftHz
                                     * std::pow(std::abs(bipolar), 3.0f),
                                 bipolar);
        },
        [] (float, float, float value)
        {
            const float normalised = std::cbrt(
                juce::jlimit(0.0f, 1.0f,
                             std::abs(value) / maximumGhostShiftHz));
            return 0.5f + 0.5f * std::copysign(normalised, value);
        },
        [] (float, float, float value)
        {
            return juce::jlimit(-maximumGhostShiftHz, maximumGhostShiftHz,
                                std::round(value * 10.0f) * 0.1f);
        }
    };
}

float feedbackGainForPercentage(float percentage) noexcept
{
    const float control = juce::jlimit(0.0f, 2.0f, percentage * 0.01f);
    if (control <= 0.0f)
        return 0.0f;

    if (control < 1.0f)
    {
        float calibratedGain = control;
        if (control > 0.80f)
        {
            // Retain the established near-unity calibration, including its
            // conservative replacement for the old multi-minute 95% tail.
            const float position = juce::jlimit(
                0.0f, 1.0f, (control - 0.80f) / 0.17f);
            const float blend = position * position
                              * (3.0f - 2.0f * position);
            const float doubledDecayGain = std::sqrt(control);
            calibratedGain += (doubledDecayGain - control) * blend;
        }

        // A loop's time to decay is inversely proportional to log(gain).
        // Raising the complete sub-unity calibration to 0.9 therefore adds
        // a predictable 1 / 0.9 (about 11%) to its decay duration at every
        // Feedback setting, rather than applying a mode- or preset-specific
        // gain offset.  Zero and exact unity remain exact endpoints.
        return std::pow(calibratedGain, 0.90f);
    }

    // Above 100%, retain a genuinely regenerative range.  The per-mode
    // nonlinearities stabilise the loop while the output safety stage keeps
    // the audible return bounded without flattening the feedback behaviour.
    return 1.0f + (control - 1.0f) * 0.25f;
}

float diffusionCoefficientForAmount(float amount) noexcept
{
    const float clamped = juce::jlimit(0.0f, 1.0f, amount);
    return clamped <= 0.91f
        ? clamped
        : 0.91f + (clamped - 0.91f) * (0.06f / 0.09f);
}

float noiseExcitationGain(float signal, float feedback,
                          float sensitivity = 8.0f) noexcept
{
    // Compander/tape noise should follow programme material below unity.  A
    // permanent noise floor in a very short high-feedback loop becomes an
    // audible oscillator even when the user has supplied no input.  Restore
    // a small free-running component only in the intentional regenerative
    // range, fading it in smoothly between 100% and 125% loop gain.
    const float signalActivity = juce::jlimit(
        0.0f, 1.0f, std::abs(signal) * sensitivity);
    const float regenerativePosition = juce::jlimit(
        0.0f, 1.0f, (feedback - 1.0f) * 4.0f);
    const float regenerativeNoise = regenerativePosition
        * regenerativePosition * (3.0f - 2.0f * regenerativePosition);
    return juce::jlimit(0.0f, 1.0f,
                        signalActivity + regenerativeNoise * 0.25f);
}
}

AfterglowAudioProcessor::AfterglowAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "AfterglowParameters", createParameterLayout())
{
    bindParameterPointers();
    parameters.state.setProperty("schemaVersion", stateSchemaVersion, nullptr);
    setSavedParameterState(parameters.copyState());
    getSettingsDirectory().createDirectory();
    getPresetsDirectory().createDirectory();
    getSequencesDirectory().createDirectory();
}

juce::File AfterglowAudioProcessor::getDataDirectory()
{
#if JUCE_WINDOWS
    std::array<wchar_t, 32768> modulePath {};
    const auto moduleHandle = static_cast<HMODULE>(
        juce::Process::getCurrentModuleInstanceHandle());

    if (moduleHandle != nullptr)
    {
        const auto pathLength = ::GetModuleFileNameW(
            moduleHandle, modulePath.data(),
            static_cast<DWORD>(modulePath.size()));
        if (pathLength > 0 && pathLength < modulePath.size() - 1)
        {
            const juce::File moduleFile(juce::String(modulePath.data()));
            auto folder = moduleFile.getParentDirectory();
            for (int depth = 0; depth < 6; ++depth)
            {
                if (folder.hasFileExtension(".vst3"))
                    return folder.getParentDirectory().getChildFile("Afterglow");

                const auto parent = folder.getParentDirectory();
                if (parent == folder)
                    break;
                folder = parent;
            }
            return moduleFile.getParentDirectory().getChildFile("Afterglow");
        }
    }
#endif
    return juce::File::getSpecialLocation(juce::File::currentApplicationFile)
        .getParentDirectory().getChildFile("Afterglow");
}

juce::File AfterglowAudioProcessor::getSettingsDirectory()
{
    return getDataDirectory().getChildFile("Settings");
}

juce::File AfterglowAudioProcessor::getPresetsDirectory()
{
    return getDataDirectory().getChildFile("Presets");
}

juce::File AfterglowAudioProcessor::getSequencesDirectory()
{
    return getDataDirectory().getChildFile("Sequences");
}

bool AfterglowAudioProcessor::migrateStateToCurrentSchema(
    juce::ValueTree& state)
{
    const int sourceSchema = static_cast<int>(
        state.getProperty("schemaVersion", 0));
    if (sourceSchema == stateSchemaVersion)
        return true;

    if (sourceSchema < 32 || sourceSchema > stateSchemaVersion)
        return false;

    const auto addParameterIfMissing = [&state] (const char* id, float value)
    {
        for (int childIndex = 0; childIndex < state.getNumChildren();
             ++childIndex)
        {
            if (state.getChild(childIndex).getProperty("id").toString() == id)
                return;
        }

        juce::ValueTree parameter("PARAM");
        parameter.setProperty("id", id, nullptr);
        parameter.setProperty("value", value, nullptr);
        state.addChild(parameter, -1, nullptr);
    };

    const auto parameterValue = [&state] (const char* id, float fallback)
    {
        for (int childIndex = 0; childIndex < state.getNumChildren();
             ++childIndex)
        {
            const auto child = state.getChild(childIndex);
            if (child.getProperty("id").toString() == id)
                return static_cast<float>(child.getProperty("value", fallback));
        }

        return fallback;
    };

    if (sourceSchema == 32)
    {
        // Schema 33 added only the two independent Ghost controls.  A 14 Hz
        // left/right separation exactly retains schema 32's fixed +/-7 Hz sound.
        addParameterIfMissing(ParameterIds::ghostShift, defaultGhostShiftHz);
        addParameterIfMissing(ParameterIds::ghostDetune, defaultGhostDetuneHz);
    }

    // Schema 34 records the source delay topology independently of its
    // materialised timing controls.  Existing public states can distinguish
    // every established page except Ratio, which previously had no stored
    // identity and therefore remains ordinary Dual timing until reconverted.
    int inferredStyle = DelayStyleIndex::dual;
    if (parameterValue(ParameterIds::multiTap, 0.0f) >= 0.5f)
        inferredStyle = DelayStyleIndex::quad;
    else if (parameterValue(ParameterIds::pingPong, 0.0f) >= 0.5f)
        inferredStyle = DelayStyleIndex::pingPong;
    else if (parameterValue(ParameterIds::link, 0.0f) >= 0.5f)
        inferredStyle = DelayStyleIndex::single;
    addParameterIfMissing(ParameterIds::delayStyle,
                          static_cast<float>(inferredStyle));

    // Schema 35 adds the sequencer. Earlier public presets must always open
    // with a neutral sequence and no assigned targets, irrespective of the
    // sequencer state that happened to be active before the preset was loaded.
    if (sourceSchema < 35)
    {
        addParameterIfMissing("sequencerEnabled", 1.0f);
        addParameterIfMissing("sequencerHostSync", 1.0f);
        addParameterIfMissing("sequencerRate", 6.0f);
        addParameterIfMissing("sequencerStart", 1.0f);
        addParameterIfMissing("sequencerEnd",
                              static_cast<float>(afterglow::sequencer::stepCount));
        addParameterIfMissing("sequencerRangeLink", 0.0f);
        addParameterIfMissing("sequencerDirection", 0.0f);
        addParameterIfMissing("sequencerBipolar", 0.0f);
        addParameterIfMissing("sequencerAttack", 0.0f);
        addParameterIfMissing("sequencerRelease", 0.0f);
        addParameterIfMissing("sequencerPeak", 0.0f);
        addParameterIfMissing("sequencerMove", 0.0f);

        for (int step = 0; step < afterglow::sequencer::stepCount; ++step)
            addParameterIfMissing(
                afterglow::sequencer::stepParameterId(step).toRawUTF8(), 1.0f);

        for (int slot = 0; slot < afterglow::sequencer::targetSlotCount; ++slot)
        {
            addParameterIfMissing(
                afterglow::sequencer::targetParameterId(slot).toRawUTF8(), 0.0f);
            addParameterIfMissing(
                afterglow::sequencer::targetEnabledParameterId(slot).toRawUTF8(),
                1.0f);
        }
    }
    if (sourceSchema < 36)
    {
        for (int child = state.getNumChildren() - 1; child >= 0; --child)
        {
            const auto id = state.getChild(child).getProperty("id").toString();
            if (id.startsWith("seqDivision") || id.startsWith("seqSub"))
                state.removeChild(child, nullptr);
        }
    }
    state.setProperty("schemaVersion", stateSchemaVersion, nullptr);
    return true;
}

juce::Result AfterglowAudioProcessor::savePresetToFile(
    const juce::File& presetFile)
{
    if (presetFile == juce::File())
        return juce::Result::fail("No preset file was selected.");

    const auto parentDirectory = presetFile.getParentDirectory();
    const auto presetsDirectory = getPresetsDirectory();
    if (parentDirectory != presetsDirectory
        && ! parentDirectory.isAChildOf(presetsDirectory))
        return juce::Result::fail(
            "Presets must be saved inside the Afterglow Presets folder.");

    if (! parentDirectory.isDirectory()
        && parentDirectory.createDirectory().failed())
        return juce::Result::fail("The preset folder could not be created.");

    auto state = parameters.copyState();
    state.setProperty("schemaVersion", stateSchemaVersion, nullptr);
    state.removeProperty("currentPresetPath", nullptr);
    const auto xml = state.createXml();
    if (xml == nullptr)
        return juce::Result::fail("The current plugin state could not be encoded.");

    const juce::TemporaryFile temporaryFile(presetFile);
    if (! temporaryFile.getFile().replaceWithText(xml->toString()))
        return juce::Result::fail("The preset file could not be written.");
    if (! temporaryFile.overwriteTargetFileWithTemporary())
        return juce::Result::fail("The preset file could not be replaced safely.");

    parameters.state.setProperty("currentPresetPath",
                                 presetFile.getFullPathName(), nullptr);
    setSavedParameterState(state);
    return juce::Result::ok();
}

juce::Result AfterglowAudioProcessor::loadPresetFromFile(
    const juce::File& presetFile)
{
    if (! presetFile.existsAsFile())
        return juce::Result::fail("The preset file no longer exists.");

    const auto presetsDirectory = getPresetsDirectory();
    const auto parentDirectory = presetFile.getParentDirectory();
    if (parentDirectory != presetsDirectory
        && ! parentDirectory.isAChildOf(presetsDirectory))
        return juce::Result::fail(
            "Presets must be loaded from the Afterglow Presets folder.");

    const auto xml = juce::XmlDocument::parse(presetFile);
    if (xml == nullptr || ! xml->hasTagName(parameters.state.getType()))
        return juce::Result::fail("This is not an Afterglow preset.");

    auto restored = juce::ValueTree::fromXml(*xml);
    if (! migrateStateToCurrentSchema(restored))
        return juce::Result::fail(
            "This preset was not created by the current Afterglow version.");

    restored.removeProperty("currentPresetPath", nullptr);
    setSavedParameterState(restored);
    parameters.replaceState(restored);
    pendingProcessingReset.store(true, std::memory_order_release);
    parameters.state.setProperty("currentPresetPath",
                                 presetFile.getFullPathName(), nullptr);
    return juce::Result::ok();
}

juce::File AfterglowAudioProcessor::getCurrentPresetFile() const
{
    const auto path = parameters.state.getProperty(
        "currentPresetPath", juce::String()).toString();
    return path.isEmpty() ? juce::File() : juce::File(path);
}

void AfterglowAudioProcessor::setSavedParameterState(
    const juce::ValueTree& state)
{
    const juce::ScopedLock lock(savedParameterStateLock);
    savedParameterState = state.createCopy();
    savedParameterState.removeProperty("currentPresetPath", nullptr);
}

std::optional<float> AfterglowAudioProcessor::getSavedParameterValue(
    const juce::String& parameterId) const
{
    const juce::ScopedLock lock(savedParameterStateLock);
    for (int index = 0; index < savedParameterState.getNumChildren(); ++index)
    {
        const auto child = savedParameterState.getChild(index);
        if (child.getProperty("id").toString() != parameterId)
            continue;

        const float value = child.getProperty("value").toString().getFloatValue();
        return std::isfinite(value) ? std::optional<float>(value)
                                    : std::nullopt;
    }

    return std::nullopt;
}

juce::AudioProcessorValueTreeState::ParameterLayout AfterglowAudioProcessor::createParameterLayout()
{
    using APF = juce::AudioParameterFloat;
    using APB = juce::AudioParameterBool;
    using APC = juce::AudioParameterChoice;
    using API = juce::AudioParameterInt;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> layout;
    const auto percentageAttributes = juce::AudioParameterFloatAttributes().withLabel("%");
    const auto millisecondAttributes = juce::AudioParameterFloatAttributes()
        .withLabel("ms")
        .withStringFromValueFunction([](float value, int)
        {
            return juce::String(value, value < 100.0f ? 1 : 0) + " ms";
        })
        .withValueFromStringFunction([](const juce::String& text)
        {
            return text.getFloatValue();
        });
    const auto frequencyAttributes = juce::AudioParameterFloatAttributes()
        .withLabel("Hz")
        .withStringFromValueFunction([](float value, int)
        {
            const int decimalPlaces = value < 1.0f ? 2 : 1;
            return juce::String(value, decimalPlaces) + " Hz";
        })
        .withValueFromStringFunction([](const juce::String& text)
        {
            return text.getFloatValue();
        });
    const auto semitoneAttributes = juce::AudioParameterFloatAttributes()
        .withLabel("st")
        .withStringFromValueFunction([](float value, int)
        {
            const auto sign = value > 0.0f ? "+" : "";
            return juce::String(sign) + juce::String(value, 2) + " st";
        })
        .withValueFromStringFunction([](const juce::String& text)
        {
            return text.getFloatValue();
        });
    const auto pitchDetuneAttributes = juce::AudioParameterFloatAttributes()
        .withLabel("st")
        .withStringFromValueFunction([](float value, int)
        {
            const auto sign = value > 0.0f ? "+" : "";
            return juce::String(sign) + juce::String(value, 3) + " st";
        })
        .withValueFromStringFunction([](const juce::String& text)
        {
            return text.getFloatValue();
        });
    const auto stageAttributes = juce::AudioParameterFloatAttributes()
        .withLabel("stages")
        .withStringFromValueFunction([](float value, int)
        {
            return juce::String(juce::roundToInt(value));
        })
        .withValueFromStringFunction([](const juce::String& text)
        {
            return static_cast<float>(text.getIntValue());
        });
    auto outputRange = juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f);
    outputRange.setSkewForCentre(0.0f);
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::timeL, 1), "Left Time", juce::NormalisableRange<float>(1.0f, 2000.0f, 0.1f, 0.45f), 375.0f, millisecondAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::timeR, 1), "Right Time", juce::NormalisableRange<float>(1.0f, 2000.0f, 0.1f, 0.45f), 500.0f, millisecondAttributes));
    layout.push_back(std::make_unique<APB>(juce::ParameterID(ParameterIds::link, 1), "Link Left and Right Time", false));
    layout.push_back(std::make_unique<APB>(juce::ParameterID(ParameterIds::syncL, 1), "Left Tempo Sync", false));
    layout.push_back(std::make_unique<APB>(juce::ParameterID(ParameterIds::syncR, 1), "Right Tempo Sync", false));
    layout.push_back(std::make_unique<APB>(juce::ParameterID(ParameterIds::pingPong, 1), "Ping Pong", false));
    layout.push_back(std::make_unique<APC>(juce::ParameterID(ParameterIds::delayStyle, 1), "Delay Style", juce::StringArray{ "Single", "Dual", "Ratio", "Ping Pong", "Quad" }, DelayStyleIndex::dual));
    jassert(tempoDivisionNames.size() == static_cast<int>(tempoDivisionBeats.size()));
    layout.push_back(std::make_unique<APC>(juce::ParameterID(ParameterIds::divisionL, 1), "Left Division", tempoDivisionNames, defaultLeftDivision));
    layout.push_back(std::make_unique<APC>(juce::ParameterID(ParameterIds::divisionR, 1), "Right Division", tempoDivisionNames, defaultRightDivision));
    layout.push_back(std::make_unique<APB>(juce::ParameterID(ParameterIds::multiTap, 1), "Multi-Tap Timing", false));
    layout.push_back(std::make_unique<APB>(juce::ParameterID(ParameterIds::multiTapSync, 1), "Multi-Tap Tempo Sync", false));
    for (size_t tap = 0; tap < multiTapCount; ++tap)
    {
        const auto tapNumber = juce::String(static_cast<int>(tap) + 1);
        layout.push_back(std::make_unique<APF>(
            juce::ParameterID(ParameterIds::tapTime[tap], 1),
            "Tap " + tapNumber + " Time",
            juce::NormalisableRange<float>(1.0f, 2000.0f, 0.1f, 0.45f),
            defaultTapTimesMs[tap], millisecondAttributes));
        layout.push_back(std::make_unique<APC>(
            juce::ParameterID(ParameterIds::tapDivision[tap], 1),
            "Tap " + tapNumber + " Division", tempoDivisionNames,
            defaultTapDivisions[tap]));
        layout.push_back(std::make_unique<APF>(
            juce::ParameterID(ParameterIds::tapPan[tap], 1),
            "Tap " + tapNumber + " Pan",
            juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f),
            defaultTapPans[tap], percentageAttributes));
        layout.push_back(std::make_unique<APF>(
            juce::ParameterID(ParameterIds::tapLevel[tap], 1),
            "Tap " + tapNumber + " Level",
            juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
            defaultTapLevels[tap], percentageAttributes));
        layout.push_back(std::make_unique<APB>(
            juce::ParameterID(ParameterIds::tapEnabled[tap], 1),
            "Tap " + tapNumber + " Enabled", true));
    }
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::feedback, 1), "Feedback", juce::NormalisableRange<float>(0.0f, maximumFeedback * 100.0f, 0.1f), 35.0f, percentageAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::cross, 1), "Cross Feedback", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f, percentageAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::spread, 1), "Spread", juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f), 100.0f, percentageAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::duck, 1), "Duck Amount", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f, percentageAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::duckAttack, 1), "Duck Attack", juce::NormalisableRange<float>(1.0f, 500.0f, 1.0f, 0.45f), 20.0f, millisecondAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::duckRelease, 1), "Duck Release", juce::NormalisableRange<float>(20.0f, 3000.0f, 1.0f, 0.45f), 400.0f, millisecondAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::diffusionAmount, 1), "Diffusion Amount", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f, percentageAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::diffusionSize, 1), "Diffusion Size", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 50.0f, percentageAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::modulationAmount, 1), "Modulation Amount", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f, percentageAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::modulationRate, 1), "Modulation Rate", juce::NormalisableRange<float>(0.02f, 10.0f, 0.01f, 0.35f), 0.30f, frequencyAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::pitchShift, 1), "Pitch Shift", juce::NormalisableRange<float>(-maximumPitchShiftSemitones, maximumPitchShiftSemitones, 0.01f), defaultPitchShiftSemitones, semitoneAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::pitchDetune, 1), "Pitch Detune", juce::NormalisableRange<float>(-maximumPitchDetuneSemitones, maximumPitchDetuneSemitones, 0.001f), 0.0f, pitchDetuneAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::ghostShift, 1), "Ghost Frequency Shift", makeGhostShiftRange(), defaultGhostShiftHz, frequencyAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::ghostDetune, 1), "Ghost Frequency Detune", juce::NormalisableRange<float>(0.0f, maximumGhostDetuneHz, 0.1f, 0.35f), defaultGhostDetuneHz, frequencyAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::phaserResonance, 1), "Phaser Resonance", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 30.0f, percentageAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::phaserStages, 1), "Phaser Stages", juce::NormalisableRange<float>(2.0f, 12.0f, 2.0f), 12.0f, stageAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::pitchMix, 1), "Pitch Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 50.0f, percentageAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::mix, 1), "Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 35.0f, percentageAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::lowCut, 1), "Low Cut", juce::NormalisableRange<float>(10.0f, 2000.0f, 1.0f, 0.35f), 10.0f));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::highCut, 1), "High Cut", juce::NormalisableRange<float>(200.0f, 20000.0f, 1.0f, 0.4f), 20000.0f));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::output, 1), "Output", outputRange, 0.0f, juce::AudioParameterFloatAttributes().withLabel("dB")));
    layout.push_back(std::make_unique<APB>(juce::ParameterID(ParameterIds::bypass, 1), "Bypass", false));
    layout.push_back(std::make_unique<APB>(juce::ParameterID(ParameterIds::freeze, 1), "Freeze", false));
    layout.push_back(std::make_unique<APB>(juce::ParameterID(ParameterIds::freezeToggle, 1), "Freeze Toggle Mode", false));
    layout.push_back(std::make_unique<APC>(juce::ParameterID(ParameterIds::mode, 1), "Mode", juce::StringArray{ "Digital", "Tape I", "Brigade", "HiFi", "Ghost", "Pitch", "Pitch Rev", "LoFi", "Crystal", "Phaser", "Pitch Lush", "Chromatic", "Analogue", "Chaotic", "Pure Reverse", "Chaotic Rev", "Forward Rev", "Tape II" }, 0));
    layout.push_back(std::make_unique<APC>(juce::ParameterID(ParameterIds::distortionType, 1), "Distortion Type", juce::StringArray{ "Soft Clip", "Saturation", "Tube", "Rectify", "Crackle", "Lightning", "Amp", "Bumpy", "Asym", "Razor", "Pixelated", "Comb", "Stressed", "Samurai", "Fresh", "Hard Clip" }, 0));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::drive, 1), "Distortion Amount", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f, percentageAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::age, 1), "Age", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 30.0f, percentageAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::wow, 1), "Wow", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f, percentageAttributes));
    layout.push_back(std::make_unique<APF>(juce::ParameterID(ParameterIds::flutter, 1), "Flutter", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f, percentageAttributes));

    layout.push_back(std::make_unique<APB>(
        juce::ParameterID("sequencerEnabled", 1), "Sequencer Enabled", true));
    layout.push_back(std::make_unique<APB>(
        juce::ParameterID("sequencerHostSync", 1), "Sequencer Host Sync", true));
    layout.push_back(std::make_unique<APC>(
        juce::ParameterID("sequencerRate", 1), "Sequencer Rate",
        juce::StringArray { "1/128", "1/64T", "1/64", "1/32T",
                            "1/32", "1/16T", "1/16", "1/8T",
                            "1/8", "1/4T", "1/4", "1/2T",
                            "1/2", "1/1" }, 6));
    layout.push_back(std::make_unique<API>(
        juce::ParameterID("sequencerStart", 1), "Sequencer Start", 1,
        afterglow::sequencer::stepCount - 1, 1));
    layout.push_back(std::make_unique<API>(
        juce::ParameterID("sequencerEnd", 1), "Sequencer End", 2,
        afterglow::sequencer::stepCount,
        afterglow::sequencer::stepCount));
    layout.push_back(std::make_unique<APB>(
        juce::ParameterID("sequencerRangeLink", 1),
        "Sequencer Range Link", false));
    layout.push_back(std::make_unique<APC>(
        juce::ParameterID("sequencerDirection", 1), "Sequencer Direction",
        juce::StringArray { "Loop", "Bounce", "Reverse", "Played", "Random" }, 0));
    layout.push_back(std::make_unique<APB>(
        juce::ParameterID("sequencerBipolar", 1), "Sequencer Bipolar", false));
    layout.push_back(std::make_unique<APF>(
        juce::ParameterID("sequencerAttack", 1), "Sequencer Attack",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));
    layout.push_back(std::make_unique<APF>(
        juce::ParameterID("sequencerRelease", 1), "Sequencer Release",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));
    layout.push_back(std::make_unique<APF>(
        juce::ParameterID("sequencerPeak", 1), "Sequencer Peak",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0f));
    layout.push_back(std::make_unique<APF>(
        juce::ParameterID("sequencerMove", 1), "Sequencer Move",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0f));

    for (int step = 0; step < afterglow::sequencer::stepCount; ++step)
    {
        layout.push_back(std::make_unique<APF>(
            juce::ParameterID(afterglow::sequencer::stepParameterId(step), 1),
            "Sequencer Step " + juce::String(step + 1),
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 1.0f));
    }

    juce::StringArray targetNames { "None" };
    for (const auto& target : afterglow::sequencer::targets)
        targetNames.add(target.displayName);
    for (int slot = 0; slot < afterglow::sequencer::targetSlotCount; ++slot)
    {
        layout.push_back(std::make_unique<APC>(
            juce::ParameterID(afterglow::sequencer::targetParameterId(slot), 1),
            "Sequencer Target " + juce::String(slot + 1), targetNames, 0));
        layout.push_back(std::make_unique<APB>(
            juce::ParameterID(
                afterglow::sequencer::targetEnabledParameterId(slot), 1),
            "Sequencer Target " + juce::String(slot + 1) + " Enabled", true));
    }

    return { layout.begin(), layout.end() };
}

void AfterglowAudioProcessor::bindParameterPointers()
{
    const auto bind = [this](const char* id)
    {
        auto* value = parameters.getRawParameterValue(id);
        jassert(value != nullptr);
        return value;
    };

    raw.timeL = bind(ParameterIds::timeL);
    raw.timeR = bind(ParameterIds::timeR);
    raw.link = bind(ParameterIds::link);
    raw.syncL = bind(ParameterIds::syncL);
    raw.syncR = bind(ParameterIds::syncR);
    raw.pingPong = bind(ParameterIds::pingPong);
    raw.delayStyle = bind(ParameterIds::delayStyle);
    raw.divisionL = bind(ParameterIds::divisionL);
    raw.divisionR = bind(ParameterIds::divisionR);
    raw.multiTap = bind(ParameterIds::multiTap);
    raw.multiTapSync = bind(ParameterIds::multiTapSync);
    for (size_t tap = 0; tap < multiTapCount; ++tap)
    {
        raw.tapTime[tap] = bind(ParameterIds::tapTime[tap]);
        raw.tapDivision[tap] = bind(ParameterIds::tapDivision[tap]);
        raw.tapPan[tap] = bind(ParameterIds::tapPan[tap]);
        raw.tapLevel[tap] = bind(ParameterIds::tapLevel[tap]);
        raw.tapEnabled[tap] = bind(ParameterIds::tapEnabled[tap]);
    }
    raw.feedback = bind(ParameterIds::feedback);
    raw.cross = bind(ParameterIds::cross);
    raw.spread = bind(ParameterIds::spread);
    raw.duck = bind(ParameterIds::duck);
    raw.duckAttack = bind(ParameterIds::duckAttack);
    raw.duckRelease = bind(ParameterIds::duckRelease);
    raw.diffusionAmount = bind(ParameterIds::diffusionAmount);
    raw.diffusionSize = bind(ParameterIds::diffusionSize);
    raw.modulationAmount = bind(ParameterIds::modulationAmount);
    raw.modulationRate = bind(ParameterIds::modulationRate);
    raw.pitchShift = bind(ParameterIds::pitchShift);
    raw.pitchDetune = bind(ParameterIds::pitchDetune);
    raw.ghostShift = bind(ParameterIds::ghostShift);
    raw.ghostDetune = bind(ParameterIds::ghostDetune);
    raw.phaserResonance = bind(ParameterIds::phaserResonance);
    raw.phaserStages = bind(ParameterIds::phaserStages);
    raw.pitchMix = bind(ParameterIds::pitchMix);
    raw.mix = bind(ParameterIds::mix);
    raw.lowCut = bind(ParameterIds::lowCut);
    raw.highCut = bind(ParameterIds::highCut);
    raw.output = bind(ParameterIds::output);
    raw.bypass = bind(ParameterIds::bypass);
    raw.freeze = bind(ParameterIds::freeze);
    raw.freezeToggle = bind(ParameterIds::freezeToggle);
    raw.mode = bind(ParameterIds::mode);
    raw.distortionType = bind(ParameterIds::distortionType);
    raw.drive = bind(ParameterIds::drive);
    raw.age = bind(ParameterIds::age);
    raw.wow = bind(ParameterIds::wow);
    raw.flutter = bind(ParameterIds::flutter);
    raw.sequencerEnabled = bind("sequencerEnabled");
    raw.sequencerHostSync = bind("sequencerHostSync");
    raw.sequencerRate = bind("sequencerRate");
    raw.sequencerStart = bind("sequencerStart");
    raw.sequencerEnd = bind("sequencerEnd");
    raw.sequencerDirection = bind("sequencerDirection");
    raw.sequencerBipolar = bind("sequencerBipolar");
    raw.sequencerAttack = bind("sequencerAttack");
    raw.sequencerRelease = bind("sequencerRelease");
    raw.sequencerPeak = bind("sequencerPeak");
    raw.sequencerMove = bind("sequencerMove");
    for (int step = 0; step < afterglow::sequencer::stepCount; ++step)
    {
        const auto index = static_cast<size_t>(step);
        raw.sequencerSteps[index] = bind(
            afterglow::sequencer::stepParameterId(step).toRawUTF8());
    }
    for (int slot = 0; slot < afterglow::sequencer::targetSlotCount; ++slot)
    {
        const auto index = static_cast<size_t>(slot);
        raw.sequencerTargets[index] = bind(
            afterglow::sequencer::targetParameterId(slot).toRawUTF8());
        raw.sequencerTargetEnabled[index] = bind(
            afterglow::sequencer::targetEnabledParameterId(slot).toRawUTF8());
    }
    for (size_t target = 0; target < afterglow::sequencer::targets.size();
         ++target)
        sequencerTargetParameters[target] = parameters.getParameter(
            afterglow::sequencer::targets[target].parameterId);
    sequencerTimeDivisionParameters[0]
        = parameters.getParameter(ParameterIds::divisionL);
    sequencerTimeDivisionParameters[1]
        = parameters.getParameter(ParameterIds::divisionR);
    for (size_t tap = 0; tap < multiTapCount; ++tap)
        sequencerTimeDivisionParameters[2 + tap]
            = parameters.getParameter(ParameterIds::tapDivision[tap]);
}

float AfterglowAudioProcessor::load(const std::atomic<float>* value) noexcept
{
    const float loaded = value != nullptr ? value->load(std::memory_order_relaxed) : 0.0f;
    return std::isfinite(loaded) ? loaded : 0.0f;
}

bool AfterglowAudioProcessor::isSequencerTargetAssigned(
    int targetChoice) const noexcept
{
    if (targetChoice <= 0)
        return false;
    for (size_t slot = 0; slot < raw.sequencerTargets.size(); ++slot)
        if (load(raw.sequencerTargetEnabled[slot]) >= 0.5f
            && static_cast<int>(std::lround(load(raw.sequencerTargets[slot])))
                   == targetChoice)
            return true;
    return false;
}

float AfterglowAudioProcessor::sequencerModulatedValue(
    int targetChoice, float baseValue, float canonicalSequenceValue,
    bool bipolar, juce::RangedAudioParameter* valueParameter) const noexcept
{
    if (! isSequencerTargetAssigned(targetChoice)
        || targetChoice > static_cast<int>(sequencerTargetParameters.size()))
        return baseValue;

    auto* parameter = valueParameter != nullptr
        ? valueParameter
        : sequencerTargetParameters[static_cast<size_t>(targetChoice - 1)];
    if (parameter == nullptr)
        return baseValue;

    const float baseNormalised = juce::jlimit(
        0.0f, 1.0f, parameter->convertTo0to1(baseValue));
    canonicalSequenceValue = juce::jlimit(
        0.0f, 1.0f, canonicalSequenceValue);
    float modulatedNormalised = baseNormalised;
    if (bipolar)
    {
        const float deviation = 2.0f * canonicalSequenceValue - 1.0f;
        const float available = deviation >= 0.0f
            ? 1.0f - baseNormalised : baseNormalised;
        modulatedNormalised = baseNormalised + deviation * available;
    }
    else
    {
        const float unipolar = juce::jlimit(
            0.0f, 1.0f, 2.0f * canonicalSequenceValue - 1.0f);
        modulatedNormalised = baseNormalised * unipolar;
    }
    return parameter->convertFrom0to1(
        juce::jlimit(0.0f, 1.0f, modulatedNormalised));
}

float AfterglowAudioProcessor::onePoleCoefficient(float cutoffHz, double sampleRate) noexcept
{
    const auto safeRate = juce::jmax(1.0, sampleRate);
    const auto safeCutoff = juce::jlimit(1.0f, static_cast<float>(safeRate * 0.45), cutoffHz);
    return std::exp(-juce::MathConstants<float>::twoPi * safeCutoff
                    / static_cast<float>(safeRate));
}

float AfterglowAudioProcessor::envelopeCoefficient(float timeMs, double sampleRate) noexcept
{
    const float safeTimeSeconds = juce::jmax(0.001f, timeMs * 0.001f);
    const float safeRate = static_cast<float>(juce::jmax(1.0, sampleRate));
    return std::exp(-1.0f / (safeTimeSeconds * safeRate));
}

float AfterglowAudioProcessor::applyInputDrive(float sample, float amount,
                                                int distortionType) noexcept
{
    const auto safeAmount = juce::jlimit(0.0f, 1.0f, amount);
    if (safeAmount <= 0.0f)
        return sample;

    float saturated = sample;
    if (distortionType == 2)
    {
        const float driveGain = 1.0f + safeAmount * 9.0f;
        const float bias = 0.05f + safeAmount * 0.25f;
        const float zeroOffset = std::tanh(bias);
        const float reference = std::tanh(0.80f * driveGain + bias)
                              - zeroOffset;
        const float normaliser = juce::jmax(0.001f, reference);
        const float outputCompensation = 1.0f / (1.0f + 0.9f * safeAmount);
        saturated = (std::tanh(sample * driveGain + bias) - zeroOffset)
                  / normaliser * outputCompensation;
    }
    else if (distortionType == 1)
    {
        const float driveGain = 1.0f + safeAmount * 7.0f;
        const float normaliser = juce::jmax(
            0.001f, std::atan(driveGain));
        const float outputCompensation = 1.0f / (1.0f + 1.2f * safeAmount);
        saturated = std::atan(sample * driveGain)
                  / normaliser * outputCompensation;
    }
    else
    {
        const float driveGain = 1.0f + safeAmount * (maximumDriveGain - 1.0f);
        const float outputCompensation = 1.0f / (1.0f + 3.0f * safeAmount);
        saturated = std::tanh(sample * driveGain) * outputCompensation;
    }
    return sample + (saturated - sample) * safeAmount;
}

float AfterglowAudioProcessor::processRectifyDrive(float sample, float amount,
                                                    int channel) noexcept
{
    const auto index = static_cast<size_t>(juce::jlimit(
        0, delayChannelCount - 1, channel));
    const float safeAmount = juce::jlimit(0.0f, 1.0f, amount);
    const float driveGain = 1.0f + safeAmount * 11.0f;
    const float rectified = std::abs(std::tanh(sample * driveGain));
    const float dcBlocked = rectified - rectifyPreviousInput[index]
                          + rectifyDcCoefficient * rectifyPreviousOutput[index];
    rectifyPreviousInput[index] = rectified;
    rectifyPreviousOutput[index] = sanitiseSample(dcBlocked, 4.0f);

    const float outputCompensation = 0.78f / (1.0f + 0.65f * safeAmount);
    const float shaped = rectifyPreviousOutput[index] * outputCompensation;
    return sample + (shaped - sample) * safeAmount;
}

float AfterglowAudioProcessor::processCrackleDrive(float sample, float amount,
                                                   int channel) noexcept
{
    const auto index = static_cast<size_t>(juce::jlimit(
        0, delayChannelCount - 1, channel));
    const float safeAmount = juce::jlimit(0.0f, 1.0f, amount);
    const float activity = juce::jlimit(0.0f, 1.0f, std::abs(sample) * 2.5f);
    const float eventRateHz = 4.0f + safeAmount * safeAmount * 360.0f;
    const float eventProbability = eventRateHz
        / static_cast<float>(juce::jmax(1.0, currentSampleRate));
    const float trigger = nextNoiseSample(crackleNoiseState[index]) * 0.5f + 0.5f;
    if (trigger < eventProbability * (0.20f + activity * 0.80f))
    {
        const float polarity = nextNoiseSample(crackleNoiseState[index]);
        crackleBurst[index] = polarity * (0.025f + safeAmount * 0.32f)
                            * (0.30f + activity * 0.70f);
    }

    const float burstDurationSeconds = juce::jmap(safeAmount, 0.0035f, 0.00065f);
    const float burstDecay = juce::jlimit(
        0.0f, 0.999999f,
        1.0f - 1.0f
            / (burstDurationSeconds
                * static_cast<float>(juce::jmax(1.0, currentSampleRate))));
    const float burst = crackleBurst[index];
    crackleBurst[index] *= burstDecay;

    const float driveGain = 1.0f + safeAmount * 5.0f;
    const float normaliser = juce::jmax(0.001f, std::tanh(driveGain));
    const float rough = std::tanh(sample * driveGain) / normaliser + burst;
    const float dcBlocked = rough - cracklePreviousInput[index]
                          + crackleDcCoefficient * cracklePreviousOutput[index];
    cracklePreviousInput[index] = rough;
    cracklePreviousOutput[index] = sanitiseSample(dcBlocked, 4.0f);
    const float outputCompensation = 1.0f / (1.0f + safeAmount * 0.55f);
    const float shaped = cracklePreviousOutput[index] * outputCompensation;
    return sample + (shaped - sample) * safeAmount;
}

float AfterglowAudioProcessor::processLightningDrive(float sample, float amount,
                                                      int channel) noexcept
{
    const auto index = static_cast<size_t>(juce::jlimit(
        0, delayChannelCount - 1, channel));
    const float safeAmount = juce::jlimit(0.0f, 1.0f, amount);
    const float activity = juce::jlimit(0.0f, 1.0f, std::abs(sample) * 3.0f);
    const float whiteNoise = nextNoiseSample(lightningNoiseState[index])
                           * safeAmount * safeAmount * 0.075f
                           * (0.20f + activity * 0.80f);
    const float driveGain = 1.0f + safeAmount * 18.0f;
    const float driven = sample * driveGain;
    const float hardClipped = juce::jlimit(-1.0f, 1.0f, driven);
    const float sharp = juce::jlimit(
        -1.05f, 1.05f, hardClipped + whiteNoise);
    const float dcBlocked = sharp - lightningPreviousInput[index]
                          + lightningDcCoefficient
                            * lightningPreviousOutput[index];
    lightningPreviousInput[index] = sharp;
    lightningPreviousOutput[index] = sanitiseSample(dcBlocked, 4.0f);

    const float outputCompensation = 0.92f / (1.0f + safeAmount * 1.35f);
    const float shaped = lightningPreviousOutput[index] * outputCompensation;
    return sample + (shaped - sample) * safeAmount;
}

float AfterglowAudioProcessor::processAmpDrive(float sample, float amount,
                                               int channel) noexcept
{
    const auto index = static_cast<size_t>(juce::jlimit(
        0, delayChannelCount - 1, channel));
    const float safeAmount = juce::jlimit(0.0f, 1.0f, amount);
    const float preampGain = 1.0f + safeAmount * 13.0f;
    const float bias = 0.08f + safeAmount * 0.22f;
    const float zeroOffset = std::tanh(bias);
    const float reference = juce::jmax(
        0.001f, std::tanh(preampGain * 0.75f + bias) - zeroOffset);
    const float preamp = (std::tanh(sample * preampGain + bias) - zeroOffset)
                       / reference;
    const float powerGain = 1.0f + safeAmount * 3.5f;
    const float powerReference = juce::jmax(0.001f, std::tanh(powerGain));
    const float powerStage = std::tanh(preamp * powerGain) / powerReference;

    ampLowpassState[index] = (1.0f - ampToneCoefficient) * powerStage
                           + ampToneCoefficient * ampLowpassState[index];
    const float dcBlocked = ampLowpassState[index] - ampPreviousInput[index]
                          + ampDcCoefficient * ampPreviousOutput[index];
    ampPreviousInput[index] = ampLowpassState[index];
    ampPreviousOutput[index] = sanitiseSample(dcBlocked, 4.0f);

    const float outputCompensation = 0.92f / (1.0f + safeAmount * 0.35f);
    const float shaped = ampPreviousOutput[index] * outputCompensation;
    return sample + (shaped - sample) * safeAmount;
}

float AfterglowAudioProcessor::processBumpyDrive(float sample, float amount,
                                                 int channel) noexcept
{
    const auto index = static_cast<size_t>(juce::jlimit(
        0, delayChannelCount - 1, channel));
    const float safeAmount = juce::jlimit(0.0f, 1.0f, amount);

    bumpyToneState[index] = (1.0f - bumpyToneCoefficient) * sample
                          + bumpyToneCoefficient * bumpyToneState[index];
    const float midBand = sample - bumpyToneState[index];
    const float preEmphasised = sample + safeAmount
        * (0.72f * midBand - 0.10f * bumpyToneState[index]);
    const float driveGain = 1.0f + safeAmount * 12.0f;
    const float driven = preEmphasised * driveGain;
    const float base = std::tanh(driven);
    const float ripple = std::sin(driven * (2.7f + safeAmount * 2.3f)
                                  + 0.35f * std::sin(driven * 1.7f));
    const float rippleEnvelope = safeAmount * 0.16f
        * (1.0f - std::exp(-std::abs(driven) * 0.80f));
    const float asymmetric = base + ripple * rippleEnvelope
                           + safeAmount * 0.055f * base * base;
    const float dcBlocked = asymmetric - bumpyPreviousInput[index]
                          + bumpyDcCoefficient
                            * bumpyPreviousOutput[index];
    bumpyPreviousInput[index] = asymmetric;
    bumpyPreviousOutput[index] = sanitiseSample(dcBlocked, 4.0f);

    const float outputCompensation = 0.92f / (1.0f + safeAmount * 1.05f);
    const float shaped = bumpyPreviousOutput[index] * outputCompensation;
    return sample + (shaped - sample) * safeAmount;
}

float AfterglowAudioProcessor::processAsymmetricalDrive(
    float sample, float amount, int channel) noexcept
{
    const auto index = static_cast<size_t>(juce::jlimit(
        0, delayChannelCount - 1, channel));
    const float safeAmount = juce::jlimit(0.0f, 1.0f, amount);
    const float driveGain = 1.0f + safeAmount * 11.0f;
    const float bias = 0.035f + safeAmount * 0.20f;
    const float zeroOffset = std::tanh(bias);
    const float positiveScale = juce::jmax(
        0.001f,
        std::tanh(driveGain * (1.0f + safeAmount * 0.55f) + bias)
            - zeroOffset);
    const float negativeScale = juce::jmax(
        0.001f,
        zeroOffset
            - std::tanh(-driveGain * (0.72f + safeAmount * 0.18f) + bias));
    const float shaped = sample >= 0.0f
        ? (std::tanh(sample * driveGain * (1.0f + safeAmount * 0.55f)
                     + bias) - zeroOffset) / positiveScale
        : (std::tanh(sample * driveGain * (0.72f + safeAmount * 0.18f)
                     + bias) - zeroOffset) / negativeScale;
    const float dcBlocked = shaped - asymmetricalPreviousInput[index]
                          + asymmetricalDcCoefficient
                                * asymmetricalPreviousOutput[index];
    asymmetricalPreviousInput[index] = shaped;
    asymmetricalPreviousOutput[index] = sanitiseSample(dcBlocked, 4.0f);

    const float outputCompensation = 0.94f / (1.0f + safeAmount * 0.68f);
    const float output = asymmetricalPreviousOutput[index]
                       * outputCompensation;
    return sample + (output - sample) * safeAmount;
}

float AfterglowAudioProcessor::processRazorDrive(float sample, float amount,
                                                 int channel) noexcept
{
    const auto index = static_cast<size_t>(juce::jlimit(
        0, delayChannelCount - 1, channel));
    const float safeAmount = juce::jlimit(0.0f, 1.0f, amount);
    razorToneState[index] = (1.0f - razorToneCoefficient) * sample
                          + razorToneCoefficient * razorToneState[index];
    const float edge = sample - razorToneState[index];
    const float preEmphasised = sample
                              + edge * safeAmount * (1.2f + safeAmount * 2.4f);
    const float driveGain = 1.0f + safeAmount * 19.0f;
    const float driven = preEmphasised * driveGain;
    const float sharp = std::tanh(driven)
                      + std::sin(driven * (2.6f + safeAmount * 1.8f))
                            * safeAmount * 0.15f;
    const float dcBlocked = sharp - razorPreviousInput[index]
                          + razorDcCoefficient * razorPreviousOutput[index];
    razorPreviousInput[index] = sharp;
    razorPreviousOutput[index] = sanitiseSample(dcBlocked, 4.0f);

    const float outputCompensation = 0.88f / (1.0f + safeAmount * 1.10f);
    const float output = razorPreviousOutput[index] * outputCompensation;
    return sample + (output - sample) * safeAmount;
}

float AfterglowAudioProcessor::processPixelatedDrive(
    float sample, float amount, int channel) noexcept
{
    const auto index = static_cast<size_t>(juce::jlimit(
        0, delayChannelCount - 1, channel));
    const float safeAmount = juce::jlimit(0.0f, 1.0f, amount);
    const float holdRate = 1.0f - 0.94f * safeAmount * safeAmount;
    pixelatedPhase[index] += holdRate;
    if (pixelatedPhase[index] >= 1.0f)
    {
        pixelatedPhase[index] -= 1.0f;
        pixelatedHeldSample[index] = sample;
    }

    const int bitDepth = juce::jlimit(
        4, 16, juce::roundToInt(16.0f - safeAmount * 12.0f));
    const float quantisationLevels = static_cast<float>(
        (1u << static_cast<unsigned int>(bitDepth - 1)) - 1u);
    const float dither = nextNoiseSample(pixelatedNoiseState[index])
                       * safeAmount * 0.35f / quantisationLevels;
    const float quantised = std::round(
        (pixelatedHeldSample[index] + dither) * quantisationLevels)
        / quantisationLevels;
    const float output = sanitiseSample(
        quantised / (1.0f + safeAmount * 0.08f), 2.0f);
    return sample + (output - sample) * safeAmount;
}

float AfterglowAudioProcessor::processCombDrive(float sample, float amount,
                                                int channel) noexcept
{
    const auto index = static_cast<size_t>(juce::jlimit(
        0, delayChannelCount - 1, channel));
    const float safeAmount = juce::jlimit(0.0f, 1.0f, amount);
    const size_t delaySamples = static_cast<size_t>(juce::jlimit(
        2, static_cast<int>(distortionCombBufferSize - 1),
        juce::roundToInt(47.0f - safeAmount * 36.0f)
            + static_cast<int>(index) * 3));
    const size_t write = distortionCombWritePosition[index];
    const size_t read = (write + distortionCombBufferSize - delaySamples)
                      % distortionCombBufferSize;
    const float delayed = distortionCombBuffer[index][read];
    distortionCombDampingState[index] =
        (1.0f - distortionCombDampingCoefficient) * delayed
        + distortionCombDampingCoefficient
            * distortionCombDampingState[index];
    const float feedback = 0.15f + safeAmount * 0.70f;
    distortionCombBuffer[index][write] = std::tanh(
        sample + distortionCombDampingState[index] * feedback);
    distortionCombWritePosition[index] =
        (write + 1u) % distortionCombBufferSize;

    const float driveGain = 1.0f + safeAmount * 2.5f;
    const float resonant = sample
        + distortionCombDampingState[index] * (0.30f + safeAmount * 0.60f);
    const float normaliser = juce::jmax(0.001f, std::tanh(driveGain));
    const float shaped = std::tanh(resonant * driveGain) / normaliser;
    const float output = shaped / (1.0f + safeAmount * 0.38f);
    return sample + (output - sample) * safeAmount;
}

float AfterglowAudioProcessor::processStressedDrive(
    float sample, float amount, int channel) noexcept
{
    const auto index = static_cast<size_t>(juce::jlimit(
        0, delayChannelCount - 1, channel));
    const float safeAmount = juce::jlimit(0.0f, 1.0f, amount);
    const float driveGain = 1.0f + safeAmount * 21.0f;
    const float driven = sample * driveGain;
    const float compressed = std::tanh(driven);
    const float folded = std::sin(
        driven * (1.25f + safeAmount * 0.75f));
    const float stressed = compressed * (1.0f - safeAmount * 0.30f)
                         + folded * safeAmount * 0.30f
                         + compressed * compressed * safeAmount * 0.045f;
    const float dcBlocked = stressed - stressedPreviousInput[index]
                          + stressedDcCoefficient
                                * stressedPreviousOutput[index];
    stressedPreviousInput[index] = stressed;
    stressedPreviousOutput[index] = sanitiseSample(dcBlocked, 4.0f);

    const float outputCompensation = 0.90f / (1.0f + safeAmount * 1.12f);
    const float output = stressedPreviousOutput[index] * outputCompensation;
    return sample + (output - sample) * safeAmount;
}

float AfterglowAudioProcessor::processSamuraiDrive(
    float sample, float amount, int channel) noexcept
{
    const auto index = static_cast<size_t>(juce::jlimit(
        0, delayChannelCount - 1, channel));
    const float safeAmount = juce::jlimit(0.0f, 1.0f, amount);
    samuraiToneState[index] = (1.0f - samuraiToneCoefficient) * sample
                            + samuraiToneCoefficient
                                * samuraiToneState[index];
    const float edge = sample - samuraiToneState[index];
    const float driven = (sample + edge * safeAmount * 2.8f)
                       * (1.0f + safeAmount * 15.0f);
    const float deadZone = 0.035f + safeAmount * 0.10f;
    const float slicedMagnitude = juce::jmax(
        0.0f, std::abs(driven) - deadZone);
    const float sliced = std::copysign(slicedMagnitude, driven);
    const float blade = std::tanh(sliced * (2.8f + safeAmount * 5.5f))
                      + std::sin(driven * 4.6f) * safeAmount * 0.10f;
    const float dcBlocked = blade - samuraiPreviousInput[index]
                          + samuraiDcCoefficient
                                * samuraiPreviousOutput[index];
    samuraiPreviousInput[index] = blade;
    samuraiPreviousOutput[index] = sanitiseSample(dcBlocked, 4.0f);

    const float outputCompensation = 0.87f / (1.0f + safeAmount * 0.78f);
    const float output = samuraiPreviousOutput[index] * outputCompensation;
    return sample + (output - sample) * safeAmount;
}

float AfterglowAudioProcessor::processFreshDrive(float sample, float amount,
                                                 int channel) noexcept
{
    const auto index = static_cast<size_t>(juce::jlimit(
        0, delayChannelCount - 1, channel));
    const float safeAmount = juce::jlimit(0.0f, 1.0f, amount);
    freshToneState[index] = (1.0f - freshToneCoefficient) * sample
                          + freshToneCoefficient * freshToneState[index];
    const float highBand = sample - freshToneState[index];
    const float exciterGain = 2.0f + safeAmount * 10.0f;
    const float exciterNormaliser = juce::jmax(
        0.001f, std::tanh(exciterGain));
    const float excitedHigh = std::tanh(highBand * exciterGain)
                           / exciterNormaliser;
    const float brightened = sample
        + excitedHigh * safeAmount * (0.32f + safeAmount * 0.40f);
    const float outputDrive = 1.0f + safeAmount * 0.55f;
    const float outputNormaliser = juce::jmax(
        0.001f, std::tanh(outputDrive));
    const float output = std::tanh(brightened * outputDrive)
                       / outputNormaliser
                       / (1.0f + safeAmount * 0.12f);
    return sample + (sanitiseSample(output, 2.0f) - sample) * safeAmount;
}

float AfterglowAudioProcessor::processHardClipDrive(float sample,
                                                    float amount) noexcept
{
    const float safeAmount = juce::jlimit(0.0f, 1.0f, amount);
    if (safeAmount <= 0.0f)
        return sample;

    const float inputGain = 1.0f + safeAmount * 19.0f;
    const float clipped = juce::jlimit(-1.0f, 1.0f, sample * inputGain);
    const float outputCompensation = 1.0f / (1.0f + safeAmount * 0.80f);
    const float shaped = clipped * outputCompensation;
    return sample + (shaped - sample) * safeAmount;
}

float AfterglowAudioProcessor::processDistortionType(
    float sample, float amount, int channel, int distortionType) noexcept
{
    switch (juce::jlimit(0, distortionTypeCount - 1, distortionType))
    {
        case 0:  return applyInputDrive(sample, amount, 0);
        case 1:  return applyInputDrive(sample, amount, 1);
        case 2:  return applyInputDrive(sample, amount, 2);
        case 3:  return processRectifyDrive(sample, amount, channel);
        case 4:  return processCrackleDrive(sample, amount, channel);
        case 5:  return processLightningDrive(sample, amount, channel);
        case 6:  return processAmpDrive(sample, amount, channel);
        case 7:  return processBumpyDrive(sample, amount, channel);
        case 8:  return processAsymmetricalDrive(sample, amount, channel);
        case 9:  return processRazorDrive(sample, amount, channel);
        case 10: return processPixelatedDrive(sample, amount, channel);
        case 11: return processCombDrive(sample, amount, channel);
        case 12: return processStressedDrive(sample, amount, channel);
        case 13: return processSamuraiDrive(sample, amount, channel);
        case 14: return processFreshDrive(sample, amount, channel);
        case 15: return processHardClipDrive(sample, amount);
        default: break;
    }

    return sample;
}

void AfterglowAudioProcessor::resetDistortionState() noexcept
{
    rectifyPreviousInput.fill(0.0f);
    rectifyPreviousOutput.fill(0.0f);
    crackleBurst.fill(0.0f);
    cracklePreviousInput.fill(0.0f);
    cracklePreviousOutput.fill(0.0f);
    crackleNoiseState = { 0xa511e9b3u, 0x63d83595u };
    lightningPreviousInput.fill(0.0f);
    lightningPreviousOutput.fill(0.0f);
    lightningNoiseState = { 0x27d4eb2du, 0x165667b1u };
    ampLowpassState.fill(0.0f);
    ampPreviousInput.fill(0.0f);
    ampPreviousOutput.fill(0.0f);
    bumpyToneState.fill(0.0f);
    bumpyPreviousInput.fill(0.0f);
    bumpyPreviousOutput.fill(0.0f);
    asymmetricalPreviousInput.fill(0.0f);
    asymmetricalPreviousOutput.fill(0.0f);
    razorToneState.fill(0.0f);
    razorPreviousInput.fill(0.0f);
    razorPreviousOutput.fill(0.0f);
    pixelatedHeldSample.fill(0.0f);
    pixelatedPhase.fill(1.0f);
    pixelatedNoiseState = { 0x94d049bbu, 0x369dea0fu };
    distortionCombBuffer = {};
    distortionCombWritePosition.fill(0u);
    distortionCombDampingState.fill(0.0f);
    stressedPreviousInput.fill(0.0f);
    stressedPreviousOutput.fill(0.0f);
    samuraiToneState.fill(0.0f);
    samuraiPreviousInput.fill(0.0f);
    samuraiPreviousOutput.fill(0.0f);
    freshToneState.fill(0.0f);
}

float AfterglowAudioProcessor::applyFeedbackSaturation(float sample, float headroom) noexcept
{
    const auto safeHeadroom = juce::jmax(0.001f, headroom);
    return safeHeadroom * std::tanh(sample / safeHeadroom);
}

float AfterglowAudioProcessor::applyOutputSafety(float sample) noexcept
{
    constexpr float knee = 1.0f;
    constexpr float ceiling = 1.5f;
    const float magnitude = std::abs(sample);
    if (magnitude <= knee)
        return sample;

    const float range = ceiling - knee;
    const float limited = knee + range * std::tanh((magnitude - knee) / range);
    return std::copysign(limited, sample);
}

float AfterglowAudioProcessor::sanitiseSample(float sample, float limit) noexcept
{
    if (! std::isfinite(sample))
        return 0.0f;

    return juce::jlimit(-limit, limit, sample);
}

bool AfterglowAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();
    const auto output = layouts.getMainOutputChannelSet();
    const bool supportedOutput = output == juce::AudioChannelSet::mono()
                              || output == juce::AudioChannelSet::stereo();
    return supportedOutput && input == output;
}

void AfterglowAudioProcessor::initialiseSmoother(LinearSmoother& smoother, float value)
{
    smoother.reset(currentSampleRate, smoothingSeconds);
    smoother.setCurrentAndTargetValue(value);
}

void AfterglowAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);

    currentSampleRate = juce::jmax(1.0, sampleRate);
    rectifyDcCoefficient = onePoleCoefficient(18.0f, currentSampleRate);
    crackleDcCoefficient = onePoleCoefficient(22.0f, currentSampleRate);
    lightningDcCoefficient = onePoleCoefficient(28.0f, currentSampleRate);
    ampDcCoefficient = onePoleCoefficient(24.0f, currentSampleRate);
    ampToneCoefficient = onePoleCoefficient(9000.0f, currentSampleRate);
    bumpyDcCoefficient = onePoleCoefficient(22.0f, currentSampleRate);
    bumpyToneCoefficient = onePoleCoefficient(1450.0f, currentSampleRate);
    asymmetricalDcCoefficient = onePoleCoefficient(22.0f, currentSampleRate);
    razorToneCoefficient = onePoleCoefficient(2100.0f, currentSampleRate);
    razorDcCoefficient = onePoleCoefficient(26.0f, currentSampleRate);
    distortionCombDampingCoefficient = onePoleCoefficient(
        7600.0f, currentSampleRate);
    stressedDcCoefficient = onePoleCoefficient(26.0f, currentSampleRate);
    samuraiToneCoefficient = onePoleCoefficient(2700.0f, currentSampleRate);
    samuraiDcCoefficient = onePoleCoefficient(28.0f, currentSampleRate);
    freshToneCoefficient = onePoleCoefficient(3400.0f, currentSampleRate);
    const auto delayBufferLength = static_cast<int>(std::ceil(currentSampleRate * maximumDelaySeconds)) + 8;
    delayBuffer.setSize(delayChannelCount, delayBufferLength, false, true, false);
    delayBuffer.clear();
    writePosition = 0;

    for (int stage = 0; stage < diffusionStageCount; ++stage)
    {
        const auto stageIndex = static_cast<size_t>(stage);
        const float maximumStageFraction = juce::jmax(
            diffusionStageFractions[0][stageIndex],
            diffusionStageFractions[1][stageIndex]);
        const auto stageBufferLength = static_cast<int>(std::ceil(
            currentSampleRate
                * (maximumDiffusionSeconds * maximumStageFraction
                   + maximumDiffusionModulationSeconds))) + 8;
        diffusionStageBuffer[stageIndex].setSize(
            delayChannelCount * diffusionBankCount,
            stageBufferLength, false, true, false);
        diffusionStageBuffer[stageIndex].clear();
        diffusionWritePosition[stageIndex] = 0;
    }

    const auto freezeBufferLength = static_cast<int>(
        std::ceil(currentSampleRate * maximumFreezeSeconds)) + 8;
    freezeBuffer.setSize(delayChannelCount, freezeBufferLength,
                         false, true, false);
    freezeBuffer.clear();
    freezeCaptureWritePosition = 0;
    freezeCaptureValidSamples = 0;
    freezeLoopStart.fill(0);
    freezeReadPosition.fill(0);
    freezeLoopLength.fill(1);
    freezePlaybackActive = false;
    sequencerFreePhase = 0.0;
    sequencerHeldNotes = 0;
    sequencerPhraseTriggered = false;
    previousSequencerCanonical = 1.0f;
    activeSequencerStep.store(-1, std::memory_order_relaxed);

    const float maximumDelaySamples = static_cast<float>(delayBufferLength - 4);
    const std::array<float, delayChannelCount> initialDelay
    {
        juce::jlimit(1.0f, maximumDelaySamples,
                     load(raw.timeL) * static_cast<float>(currentSampleRate / 1000.0)),
        juce::jlimit(1.0f, maximumDelaySamples,
                     load(raw.timeR) * static_cast<float>(currentSampleRate / 1000.0))
    };

    for (int channel = 0; channel < delayChannelCount; ++channel)
    {
        auto& state = digitalDelay[static_cast<size_t>(channel)];
        state.previousDelaySamples = initialDelay[static_cast<size_t>(channel)];
        state.targetDelaySamples = initialDelay[static_cast<size_t>(channel)];
        state.requestedDelaySamples = initialDelay[static_cast<size_t>(channel)];
        state.crossfade = 1.0f;
    }

    digitalCrossfadeStep = 1.0f / juce::jmax(1.0f, static_cast<float>(currentSampleRate * smoothingSeconds));

    tapeDelayL.reset(currentSampleRate, tapeTimeSmoothingSeconds);
    tapeDelayR.reset(currentSampleRate, tapeTimeSmoothingSeconds);
    tapeDelayL.setCurrentAndTargetValue(initialDelay[0]);
    tapeDelayR.setCurrentAndTargetValue(initialDelay[1]);

    const bool initialMultiTapSync = load(raw.multiTapSync) > 0.5f;
    initialiseSmoother(multiTapBlend,
                       load(raw.multiTap) > 0.5f ? 1.0f : 0.0f);
    for (size_t tap = 0; tap < multiTapCount; ++tap)
    {
        float tapMs = load(raw.tapTime[tap]);
        if (initialMultiTapSync)
        {
            const int division = juce::jlimit(
                0, static_cast<int>(tempoDivisionBeats.size()) - 1,
                static_cast<int>(std::lround(load(raw.tapDivision[tap]))));
            tapMs = 500.0f
                  * tempoDivisionBeats[static_cast<size_t>(division)];
        }
        multiTapDelayValue[tap].reset(
            currentSampleRate, tapeTimeSmoothingSeconds);
        multiTapDelayValue[tap].setCurrentAndTargetValue(juce::jlimit(
            1.0f, maximumDelaySamples,
            tapMs * static_cast<float>(currentSampleRate * 0.001)));
        initialiseSmoother(multiTapPanValue[tap], juce::jlimit(
            -1.0f, 1.0f, load(raw.tapPan[tap]) * 0.01f));
        initialiseSmoother(multiTapLevelValue[tap], juce::jlimit(
            0.0f, 1.0f, load(raw.tapLevel[tap]) * 0.01f));
        initialiseSmoother(multiTapEnabledValue[tap],
            load(raw.tapEnabled[tap]) > 0.5f ? 1.0f : 0.0f);
    }
    initialiseSmoother(multiTapLowpassCoefficientValue,
                       onePoleCoefficient(20000.0f, currentSampleRate));
    initialiseSmoother(multiTapHighpassCoefficientValue,
                       onePoleCoefficient(20.0f, currentSampleRate));
    initialiseSmoother(multiTapHeadroomValue, 8.0f);

    const int selectedMode = juce::jlimit(0, delayModeCount - 1,
        static_cast<int>(std::lround(load(raw.mode))));
    currentMode = selectedMode;
    digitalModeBlend.reset(currentSampleRate, smoothingSeconds);
    tapeModeBlend.reset(currentSampleRate, smoothingSeconds);
    bbdModeBlend.reset(currentSampleRate, smoothingSeconds);
    hifiModeBlend.reset(currentSampleRate, smoothingSeconds);
    ghostModeBlend.reset(currentSampleRate, smoothingSeconds);
    pitchModeBlend.reset(currentSampleRate, smoothingSeconds);
    reversePitchModeBlend.reset(currentSampleRate, smoothingSeconds);
    lofiModeBlend.reset(currentSampleRate, smoothingSeconds);
    crystalModeBlend.reset(currentSampleRate, smoothingSeconds);
    phaserModeBlend.reset(currentSampleRate, smoothingSeconds);
    lushPitchModeBlend.reset(currentSampleRate, smoothingSeconds);
    chromeTapeModeBlend.reset(currentSampleRate, smoothingSeconds);
    analogModeBlend.reset(currentSampleRate, smoothingSeconds);
    chaosModeBlend.reset(currentSampleRate, smoothingSeconds);
    pureReverseModeBlend.reset(currentSampleRate, smoothingSeconds);
    reverseChaosModeBlend.reset(currentSampleRate, smoothingSeconds);
    reverseForwardModeBlend.reset(currentSampleRate, smoothingSeconds);
    tapeIIModeBlend.reset(currentSampleRate, smoothingSeconds);
    digitalModeBlend.setCurrentAndTargetValue(selectedMode == 0 ? 1.0f : 0.0f);
    tapeModeBlend.setCurrentAndTargetValue(selectedMode == 1 ? 1.0f : 0.0f);
    bbdModeBlend.setCurrentAndTargetValue(selectedMode == 2 ? 1.0f : 0.0f);
    hifiModeBlend.setCurrentAndTargetValue(selectedMode == 3 ? 1.0f : 0.0f);
    ghostModeBlend.setCurrentAndTargetValue(selectedMode == 4 ? 1.0f : 0.0f);
    pitchModeBlend.setCurrentAndTargetValue(selectedMode == 5 ? 1.0f : 0.0f);
    reversePitchModeBlend.setCurrentAndTargetValue(selectedMode == 6 ? 1.0f : 0.0f);
    lofiModeBlend.setCurrentAndTargetValue(selectedMode == 7 ? 1.0f : 0.0f);
    crystalModeBlend.setCurrentAndTargetValue(selectedMode == 8 ? 1.0f : 0.0f);
    phaserModeBlend.setCurrentAndTargetValue(selectedMode == 9 ? 1.0f : 0.0f);
    lushPitchModeBlend.setCurrentAndTargetValue(selectedMode == 10 ? 1.0f : 0.0f);
    chromeTapeModeBlend.setCurrentAndTargetValue(selectedMode == 11 ? 1.0f : 0.0f);
    analogModeBlend.setCurrentAndTargetValue(selectedMode == 12 ? 1.0f : 0.0f);
    chaosModeBlend.setCurrentAndTargetValue(selectedMode == ModeIndex::chaos ? 1.0f : 0.0f);
    pureReverseModeBlend.setCurrentAndTargetValue(selectedMode == ModeIndex::pureReverse ? 1.0f : 0.0f);
    reverseChaosModeBlend.setCurrentAndTargetValue(selectedMode == ModeIndex::reverseChaos ? 1.0f : 0.0f);
    reverseForwardModeBlend.setCurrentAndTargetValue(selectedMode == ModeIndex::reverseForward ? 1.0f : 0.0f);
    tapeIIModeBlend.setCurrentAndTargetValue(selectedMode == ModeIndex::tapeII ? 1.0f : 0.0f);

    initialiseSmoother(feedbackValue, feedbackGainForPercentage(load(raw.feedback)));
    initialiseSmoother(crossValue, juce::jlimit(0.0f, 1.0f, load(raw.cross) * 0.01f));
    initialiseSmoother(pingPongValue, load(raw.pingPong) > 0.5f ? 1.0f : 0.0f);
    initialiseSmoother(ratioStyleValue,
                       static_cast<int>(std::lround(load(raw.delayStyle)))
                               == DelayStyleIndex::ratio
                           ? 1.0f : 0.0f);
    initialiseSmoother(spreadValue, juce::jlimit(-1.0f, 1.0f, load(raw.spread) * 0.01f));
    initialiseSmoother(duckAmountValue, juce::jlimit(0.0f, 1.0f, load(raw.duck) * 0.01f));
    initialiseSmoother(duckAttackCoefficientValue,
                       envelopeCoefficient(load(raw.duckAttack), currentSampleRate));
    initialiseSmoother(duckReleaseCoefficientValue,
                       envelopeCoefficient(load(raw.duckRelease), currentSampleRate));
    initialiseSmoother(diffusionAmountValue,
                       juce::jlimit(0.0f, 1.0f, load(raw.diffusionAmount) * 0.01f));
    initialiseSmoother(diffusionSizeValue,
                       juce::jlimit(0.0f, 1.0f, load(raw.diffusionSize) * 0.01f));
    const float modulationAmount = juce::jlimit(
        0.0f, 1.0f, load(raw.modulationAmount) * 0.01f);
    initialiseSmoother(modulationAmountValue,
                       modulationAmount * modulationAmount
                           * static_cast<float>(currentSampleRate
                                               * maximumModulationDelaySeconds));
    initialiseSmoother(modulationRateValue,
                       juce::jlimit(0.02f, 10.0f, load(raw.modulationRate)));
    initialiseSmoother(mixValue, juce::jlimit(0.0f, 1.0f, load(raw.mix) * 0.01f));
    initialiseSmoother(outputGainValue, juce::Decibels::decibelsToGain(load(raw.output)));
    initialiseSmoother(bypassValue, load(raw.bypass) > 0.5f ? 1.0f : 0.0f);
    initialiseSmoother(freezeValue, load(raw.freeze) > 0.5f ? 1.0f : 0.0f);
    initialiseSmoother(driveValue, juce::jlimit(0.0f, 1.0f, load(raw.drive) * 0.01f));
    currentDistortionType = juce::jlimit(
        0, distortionTypeCount - 1,
        static_cast<int>(std::lround(load(raw.distortionType))));
    previousDistortionType = currentDistortionType;
    initialiseSmoother(distortionTypeCrossfade, 1.0f);
    initialiseSmoother(phaserResonanceValue,
                       juce::jlimit(0.0f, 1.0f,
                                    load(raw.phaserResonance) * 0.01f));
    initialiseSmoother(phaserStageValue,
                       juce::jlimit(2.0f, 12.0f,
                                    load(raw.phaserStages)));
    initialiseSmoother(pitchMixValue,
                       juce::jlimit(0.0f, 1.0f,
                                    load(raw.pitchMix) * 0.01f));

    const auto wow = juce::jlimit(0.0f, 1.0f, load(raw.wow) * 0.01f);
    const auto flutter = juce::jlimit(0.0f, 1.0f, load(raw.flutter) * 0.01f);
    initialiseSmoother(wowDepthValue, wow * wow * static_cast<float>(currentSampleRate * 0.010));
    initialiseSmoother(flutterDepthValue, flutter * flutter * static_cast<float>(currentSampleRate * 0.0015));

    const auto lowCut = load(raw.lowCut);
    const auto highCut = load(raw.highCut);
    const auto age = juce::jlimit(0.0f, 1.0f, load(raw.age) * 0.01f);
    const float averageDelayMs = (initialDelay[0] + initialDelay[1])
                               * 500.0f / static_cast<float>(currentSampleRate);
    const float tapeDelayWear = std::sqrt(juce::jlimit(0.0f, 1.0f,
                                                       averageDelayMs / 2000.0f));
    const float tapeWear = juce::jlimit(0.0f, 1.0f, age * 0.80f + tapeDelayWear * 0.20f);
    const auto tapeLowpassHz = juce::jmap(tapeWear, 12500.0f, 3200.0f);
    const auto tapeHighpassHz = juce::jmap(age, 35.0f, 180.0f);
    const float tapeIIWear = juce::jlimit(
        0.0f, 1.0f, age * 0.72f + tapeDelayWear * 0.18f);
    const auto tapeIILowpassHz = juce::jmap(
        tapeIIWear, 15500.0f, 3600.0f);
    const auto tapeIIHighpassHz = juce::jmap(age, 25.0f, 145.0f);
    const float chromeTapeWear = juce::jlimit(
        0.0f, 1.0f, age * 0.90f + tapeDelayWear * 0.10f);
    const auto chromeTapeLowpassHz = juce::jmap(
        chromeTapeWear, 19000.0f, 3800.0f);
    const auto chromeTapeHighpassHz = juce::jmap(age, 20.0f, 160.0f);
    const auto hifiLowpassHz = juce::jmap(age, 19500.0f, 11500.0f);
    const auto hifiHighpassHz = juce::jmap(age, 18.0f, 65.0f);
    const auto ghostLowpassHz = juce::jmap(age, 17000.0f, 8500.0f);
    const auto ghostHighpassHz = juce::jmap(age, 25.0f, 120.0f);
    const float lofiWear = 0.30f + age * 0.70f;
    const auto lofiLowpassHz = juce::jmap(lofiWear, 9000.0f, 2400.0f);
    const auto lofiHighpassHz = juce::jmap(lofiWear, 45.0f, 170.0f);
    const auto crystalAgeLowpassHz = juce::jmap(age * age, 20000.0f, 4200.0f);
    initialiseSmoother(lowpassCoefficientValue, onePoleCoefficient(highCut, currentSampleRate));
    initialiseSmoother(highpassCoefficientValue, onePoleCoefficient(lowCut, currentSampleRate));
    initialiseSmoother(tapeLowpassCoefficientValue, onePoleCoefficient(tapeLowpassHz, currentSampleRate));
    initialiseSmoother(tapeHighpassCoefficientValue, onePoleCoefficient(tapeHighpassHz, currentSampleRate));
    initialiseSmoother(tapeIILowpassCoefficientValue,
                       onePoleCoefficient(tapeIILowpassHz, currentSampleRate));
    initialiseSmoother(tapeIIHighpassCoefficientValue,
                       onePoleCoefficient(tapeIIHighpassHz, currentSampleRate));
    initialiseSmoother(chromeTapeLowpassCoefficientValue,
                       onePoleCoefficient(chromeTapeLowpassHz, currentSampleRate));
    initialiseSmoother(chromeTapeHighpassCoefficientValue,
                       onePoleCoefficient(chromeTapeHighpassHz, currentSampleRate));
    initialiseSmoother(hifiLowpassCoefficientValue, onePoleCoefficient(hifiLowpassHz, currentSampleRate));
    initialiseSmoother(hifiHighpassCoefficientValue, onePoleCoefficient(hifiHighpassHz, currentSampleRate));
    initialiseSmoother(ghostLowpassCoefficientValue, onePoleCoefficient(ghostLowpassHz, currentSampleRate));
    initialiseSmoother(ghostHighpassCoefficientValue, onePoleCoefficient(ghostHighpassHz, currentSampleRate));
    initialiseSmoother(lofiLowpassCoefficientValue, onePoleCoefficient(lofiLowpassHz, currentSampleRate));
    initialiseSmoother(lofiHighpassCoefficientValue, onePoleCoefficient(lofiHighpassHz, currentSampleRate));
    initialiseSmoother(crystalAgeLowpassCoefficientValue,
                       onePoleCoefficient(crystalAgeLowpassHz, currentSampleRate));
    initialiseSmoother(ageValue, age);

    for (int channel = 0; channel < delayChannelCount; ++channel)
    {
        const auto index = static_cast<size_t>(channel);
        const float delayMs = initialDelay[index] * 1000.0f / static_cast<float>(currentSampleRate);
        const float delayWear = std::sqrt(juce::jlimit(0.0f, 1.0f, delayMs / 2000.0f));
        const float bbdWear = juce::jlimit(0.0f, 1.0f,
                                           0.18f + age * 0.62f + delayWear * 0.20f);
        const float bbdLowpassHz = juce::jmap(bbdWear, 11000.0f, 2200.0f);
        const float bbdHighpassHz = juce::jmap(age, 35.0f, 180.0f);
        const float analogWear = juce::jlimit(
            0.0f, 1.0f, 0.08f + age * 0.55f + delayWear * 0.15f);
        const float analogLowpassHz = juce::jmap(
            analogWear, 19000.0f, 4400.0f);
        const float analogHighpassHz = juce::jmap(age, 25.0f, 120.0f);
        initialiseSmoother(bbdLowpassCoefficientValue[index],
                           onePoleCoefficient(bbdLowpassHz, currentSampleRate));
        initialiseSmoother(bbdHighpassCoefficientValue[index],
                           onePoleCoefficient(bbdHighpassHz, currentSampleRate));
        initialiseSmoother(analogLowpassCoefficientValue[index],
                           onePoleCoefficient(analogLowpassHz, currentSampleRate));
        initialiseSmoother(analogHighpassCoefficientValue[index],
                           onePoleCoefficient(analogHighpassHz, currentSampleRate));
    }

    lowpassState.fill(0.0f);
    highpassState.fill(0.0f);
    highpassInput.fill(0.0f);
    tapeLowpassState.fill(0.0f);
    tapeLowpassState2.fill(0.0f);
    tapeHighpassState.fill(0.0f);
    tapeHighpassInput.fill(0.0f);
    resetTapeIIState();
    chromeTapeLowpassState.fill(0.0f);
    chromeTapeLowpassState2.fill(0.0f);
    chromeTapeHighpassState.fill(0.0f);
    chromeTapeHighpassInput.fill(0.0f);
    bbdLowpassState.fill(0.0f);
    bbdLowpassState2.fill(0.0f);
    bbdHighpassState.fill(0.0f);
    bbdHighpassInput.fill(0.0f);
    analogLowpassState.fill(0.0f);
    analogLowpassState2.fill(0.0f);
    analogHighpassState.fill(0.0f);
    analogHighpassInput.fill(0.0f);
    hifiLowpassState.fill(0.0f);
    hifiHighpassState.fill(0.0f);
    hifiHighpassInput.fill(0.0f);
    resetGhostState();
    resetLoFiState();
    digitalTriangleState.fill(0.0f);
    multiTapDigitalTriangleState = {};
    resetCrystalState();
    resetPhaserState();
    resetGranularModeState();
    resetDistortionState();
    multiTapLowpassState.fill(0.0f);
    multiTapLowpassState2.fill(0.0f);
    multiTapHighpassState.fill(0.0f);
    multiTapHighpassInput.fill(0.0f);
    bbdNoiseState = { 0x9e3779b9u, 0x243f6a88u };
    analogNoiseState = { 0xd3a2646cu, 0xfd7046c5u };
    hifiNoiseState = { 0xb7e15162u, 0x8aed2a6bu };
    wowPhase = 0.0;
    flutterPhase = 0.0;
    modulationPhase = 0.0;
    crystalPhase = 0.0;
    diffusionPhaseA = 0.0;
    diffusionPhaseB = 0.37;
    pitchPhase = { 0.0f, 0.0f };
    reversePitchPhase = { 0.0f, 0.0f };
    lushPitchShiftPhase = { 0.0f, 0.0f };
    lushPitchDetunePhase = { 0.25f, 0.75f };
    multiTapPitchPhase = {};
    multiTapReversePitchPhase = {};
    multiTapLushShiftPhase = {};
    multiTapLushDetunePhase = {{
        { 0.25f, 0.75f }, { 0.25f, 0.75f },
        { 0.25f, 0.75f }, { 0.25f, 0.75f }
    }};
    multiTapNoiseState =
    {
        0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au
    };
    pitchWindowSamples = juce::jlimit(
        64.0f, static_cast<float>(juce::jmax(64, delayBufferLength - 4)),
        static_cast<float>(currentSampleRate) * pitchWindowSeconds);
    const float initialPitchShift = juce::jlimit(
        -maximumPitchShiftSemitones, maximumPitchShiftSemitones,
        load(raw.pitchShift));
    const float initialPitchDetune = juce::jlimit(
        -maximumPitchDetuneSemitones, maximumPitchDetuneSemitones,
        load(raw.pitchDetune));
    initialiseSmoother(pitchRatioL, pitchRatioForSemitones(
        initialPitchShift - 0.5f * initialPitchDetune));
    initialiseSmoother(pitchRatioR, pitchRatioForSemitones(
        initialPitchShift + 0.5f * initialPitchDetune));
    initialiseSmoother(detuneRatioL, pitchRatioForSemitones(
        -0.5f * initialPitchDetune));
    initialiseSmoother(detuneRatioR, pitchRatioForSemitones(
        0.5f * initialPitchDetune));
    updateGhostOscillatorIncrements(load(raw.ghostShift),
                                    load(raw.ghostDetune));
    duckEnvelope = 0.0f;
}

void AfterglowAudioProcessor::releaseResources()
{
    delayBuffer.setSize(0, 0);
    for (auto& stageBuffer : diffusionStageBuffer)
        stageBuffer.setSize(0, 0);
    freezeBuffer.setSize(0, 0);
    writePosition = 0;
    diffusionWritePosition.fill(0);
    freezeCaptureWritePosition = 0;
    freezeCaptureValidSamples = 0;
    freezeLoopStart.fill(0);
    freezeReadPosition.fill(0);
    freezeLoopLength.fill(1);
    freezePlaybackActive = false;
    pitchPhase = { 0.0f, 0.0f };
    reversePitchPhase = { 0.0f, 0.0f };
    lushPitchShiftPhase = { 0.0f, 0.0f };
    lushPitchDetunePhase = { 0.25f, 0.75f };
    resetGhostState();
    resetLoFiState();
    digitalTriangleState.fill(0.0f);
    multiTapDigitalTriangleState = {};
    resetCrystalState();
    resetPhaserState();
    resetGranularModeState();
    resetTapeIIState();
    resetDistortionState();
    multiTapLowpassState.fill(0.0f);
    multiTapLowpassState2.fill(0.0f);
    multiTapHighpassState.fill(0.0f);
    multiTapHighpassInput.fill(0.0f);
    multiTapPitchPhase = {};
    multiTapReversePitchPhase = {};
    multiTapLushShiftPhase = {};
    multiTapLushDetunePhase = {{
        { 0.25f, 0.75f }, { 0.25f, 0.75f },
        { 0.25f, 0.75f }, { 0.25f, 0.75f }
    }};
    duckEnvelope = 0.0f;
}

double AfterglowAudioProcessor::getTailLengthSeconds() const
{
    if (load(raw.freeze) > 0.5f)
        return 1200.0;

    const auto feedback = feedbackGainForPercentage(load(raw.feedback));
    double longestFreeTime = (load(raw.link) > 0.5f
                                ? load(raw.timeL)
                                : juce::jmax(load(raw.timeL), load(raw.timeR)))
                           / 1000.0;
    const bool linked = load(raw.link) > 0.5f;
    bool anySynced = load(raw.syncL) > 0.5f
                  || (! linked && load(raw.syncR) > 0.5f);
    if (load(raw.multiTap) > 0.5f)
    {
        longestFreeTime = 0.001;
        for (size_t tap = 0; tap < multiTapCount; ++tap)
            if (load(raw.tapEnabled[tap]) > 0.5f)
                longestFreeTime = juce::jmax(
                    longestFreeTime,
                    static_cast<double>(load(raw.tapTime[tap]) * 0.001f));
        anySynced = load(raw.multiTapSync) > 0.5f;
    }
    const auto longestDelay = anySynced
                            ? maximumDelaySeconds
                            : juce::jlimit(0.001, maximumDelaySeconds, longestFreeTime);

    if (feedback >= 0.999f)
        return 1200.0;

    double feedbackTail = longestDelay;
    if (feedback > 0.0001f)
    {
        const auto repeatsToMinus60Db = std::log(0.001)
            / std::log(static_cast<double>(feedback));
        feedbackTail *= repeatsToMinus60Db + 1.0;
    }

    double diffusionTail = 0.0;
    const float diffusionAmount = juce::jlimit(
        0.0f, 1.0f, load(raw.diffusionAmount) * 0.01f);
    if (diffusionAmount >= 0.01f)
    {
        const float coefficient = diffusionCoefficientForAmount(
            diffusionAmount);
        const double diffusionRepeats = std::log(0.001)
            / std::log(static_cast<double>(coefficient));
        const double sizeFraction = minimumDiffusionSizeFraction
            + (1.0 - minimumDiffusionSizeFraction)
                * juce::jlimit(0.0f, 1.0f,
                               load(raw.diffusionSize) * 0.01f);
        constexpr double longestDiffusionStageFraction = 0.397;
        diffusionTail = longestDelay * sizeFraction
                      * longestDiffusionStageFraction
                      * (diffusionRepeats + 1.0);
    }

    return juce::jlimit(0.0, 1200.0, feedbackTail + diffusionTail);
}

void AfterglowAudioProcessor::setDigitalDelayTarget(int channel, float delaySamples)
{
    auto& state = digitalDelay[static_cast<size_t>(channel)];
    state.requestedDelaySamples = delaySamples;

    if (state.crossfade < 1.0f
        || std::abs(state.requestedDelaySamples - state.targetDelaySamples) < 0.5f)
        return;

    state.previousDelaySamples = state.targetDelaySamples;
    state.targetDelaySamples = state.requestedDelaySamples;
    state.crossfade = 0.0f;
}

float AfterglowAudioProcessor::advanceTapeIITransport(
    TapeTransportState& state, float targetDelaySamples) noexcept
{
    const float maximumDelaySamples = static_cast<float>(
        juce::jmax(1, delayBuffer.getNumSamples() - 4));
    const float target = juce::jlimit(
        1.0f, maximumDelaySamples, targetDelaySamples);
    if (! std::isfinite(state.delaySamples)
        || ! std::isfinite(state.velocitySamplesPerSecond))
    {
        state.delaySamples = target;
        state.velocitySamplesPerSecond = 0.0f;
        return target;
    }

    const float sampleRate = static_cast<float>(
        juce::jmax(1.0, currentSampleRate));
    const float deltaTime = 1.0f / sampleRate;
    constexpr float transportFrequencyHz = 1.30f;
    constexpr float dampingRatio = 0.92f;
    const float angularFrequency = juce::MathConstants<float>::twoPi
                                 * transportFrequencyHz;
    const float error = target - state.delaySamples;
    const float acceleration = error * angularFrequency * angularFrequency
        - 2.0f * dampingRatio * angularFrequency
            * state.velocitySamplesPerSecond;
    state.velocitySamplesPerSecond += acceleration * deltaTime;

    // A moving playback head must not outrun the delay write head. Bounding
    // transport speed also gives large Time moves a physical acceleration,
    // glide and deceleration instead of a dry linear pitch ramp.
    const float maximumTransportSpeed = sampleRate * 0.55f;
    state.velocitySamplesPerSecond = juce::jlimit(
        -maximumTransportSpeed, maximumTransportSpeed,
        state.velocitySamplesPerSecond);
    state.delaySamples += state.velocitySamplesPerSecond * deltaTime;

    if (state.delaySamples <= 1.0f
        || state.delaySamples >= maximumDelaySamples)
    {
        state.delaySamples = juce::jlimit(
            1.0f, maximumDelaySamples, state.delaySamples);
        state.velocitySamplesPerSecond = 0.0f;
    }
    else if (std::abs(error) < 0.01f
             && std::abs(state.velocitySamplesPerSecond) < 0.01f)
    {
        state.delaySamples = target;
        state.velocitySamplesPerSecond = 0.0f;
    }

    return state.delaySamples;
}

void AfterglowAudioProcessor::resetTapeIIState() noexcept
{
    const float maximumDelaySamples = static_cast<float>(
        juce::jmax(1, delayBuffer.getNumSamples() - 4));
    const std::array<float, delayChannelCount> stereoDelay
    {
        tapeDelayL.getCurrentValue(), tapeDelayR.getCurrentValue()
    };
    for (size_t channel = 0; channel < tapeIITransport.size(); ++channel)
    {
        tapeIITransport[channel].delaySamples = juce::jlimit(
            1.0f, maximumDelaySamples, stereoDelay[channel]);
        tapeIITransport[channel].velocitySamplesPerSecond = 0.0f;
    }
    for (size_t tap = 0; tap < multiTapTapeIITransport.size(); ++tap)
    {
        multiTapTapeIITransport[tap].delaySamples = juce::jlimit(
            1.0f, maximumDelaySamples,
            multiTapDelayValue[tap].getCurrentValue());
        multiTapTapeIITransport[tap].velocitySamplesPerSecond = 0.0f;
    }

    tapeIILowpassState.fill(0.0f);
    tapeIILowpassState2.fill(0.0f);
    tapeIIHighpassState.fill(0.0f);
    tapeIIHighpassInput.fill(0.0f);
}

void AfterglowAudioProcessor::resetProcessingState() noexcept
{
    // Presets describe parameters, not the audio that happened to be in the
    // previous preset's feedback network.  Clear every recirculating and
    // stateful path together on the audio thread so loading a preset cannot
    // inherit a stale tail or briefly self-oscillate at the new settings.
    delayBuffer.clear();
    for (auto& stageBuffer : diffusionStageBuffer)
        stageBuffer.clear();
    freezeBuffer.clear();
    writePosition = 0;
    diffusionWritePosition.fill(0);
    freezeCaptureWritePosition = 0;
    freezeCaptureValidSamples = 0;
    freezeLoopStart.fill(0);
    freezeReadPosition.fill(0);
    freezeLoopLength.fill(1);
    freezePlaybackActive = false;
    sequencerFreePhase = 0.0;
    sequencerHeldNotes = 0;
    sequencerPhraseTriggered = false;
    previousSequencerCanonical = 1.0f;
    activeSequencerStep.store(-1, std::memory_order_relaxed);

    lowpassState.fill(0.0f);
    highpassState.fill(0.0f);
    highpassInput.fill(0.0f);
    tapeLowpassState.fill(0.0f);
    tapeLowpassState2.fill(0.0f);
    tapeHighpassState.fill(0.0f);
    tapeHighpassInput.fill(0.0f);
    resetTapeIIState();
    chromeTapeLowpassState.fill(0.0f);
    chromeTapeLowpassState2.fill(0.0f);
    chromeTapeHighpassState.fill(0.0f);
    chromeTapeHighpassInput.fill(0.0f);
    bbdLowpassState.fill(0.0f);
    bbdLowpassState2.fill(0.0f);
    bbdHighpassState.fill(0.0f);
    bbdHighpassInput.fill(0.0f);
    analogLowpassState.fill(0.0f);
    analogLowpassState2.fill(0.0f);
    analogHighpassState.fill(0.0f);
    analogHighpassInput.fill(0.0f);
    hifiLowpassState.fill(0.0f);
    hifiHighpassState.fill(0.0f);
    hifiHighpassInput.fill(0.0f);
    resetGhostState();
    resetLoFiState();
    digitalTriangleState.fill(0.0f);
    multiTapDigitalTriangleState = {};
    resetCrystalState();
    resetPhaserState();
    resetGranularModeState();
    resetDistortionState();

    multiTapLowpassState.fill(0.0f);
    multiTapLowpassState2.fill(0.0f);
    multiTapHighpassState.fill(0.0f);
    multiTapHighpassInput.fill(0.0f);
    bbdNoiseState = { 0x9e3779b9u, 0x243f6a88u };
    analogNoiseState = { 0xd3a2646cu, 0xfd7046c5u };
    hifiNoiseState = { 0xb7e15162u, 0x8aed2a6bu };
    multiTapNoiseState =
    {
        0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au
    };

    wowPhase = 0.0;
    flutterPhase = 0.0;
    modulationPhase = 0.0;
    diffusionPhaseA = 0.0;
    diffusionPhaseB = 0.37;
    pitchPhase = { 0.0f, 0.0f };
    reversePitchPhase = { 0.0f, 0.0f };
    lushPitchShiftPhase = { 0.0f, 0.0f };
    lushPitchDetunePhase = { 0.25f, 0.75f };
    multiTapPitchPhase = {};
    multiTapReversePitchPhase = {};
    multiTapLushShiftPhase = {};
    multiTapLushDetunePhase = {{
        { 0.25f, 0.75f }, { 0.25f, 0.75f },
        { 0.25f, 0.75f }, { 0.25f, 0.75f }
    }};
    duckEnvelope = 0.0f;
}

float AfterglowAudioProcessor::readDelaySample(int channel, float delaySamples) const
{
    const int length = delayBuffer.getNumSamples();
    if (length < 8)
        return 0.0f;

    delaySamples = juce::jlimit(1.0f, static_cast<float>(length - 4), delaySamples);
    float position = static_cast<float>(writePosition) - delaySamples;
    while (position < 0.0f)
        position += static_cast<float>(length);

    const int index0 = static_cast<int>(position);
    const int index1 = (index0 + 1) % length;
    const float fraction = position - static_cast<float>(index0);
    const auto* data = delayBuffer.getReadPointer(channel);
    return data[index0] + (data[index1] - data[index0]) * fraction;
}

float AfterglowAudioProcessor::readDigitalDelay(int channel, float modulationSamples)
{
    auto& state = digitalDelay[static_cast<size_t>(channel)];
    if (state.crossfade >= 1.0f)
        return readDelaySample(channel, state.targetDelaySamples + modulationSamples);

    const float progress = juce::jlimit(0.0f, 1.0f, state.crossfade);
    const float smoothProgress = progress * progress * (3.0f - 2.0f * progress);
    const float previous = readDelaySample(channel, state.previousDelaySamples + modulationSamples);
    const float target = readDelaySample(channel, state.targetDelaySamples + modulationSamples);
    state.crossfade = juce::jmin(1.0f, state.crossfade + digitalCrossfadeStep);

    if (state.crossfade >= 1.0f)
        state.previousDelaySamples = state.targetDelaySamples;

    return previous + (target - previous) * smoothProgress;
}

float AfterglowAudioProcessor::readPitchDelay(int channel, float delaySamples,
                                               float pitchRatio) noexcept
{
    return readPitchDelayWithPhase(channel, delaySamples, pitchRatio, pitchPhase);
}

float AfterglowAudioProcessor::readPitchDelayWithPhase(
    int channel, float delaySamples, float pitchRatio,
    std::array<float, delayChannelCount>& phaseState) noexcept
{
    const auto channelIndex = static_cast<size_t>(juce::jlimit(
        0, delayChannelCount - 1, channel));
    const float phaseA = phaseState[channelIndex];
    float phaseB = phaseA + 0.5f;
    if (phaseB >= 1.0f)
        phaseB -= 1.0f;

    const float gainA = 0.5f - 0.5f * std::cos(
        juce::MathConstants<float>::twoPi * phaseA);
    const float gainB = 0.5f - 0.5f * std::cos(
        juce::MathConstants<float>::twoPi * phaseB);
    const float sampleA = readDelaySample(
        channel, delaySamples + (1.0f - phaseA) * pitchWindowSamples);
    const float sampleB = readDelaySample(
        channel, delaySamples + (1.0f - phaseB) * pitchWindowSamples);
    const float gainSum = juce::jmax(0.001f, gainA + gainB);

    const float safePitchRatio = juce::jlimit(0.25f, 4.0f, pitchRatio);
    const float phaseIncrement = (safePitchRatio - 1.0f)
                               / juce::jmax(1.0f, pitchWindowSamples);
    float nextPhase = phaseA + phaseIncrement;
    if (nextPhase >= 1.0f)
        nextPhase -= 1.0f;
    else if (nextPhase < 0.0f)
        nextPhase += 1.0f;
    phaseState[channelIndex] = nextPhase;

    const float shifted = (sampleA * gainA + sampleB * gainB) / gainSum;
    const float direct = readDelaySample(channel, delaySamples);
    const float ratioDistance = std::abs(safePitchRatio - 1.0f);
    const float shiftBlend = juce::jlimit(0.0f, 1.0f, ratioDistance * 100.0f);
    return sanitiseSample(direct + (shifted - direct) * shiftBlend, 16.0f);
}

float AfterglowAudioProcessor::readReversePitchDelay(
    int channel, float spliceSamples, float pitchRatio) noexcept
{
    return readReversePitchDelayWithPhase(
        channel, spliceSamples, pitchRatio, reversePitchPhase, true);
}

float AfterglowAudioProcessor::readReversePitchDelayWithPhase(
    int channel, float spliceSamples, float pitchRatio,
    std::array<float, delayChannelCount>& phaseState,
    bool emphasiseShortSplices) noexcept
{
    const auto channelIndex = static_cast<size_t>(juce::jlimit(
        0, delayChannelCount - 1, channel));
    const float maximumSplice = static_cast<float>(
        juce::jmax(32, delayBuffer.getNumSamples() - 4));
    const float safeSplice = juce::jlimit(32.0f, maximumSplice,
                                          spliceSamples);
    const float safePitchRatio = juce::jlimit(0.25f, 4.0f, pitchRatio);
    const float phaseA = phaseState[channelIndex];
    float phaseB = phaseA + 0.5f;
    if (phaseB >= 1.0f)
        phaseB -= 1.0f;

    float gainA = 0.5f - 0.5f * std::cos(
        juce::MathConstants<float>::twoPi * phaseA);
    float gainB = 0.5f - 0.5f * std::cos(
        juce::MathConstants<float>::twoPi * phaseB);
    if (emphasiseShortSplices)
    {
        // RevPitch's Delay controls are splice sizes, not conventional tap
        // offsets.  A permanently broad Hann overlap made sub-40 ms splices
        // sound too smooth and hid the deliberate granular articulation of
        // very short settings.  Progressively focus only those windows while
        // retaining zero-valued boundaries, constant overlap gain and the
        // existing click-safe behaviour at normal delay lengths.
        const float fullFocusSamples = static_cast<float>(currentSampleRate)
                                     * 0.012f;
        const float noFocusSamples = static_cast<float>(currentSampleRate)
                                   * 0.040f;
        const float shortSpliceAmount = 1.0f - juce::jlimit(
            0.0f, 1.0f,
            (safeSplice - fullFocusSamples)
                / juce::jmax(1.0f, noFocusSamples - fullFocusSamples));
        const float pitchDistance = juce::jlimit(
            0.0f, 1.0f,
            std::abs(std::log2(safePitchRatio)) * 0.5f);
        const float windowExponent = 1.0f + shortSpliceAmount
            * (0.90f + pitchDistance * 1.10f);
        gainA = std::pow(juce::jmax(0.0f, gainA), windowExponent);
        gainB = std::pow(juce::jmax(0.0f, gainB), windowExponent);
    }
    const float delayA = 1.0f + phaseA * (safeSplice - 1.0f);
    const float delayB = 1.0f + phaseB * (safeSplice - 1.0f);
    const float sampleA = readDelaySample(channel, delayA);
    const float sampleB = readDelaySample(channel, delayB);
    const float gainSum = juce::jmax(0.001f, gainA + gainB);

    const float phaseIncrement = (1.0f + safePitchRatio)
                               / juce::jmax(1.0f, safeSplice - 1.0f);
    float nextPhase = phaseA + phaseIncrement;
    if (nextPhase >= 1.0f)
        nextPhase -= 1.0f;
    phaseState[channelIndex] = nextPhase;

    return sanitiseSample((sampleA * gainA + sampleB * gainB) / gainSum,
                          16.0f);
}

float AfterglowAudioProcessor::processDiffusion(float sample, int channel,
                                                 float amount,
                                                 float spanSamples,
                                                 int bank) noexcept
{
    if (channel < 0 || channel >= delayChannelCount
        || bank < 0 || bank >= diffusionBankCount
        || amount < 0.01f || spanSamples < 4.0f)
        return sample;

    const float safeAmount = diffusionCoefficientForAmount(amount);
    const float safeSpan = juce::jlimit(
        4.0f,
        static_cast<float>(currentSampleRate * maximumDiffusionSeconds),
        spanSamples);
    const float modulationDepth = juce::jmin(
        safeSpan * 0.0025f,
        static_cast<float>(currentSampleRate
                           * maximumDiffusionModulationSeconds))
        * safeAmount * safeAmount;
    const double channelPhase = channel == 0 ? 0.0 : 0.173;
    const float motionA = static_cast<float>(std::sin(
        juce::MathConstants<double>::twoPi
            * (diffusionPhaseA + channelPhase))) * modulationDepth;
    const float motionB = static_cast<float>(std::sin(
        juce::MathConstants<double>::twoPi
            * (diffusionPhaseB - channelPhase))) * modulationDepth;
    float stageInput = sanitiseSample(sample, 16.0f);

    for (int stage = 0; stage < diffusionStageCount; ++stage)
    {
        const auto stageIndex = static_cast<size_t>(stage);
        auto& stageBuffer = diffusionStageBuffer[stageIndex];
        const int bufferLength = stageBuffer.getNumSamples();
        if (bufferLength < 4)
            return sample;

        const float pairedMotion = stage < 2 ? motionA : motionB;
        const float signedMotion = (stage & 1) == 0
                                 ? pairedMotion : -pairedMotion;
        const float delaySamples = juce::jlimit(
            1.0f, static_cast<float>(bufferLength - 2),
            safeSpan
                * diffusionStageFractions[static_cast<size_t>(channel)]
                                         [stageIndex]
                + signedMotion);
        float readPosition = static_cast<float>(
            diffusionWritePosition[stageIndex]) - delaySamples;
        if (readPosition < 0.0f)
            readPosition += static_cast<float>(bufferLength);

        const int indexA = static_cast<int>(readPosition);
        const int indexB = indexA + 1 < bufferLength ? indexA + 1 : 0;
        const float fraction = readPosition - static_cast<float>(indexA);
        const int bufferChannel = bank * delayChannelCount + channel;
        const float delayedA = stageBuffer.getSample(bufferChannel, indexA);
        const float delayedB = stageBuffer.getSample(bufferChannel, indexB);
        const float delayed = delayedA + fraction * (delayedB - delayedA);
        constexpr std::array<float, diffusionStageCount> coefficientScale
        {
            0.96f, 1.00f, 0.98f, 0.94f
        };
        const float coefficient = juce::jmin(
            0.97f, safeAmount * coefficientScale[stageIndex]);
        const float stageOutput = delayed - coefficient * stageInput;
        const float stageWrite = stageInput + coefficient * stageOutput;
        stageBuffer.setSample(bufferChannel,
                              diffusionWritePosition[stageIndex],
                              sanitiseSample(stageWrite, 16.0f));
        stageInput = sanitiseSample(stageOutput, 16.0f);
    }

    return stageInput;
}

float AfterglowAudioProcessor::nextNoiseSample(uint32_t& state) noexcept
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return static_cast<float>(state & 0x00ffffffu) * (2.0f / 16777215.0f) - 1.0f;
}

float AfterglowAudioProcessor::processAllpass(float sample, float coefficient,
                                               AllpassState& state) noexcept
{
    const float output = coefficient * sample + state.previousInput
                       - coefficient * state.previousOutput;
    state.previousInput = sample;
    state.previousOutput = output;
    return output;
}

float AfterglowAudioProcessor::processGhostFrequencyShift(float sample, int channel) noexcept
{
    const auto channelIndex = static_cast<size_t>(juce::jlimit(
        0, delayChannelCount - 1, channel));
    return processGhostFrequencyShift(sample, channel,
                                      ghostHilbertA[channelIndex],
                                      ghostHilbertB[channelIndex]);
}

float AfterglowAudioProcessor::processGhostFrequencyShift(
    float sample, int channel, HilbertPath& hilbertA,
    HilbertPath& hilbertB) noexcept
{
    static constexpr std::array<float, ghostHilbertStages> coefficientsA
    {
        0.16175850f, 0.73302895f, 0.94534970f, 0.99059916f
    };
    static constexpr std::array<float, ghostHilbertStages> coefficientsB
    {
        0.47940087f, 0.87621849f, 0.97659759f, 0.99749926f
    };

    const auto channelIndex = static_cast<size_t>(juce::jlimit(0, delayChannelCount - 1,
                                                               channel));
    float quadratureA = sample;
    float quadratureB = sample;
    for (size_t stage = 0; stage < ghostHilbertStages; ++stage)
    {
        quadratureA = processAllpass(quadratureA, coefficientsA[stage],
                                     hilbertA[stage]);
        quadratureB = processAllpass(quadratureB, coefficientsB[stage],
                                     hilbertB[stage]);
    }

    constexpr float quadratureGain = 0.70710678f;
    const float shifted = quadratureA * ghostOscillatorCos[channelIndex]
                        - quadratureB * ghostOscillatorSin[channelIndex];
    return shifted * quadratureGain;
}

void AfterglowAudioProcessor::updateGhostOscillatorIncrements(
    float shiftHz, float detuneHz) noexcept
{
    shiftHz = juce::jlimit(
        -maximumGhostShiftHz, maximumGhostShiftHz, shiftHz);
    detuneHz = juce::jlimit(
        0.0f, maximumGhostDetuneHz, detuneHz);
    const float maximumFrequency = static_cast<float>(
        juce::jmax(1.0, currentSampleRate * 0.45));

    for (int channel = 0; channel < delayChannelCount; ++channel)
    {
        const auto index = static_cast<size_t>(channel);
        const float detuneOffset = channel == 0 ? detuneHz * 0.5f
                                                : -detuneHz * 0.5f;
        const float frequency = juce::jlimit(
            -maximumFrequency, maximumFrequency, shiftHz + detuneOffset);
        const float increment = juce::MathConstants<float>::twoPi * frequency
                              / static_cast<float>(currentSampleRate);
        ghostOscillatorSinIncrement[index] = std::sin(increment);
        ghostOscillatorCosIncrement[index] = std::cos(increment);
    }
}

void AfterglowAudioProcessor::advanceGhostOscillators() noexcept
{
    for (size_t channel = 0; channel < ghostOscillatorSin.size(); ++channel)
    {
        const float nextSin = ghostOscillatorSin[channel]
                                * ghostOscillatorCosIncrement[channel]
                            + ghostOscillatorCos[channel]
                                * ghostOscillatorSinIncrement[channel];
        const float nextCos = ghostOscillatorCos[channel]
                                * ghostOscillatorCosIncrement[channel]
                            - ghostOscillatorSin[channel]
                                * ghostOscillatorSinIncrement[channel];
        ghostOscillatorSin[channel] = nextSin;
        ghostOscillatorCos[channel] = nextCos;

        if (++ghostOscillatorSamples[channel] < 4096u)
            continue;

        const float magnitude = std::sqrt(nextSin * nextSin
                                        + nextCos * nextCos);
        if (magnitude > 0.0001f)
        {
            ghostOscillatorSin[channel] /= magnitude;
            ghostOscillatorCos[channel] /= magnitude;
        }
        else
        {
            ghostOscillatorSin[channel] = 0.0f;
            ghostOscillatorCos[channel] = 1.0f;
        }
        ghostOscillatorSamples[channel] = 0;
    }
}

void AfterglowAudioProcessor::resetGhostState() noexcept
{
    ghostLowpassState.fill(0.0f);
    ghostHighpassState.fill(0.0f);
    ghostHighpassInput.fill(0.0f);
    for (auto& path : ghostHilbertA)
        path.fill({});
    for (auto& path : ghostHilbertB)
        path.fill({});
    for (auto& tap : multiTapGhostHilbertA)
        for (auto& path : tap)
            path.fill({});
    for (auto& tap : multiTapGhostHilbertB)
        for (auto& path : tap)
            path.fill({});
    ghostOscillatorSin.fill(0.0f);
    ghostOscillatorCos.fill(1.0f);
    ghostOscillatorSamples.fill(0);
}

void AfterglowAudioProcessor::resetLoFiState() noexcept
{
    lofiLowpassState.fill(0.0f);
    lofiLowpassState2.fill(0.0f);
    lofiHighpassState.fill(0.0f);
    lofiHighpassInput.fill(0.0f);
    lofiNoiseState = { 0x85ebca6bu, 0xc2b2ae35u };
    lofiSlowModulation = {};
    lofiScrapeModulation = {};
    lofiSlowModulation[0].noiseState = 0x27d4eb2du;
    lofiSlowModulation[1].noiseState = 0x165667b1u;
    lofiScrapeModulation[0].noiseState = 0xd3a2646cu;
    lofiScrapeModulation[1].noiseState = 0xfd7046c5u;
}

void AfterglowAudioProcessor::resetCrystalState() noexcept
{
    crystalLowpassState.fill(0.0f);
    crystalHighpassState.fill(0.0f);
    crystalHighpassInput.fill(0.0f);
    for (auto& channel : crystalAgeLowpassState)
        channel.fill(0.0f);
    crystalTriangleState.fill(0.0f);
    multiTapCrystalTriangleState = {};
    crystalPhase = 0.0;
}

void AfterglowAudioProcessor::resetPhaserState() noexcept
{
    for (auto& channel : phaserAllpassState)
        channel.fill({});
    phaserFeedbackState.fill(0.0f);
}

void AfterglowAudioProcessor::resetGranularModeState() noexcept
{
    pureReversePhase.fill(0.0f);
    reverseForwardPhase.fill(0.0f);
    chaosPhase.fill(0.0f);
    reverseChaosPhase.fill(0.0f);
    multiTapPureReversePhase = {};
    multiTapReverseForwardPhase = {};
    multiTapChaosPhase = {};
    multiTapReverseChaosPhase = {};
    chaosTimeModulation = {};
    chaosPitchModulation = {};
    chaosTimeModulation[0].noiseState = 0x9e3779b9u;
    chaosTimeModulation[1].noiseState = 0x243f6a88u;
    chaosPitchModulation[0].noiseState = 0xb7e15162u;
    chaosPitchModulation[1].noiseState = 0x8aed2a6bu;
    for (auto& path : reverseForwardLowpassState)
        path.fill(0.0f);
    for (auto& path : reverseForwardHighpassState)
        path.fill(0.0f);
    for (auto& path : reverseForwardHighpassInput)
        path.fill(0.0f);
}

float AfterglowAudioProcessor::nextRandomModulation(RandomModulator& modulator,
                                                     float rateHz) noexcept
{
    if (modulator.samplesRemaining <= 0)
    {
        const float safeRate = juce::jlimit(0.02f, 8000.0f, rateHz);
        const float durationVariation = 0.70f
                                      + 0.60f * (nextNoiseSample(modulator.noiseState)
                                                 * 0.5f + 0.5f);
        const int duration = juce::jmax(
            1, juce::roundToInt(static_cast<float>(currentSampleRate)
                                / safeRate * durationVariation));
        modulator.target = nextNoiseSample(modulator.noiseState);
        modulator.step = (modulator.target - modulator.value)
                       / static_cast<float>(duration);
        modulator.samplesRemaining = duration;
    }

    modulator.value += modulator.step;
    --modulator.samplesRemaining;
    return modulator.value;
}

void AfterglowAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    const int sampleCount = buffer.getNumSamples();
    const int hostChannelCount = buffer.getNumChannels();
    const int activeChannelCount = juce::jmin(delayChannelCount, hostChannelCount);

    for (int channel = activeChannelCount; channel < hostChannelCount; ++channel)
        buffer.clear(channel, 0, sampleCount);

    if (sampleCount <= 0 || activeChannelCount <= 0 || delayBuffer.getNumSamples() < 8)
        return;

    if (pendingProcessingReset.exchange(false, std::memory_order_acq_rel))
        resetProcessingState();

    const bool sequenceEnabled = load(raw.sequencerEnabled) >= 0.5f;
    const bool sequenceProcessing = sequenceEnabled;
    const bool timesLinked = load(raw.link) > 0.5f;
    const bool leftSynced = load(raw.syncL) > 0.5f;
    const bool rightSynced = timesLinked
        ? leftSynced : load(raw.syncR) > 0.5f;
    const bool multiTapSynced = load(raw.multiTapSync) > 0.5f;
    const bool hostSync = load(raw.sequencerHostSync) >= 0.5f;
    const auto sequenceDirection
        = static_cast<afterglow::sequencer::Direction>(
            juce::jlimit(0, 4, static_cast<int>(std::lround(
                load(raw.sequencerDirection)))));

    juce::Optional<juce::AudioPlayHead::PositionInfo> hostPosition;
    if (leftSynced || rightSynced || multiTapSynced
        || (sequenceEnabled && hostSync
            && sequenceDirection
                != afterglow::sequencer::Direction::played))
        if (auto* playhead = getPlayHead())
            hostPosition = playhead->getPosition();

    double bpm = 120.0;
    if (hostPosition.hasValue())
        if (const auto hostBpm = hostPosition->getBpm())
            if (std::isfinite(*hostBpm) && *hostBpm > 0.0)
                bpm = juce::jlimit(10.0, 1000.0, *hostBpm);

    sequencerPhraseTriggered = false;
    if (sequenceEnabled
        && sequenceDirection == afterglow::sequencer::Direction::played)
    {
        for (const auto metadata : midi)
        {
            const auto message = metadata.getMessage();
            if (message.isNoteOn())
            {
                if (sequencerHeldNotes == 0)
                    sequencerPhraseTriggered = true;
                ++sequencerHeldNotes;
            }
            else if (message.isNoteOff())
            {
                sequencerHeldNotes = juce::jmax(0,
                                                sequencerHeldNotes - 1);
            }
            else if (message.isAllNotesOff() || message.isAllSoundOff())
            {
                sequencerHeldNotes = 0;
            }
        }
    }
    else
    {
        sequencerHeldNotes = 0;
    }

    std::array<float, afterglow::sequencer::stepCount> sequenceSteps {};
    float sequenceMinimum = 1.0f;
    float sequenceMaximum = 0.0f;
    if (sequenceProcessing)
    {
        for (int step = 0; step < afterglow::sequencer::stepCount; ++step)
        {
            const auto index = static_cast<size_t>(step);
            sequenceSteps[index] = juce::jlimit(
                0.0f, 1.0f, load(raw.sequencerSteps[index]));
            sequenceMinimum = juce::jmin(sequenceMinimum,
                                          sequenceSteps[index]);
            sequenceMaximum = juce::jmax(sequenceMaximum,
                                          sequenceSteps[index]);
        }
    }

    const bool sequenceBipolar = load(raw.sequencerBipolar) >= 0.5f;
    const auto modulatedSequenceControl =
        [this, sequenceProcessing, sequenceBipolar](
            int targetChoice, const std::atomic<float>* parameter)
        {
            const float base = load(parameter);
            return sequenceProcessing
                ? sequencerModulatedValue(
                      targetChoice, base, previousSequencerCanonical,
                      sequenceBipolar)
                : base;
        };
    const float sequencePeak = juce::jlimit(
        -1.0f, 1.0f, modulatedSequenceControl(
            afterglow::sequencer::sequencerPeakTargetChoice,
            raw.sequencerPeak));
    const float sequenceMove = juce::jlimit(
        -1.0f, 1.0f, modulatedSequenceControl(
            afterglow::sequencer::sequencerMoveTargetChoice,
            raw.sequencerMove));
    if (sequenceProcessing
        && (std::abs(sequencePeak) > 0.000001f
            || std::abs(sequenceMove) > 0.000001f))
    {
        for (auto& value : sequenceSteps)
            value = afterglow::sequencer::applyPeakMove(
                value, sequenceMinimum, sequenceMaximum,
                sequencePeak, sequenceMove, sequenceBipolar);
    }

    const int sequenceRate = juce::jlimit(
        0, afterglow::sequencer::rateChoiceCount - 1,
        static_cast<int>(std::lround(load(raw.sequencerRate))));
    const double beatsPerStep = afterglow::sequencer::beatsForRate(sequenceRate);
    const double phaseIncrement = bpm > 0.0
        ? bpm / (60.0 * beatsPerStep * currentSampleRate) : 0.0;
    if (sequenceEnabled
        && sequenceDirection == afterglow::sequencer::Direction::played
        && sequencerPhraseTriggered)
        sequencerFreePhase = 0.0;

    double sequencePhase = sequencerFreePhase
                         + 0.5 * static_cast<double>(sampleCount)
                             * phaseIncrement;
    if (sequenceEnabled && hostSync
        && sequenceDirection != afterglow::sequencer::Direction::played
        && hostPosition.hasValue())
        if (const auto ppq = hostPosition->getPpqPosition())
            if (std::isfinite(*ppq))
                sequencePhase = *ppq / beatsPerStep;

    if (sequenceEnabled)
        sequencerFreePhase += static_cast<double>(sampleCount) * phaseIncrement;
    if (sequenceEnabled && sequencerFreePhase > 1048576.0)
        sequencerFreePhase = std::fmod(
            sequencerFreePhase,
            static_cast<double>(afterglow::sequencer::stepCount * 8));

    const int firstSequenceStep = juce::jlimit(
        1, afterglow::sequencer::stepCount - 1,
        static_cast<int>(std::lround(load(raw.sequencerStart)))) - 1;
    const int lastSequenceStep = juce::jlimit(
        firstSequenceStep + 2, afterglow::sequencer::stepCount,
        static_cast<int>(std::lround(load(raw.sequencerEnd)))) - 1;
    int currentSequenceStep = -1;
    float sequenceCanonical = 1.0f;
    if (sequenceEnabled)
    {
        const auto position = afterglow::sequencer::positionForPhase(
            sequencePhase, lastSequenceStep - firstSequenceStep + 1,
            sequenceDirection);
        currentSequenceStep = firstSequenceStep + position.current;
    }
    if (sequenceProcessing)
    {
        const float sequenceAttack = juce::jlimit(
            0.0f, 1.0f, modulatedSequenceControl(
                afterglow::sequencer::sequencerAttackTargetChoice,
                raw.sequencerAttack));
        const float sequenceRelease = juce::jlimit(
            0.0f, 1.0f, modulatedSequenceControl(
                afterglow::sequencer::sequencerReleaseTargetChoice,
                raw.sequencerRelease));
        sequenceCanonical = afterglow::sequencer::evaluate(
            sequenceSteps, sequencePhase, firstSequenceStep,
            lastSequenceStep, sequenceDirection,
            sequenceAttack, sequenceRelease,
            &currentSequenceStep);
    }
    previousSequencerCanonical = sequenceCanonical;
    activeSequencerStep.store(currentSequenceStep, std::memory_order_relaxed);

    const auto sequenced = [this, sequenceCanonical, sequenceBipolar,
                            sequenceProcessing](
        int targetChoice, const std::atomic<float>* parameter)
    {
        if (! sequenceProcessing)
            return load(parameter);
        return sequencerModulatedValue(
            targetChoice, load(parameter), sequenceCanonical,
            sequenceBipolar);
    };
    const auto sequencedWithValueParameter
        = [this, sequenceCanonical, sequenceBipolar, sequenceProcessing](
            int targetChoice, const std::atomic<float>* parameter,
            juce::RangedAudioParameter* valueParameter)
    {
        if (! sequenceProcessing)
            return load(parameter);
        return sequencerModulatedValue(
            targetChoice, load(parameter), sequenceCanonical,
            sequenceBipolar, valueParameter);
    };

    float leftMs = sequenced(1, raw.timeL);
    float rightMs = timesLinked ? leftMs : sequenced(2, raw.timeR);

    if (leftSynced || rightSynced || multiTapSynced)
    {
        if (leftSynced)
        {
            const auto modulatedLeftDivision = sequencedWithValueParameter(
                1, raw.divisionL, sequencerTimeDivisionParameters[0]);
            const auto leftIndex = juce::jlimit(
                0, static_cast<int>(tempoDivisionBeats.size()) - 1,
                static_cast<int>(std::lround(modulatedLeftDivision)));
            leftMs = static_cast<float>((60000.0 / bpm)
                                        * tempoDivisionBeats[static_cast<size_t>(leftIndex)]);
        }

        if (timesLinked)
        {
            rightMs = leftMs;
        }
        else if (rightSynced)
        {
            const auto rightIndex = juce::jlimit(
                0, static_cast<int>(tempoDivisionBeats.size()) - 1,
                static_cast<int>(std::lround(sequencedWithValueParameter(
                    2, raw.divisionR, sequencerTimeDivisionParameters[1]))));
            rightMs = static_cast<float>((60000.0 / bpm)
                                         * tempoDivisionBeats[static_cast<size_t>(rightIndex)]);
        }
    }

    const int delayLength = delayBuffer.getNumSamples();
    const float maximumDelaySamples = static_cast<float>(delayLength - 4);
    const std::array<float, delayChannelCount> targetDelay
    {
        juce::jlimit(1.0f, maximumDelaySamples,
                     leftMs * static_cast<float>(currentSampleRate / 1000.0)),
        juce::jlimit(1.0f, maximumDelaySamples,
                     rightMs * static_cast<float>(currentSampleRate / 1000.0))
    };
    const int selectedMode = juce::jlimit(0, delayModeCount - 1,
        static_cast<int>(std::lround(load(raw.mode))));
    const int selectedDelayStyle = juce::jlimit(
        DelayStyleIndex::single, DelayStyleIndex::quad,
        static_cast<int>(std::lround(load(raw.delayStyle))));
    const float targetDiffusionAmount = juce::jlimit(
        0.0f, 1.0f, sequenced(21, raw.diffusionAmount) * 0.01f);
    const float targetDiffusionSize = juce::jlimit(
        0.0f, 1.0f, sequenced(22, raw.diffusionSize) * 0.01f);
    const float targetDiffusionFraction = targetDiffusionAmount >= 0.01f
        ? minimumDiffusionSizeFraction
            + (1.0f - minimumDiffusionSizeFraction) * targetDiffusionSize
        : 0.0f;
    const bool diffusionShortensStraightDelay =
        selectedMode != ModeIndex::reversePitch;
    std::array<float, delayChannelCount> straightTargetDelay = targetDelay;
    if (diffusionShortensStraightDelay)
    {
        for (int channel = 0; channel < delayChannelCount; ++channel)
        {
            const auto index = static_cast<size_t>(channel);
            straightTargetDelay[index] = juce::jmax(
                1.0f,
                targetDelay[index]
                    - (targetDelay[index] - 1.0f)
                        * targetDiffusionFraction);
        }
    }

    std::array<float, multiTapCount> targetTapDelay {};
    for (size_t tap = 0; tap < multiTapCount; ++tap)
    {
        const int tapTarget = 3 + static_cast<int>(tap) * 3;
        float tapMs = sequenced(tapTarget, raw.tapTime[tap]);
        if (multiTapSynced)
        {
            const int division = juce::jlimit(
                0, static_cast<int>(tempoDivisionBeats.size()) - 1,
                static_cast<int>(std::lround(sequencedWithValueParameter(
                    tapTarget, raw.tapDivision[tap],
                    sequencerTimeDivisionParameters[2 + tap]))));
            tapMs = static_cast<float>((60000.0 / bpm)
                * tempoDivisionBeats[static_cast<size_t>(division)]);
        }
        targetTapDelay[tap] = juce::jlimit(
            1.0f, maximumDelaySamples,
            tapMs * static_cast<float>(currentSampleRate * 0.001));
        multiTapPanValue[tap].setTargetValue(juce::jlimit(
            -1.0f, 1.0f,
            sequenced(tapTarget + 1, raw.tapPan[tap]) * 0.01f));
        multiTapLevelValue[tap].setTargetValue(juce::jlimit(
            0.0f, 1.0f,
            sequenced(tapTarget + 2, raw.tapLevel[tap]) * 0.01f));
        multiTapEnabledValue[tap].setTargetValue(
            load(raw.tapEnabled[tap]) > 0.5f ? 1.0f : 0.0f);
    }
    float multiTapReferenceDelay = maximumDelaySamples;
    bool hasEnabledTap = false;
    for (size_t tap = 0; tap < multiTapCount; ++tap)
    {
        if (load(raw.tapEnabled[tap]) > 0.5f)
        {
            multiTapReferenceDelay = juce::jmin(
                multiTapReferenceDelay, targetTapDelay[tap]);
            hasEnabledTap = true;
        }
    }
    if (! hasEnabledTap)
        multiTapReferenceDelay = 1.0f;

    const bool multiTapTarget = load(raw.multiTap) > 0.5f;
    const float multiTapDiffusionSpan = diffusionShortensStraightDelay
        ? (multiTapReferenceDelay - 1.0f) * targetDiffusionFraction
        : 0.0f;
    for (size_t tap = 0; tap < multiTapCount; ++tap)
        multiTapDelayValue[tap].setTargetValue(juce::jmax(
            1.0f, targetTapDelay[tap] - multiTapDiffusionSpan));
    multiTapBlend.setTargetValue(multiTapTarget ? 1.0f : 0.0f);

    const bool freezeRequested = load(raw.freeze) > 0.5f;
    if (freezeRequested && ! freezePlaybackActive
        && freezeBuffer.getNumSamples() > 1)
    {
        std::array<float, delayChannelCount> freezePeriod = targetDelay;
        if (multiTapTarget)
        {
            float longestTap = 1.0f;
            for (size_t tap = 0; tap < multiTapCount; ++tap)
                if (load(raw.tapEnabled[tap]) > 0.5f)
                    longestTap = juce::jmax(longestTap, targetTapDelay[tap]);
            freezePeriod.fill(longestTap);
        }
        else if (static_cast<int>(std::lround(load(raw.delayStyle)))
                 == DelayStyleIndex::ratio)
        {
            // Ratio's unitary matrix couples both delay lines.  Capture one
            // complete two-line circulation rather than unrelated periods.
            freezePeriod.fill(targetDelay[0] + targetDelay[1]);
        }
        else if (load(raw.pingPong) > 0.5f)
        {
            // A Ping-Pong pattern completes only after both channel legs.
            freezePeriod.fill(targetDelay[0] + targetDelay[1]);
        }

        const int freezeBufferLength = freezeBuffer.getNumSamples();
        const int availableSamples = juce::jmax(
            1, juce::jmin(freezeCaptureValidSamples,
                          freezeBufferLength - 1));
        for (int channel = 0; channel < delayChannelCount; ++channel)
        {
            const auto index = static_cast<size_t>(channel);
            freezeLoopLength[index] = juce::jlimit(
                1, availableSamples,
                static_cast<int>(std::lround(freezePeriod[index])));
            int start = freezeCaptureWritePosition - freezeLoopLength[index];
            if (start < 0)
                start += freezeBufferLength;
            freezeLoopStart[index] = start;
            freezeReadPosition[index] = 0;
        }
        freezePlaybackActive = true;
    }

    setDigitalDelayTarget(0, straightTargetDelay[0]);
    setDigitalDelayTarget(1, straightTargetDelay[1]);
    tapeDelayL.setTargetValue(straightTargetDelay[0]);
    tapeDelayR.setTargetValue(straightTargetDelay[1]);

    feedbackValue.setTargetValue(feedbackGainForPercentage(
        sequenced(15, raw.feedback)));
    crossValue.setTargetValue(juce::jlimit(
        0.0f, 1.0f, sequenced(16, raw.cross) * 0.01f));
    pingPongValue.setTargetValue(load(raw.pingPong) > 0.5f ? 1.0f : 0.0f);
    ratioStyleValue.setTargetValue(
        selectedDelayStyle == DelayStyleIndex::ratio ? 1.0f : 0.0f);
    spreadValue.setTargetValue(juce::jlimit(
        -1.0f, 1.0f, sequenced(17, raw.spread) * 0.01f));
    duckAmountValue.setTargetValue(juce::jlimit(
        0.0f, 1.0f, sequenced(18, raw.duck) * 0.01f));
    duckAttackCoefficientValue.setTargetValue(
        envelopeCoefficient(sequenced(19, raw.duckAttack), currentSampleRate));
    duckReleaseCoefficientValue.setTargetValue(
        envelopeCoefficient(sequenced(20, raw.duckRelease), currentSampleRate));
    diffusionAmountValue.setTargetValue(
        juce::jlimit(0.0f, 1.0f,
                     sequenced(21, raw.diffusionAmount) * 0.01f));
    diffusionSizeValue.setTargetValue(
        juce::jlimit(0.0f, 1.0f,
                     sequenced(22, raw.diffusionSize) * 0.01f));
    const float modulationAmount = juce::jlimit(
        0.0f, 1.0f, sequenced(23, raw.modulationAmount) * 0.01f);
    modulationAmountValue.setTargetValue(
        modulationAmount * modulationAmount
            * static_cast<float>(currentSampleRate
                                * maximumModulationDelaySeconds));
    modulationRateValue.setTargetValue(
        juce::jlimit(0.02f, 10.0f, sequenced(24, raw.modulationRate)));
    const float pitchShiftSemitones = juce::jlimit(
        -maximumPitchShiftSemitones, maximumPitchShiftSemitones,
        sequenced(25, raw.pitchShift));
    const float pitchDetuneSemitones = juce::jlimit(
        -maximumPitchDetuneSemitones, maximumPitchDetuneSemitones,
        sequenced(26, raw.pitchDetune));
    pitchRatioL.setTargetValue(pitchRatioForSemitones(
        pitchShiftSemitones - pitchDetuneSemitones * 0.5f));
    pitchRatioR.setTargetValue(pitchRatioForSemitones(
        pitchShiftSemitones + pitchDetuneSemitones * 0.5f));
    detuneRatioL.setTargetValue(pitchRatioForSemitones(
        -pitchDetuneSemitones * 0.5f));
    detuneRatioR.setTargetValue(pitchRatioForSemitones(
        pitchDetuneSemitones * 0.5f));
    updateGhostOscillatorIncrements(sequenced(27, raw.ghostShift),
                                    sequenced(28, raw.ghostDetune));
    phaserResonanceValue.setTargetValue(juce::jlimit(
        0.0f, 1.0f, sequenced(29, raw.phaserResonance) * 0.01f));
    phaserStageValue.setTargetValue(juce::jlimit(
        2.0f, 12.0f, sequenced(30, raw.phaserStages)));
    pitchMixValue.setTargetValue(juce::jlimit(
        0.0f, 1.0f, sequenced(31, raw.pitchMix) * 0.01f));
    mixValue.setTargetValue(juce::jlimit(
        0.0f, 1.0f, sequenced(32, raw.mix) * 0.01f));
    outputGainValue.setTargetValue(juce::Decibels::decibelsToGain(
        sequenced(35, raw.output)));
    bypassValue.setTargetValue(load(raw.bypass) > 0.5f ? 1.0f : 0.0f);
    freezeValue.setTargetValue(freezeRequested ? 1.0f : 0.0f);
    driveValue.setTargetValue(juce::jlimit(
        0.0f, 1.0f, sequenced(36, raw.drive) * 0.01f));

    const auto wow = juce::jlimit(
        0.0f, 1.0f, sequenced(38, raw.wow) * 0.01f);
    const auto flutter = juce::jlimit(
        0.0f, 1.0f, sequenced(39, raw.flutter) * 0.01f);
    wowDepthValue.setTargetValue(wow * wow * static_cast<float>(currentSampleRate * 0.010));
    flutterDepthValue.setTargetValue(flutter * flutter * static_cast<float>(currentSampleRate * 0.0015));

    const float lowCutHz = sequenced(33, raw.lowCut);
    const float highCutHz = sequenced(34, raw.highCut);
    const bool highpassEnabled = lowCutHz > 10.0f;
    const bool lowpassEnabled = highCutHz < 19999.0f;
    lowpassCoefficientValue.setTargetValue(onePoleCoefficient(highCutHz, currentSampleRate));
    highpassCoefficientValue.setTargetValue(onePoleCoefficient(lowCutHz, currentSampleRate));

    const auto age = juce::jlimit(
        0.0f, 1.0f, sequenced(37, raw.age) * 0.01f);
    ageValue.setTargetValue(age);
    const float averageDelayMs = (targetDelay[0] + targetDelay[1])
                               * 500.0f / static_cast<float>(currentSampleRate);
    const float tapeDelayWear = std::sqrt(juce::jlimit(0.0f, 1.0f,
                                                       averageDelayMs / 2000.0f));
    const float tapeWear = juce::jlimit(0.0f, 1.0f, age * 0.80f + tapeDelayWear * 0.20f);
    const auto tapeLowpassHz = juce::jmap(tapeWear, 12500.0f, 3200.0f);
    const auto tapeHighpassHz = juce::jmap(age, 35.0f, 180.0f);
    const float tapeIIWear = juce::jlimit(
        0.0f, 1.0f, age * 0.72f + tapeDelayWear * 0.18f);
    const auto tapeIILowpassHz = juce::jmap(
        tapeIIWear, 15500.0f, 3600.0f);
    const auto tapeIIHighpassHz = juce::jmap(age, 25.0f, 145.0f);
    const float chromeTapeWear = juce::jlimit(
        0.0f, 1.0f, age * 0.90f + tapeDelayWear * 0.10f);
    const auto chromeTapeLowpassHz = juce::jmap(
        chromeTapeWear, 19000.0f, 3800.0f);
    const auto chromeTapeHighpassHz = juce::jmap(age, 20.0f, 160.0f);
    const auto hifiLowpassHz = juce::jmap(age, 19500.0f, 11500.0f);
    const auto hifiHighpassHz = juce::jmap(age, 18.0f, 65.0f);
    const auto ghostLowpassHz = juce::jmap(age, 17000.0f, 8500.0f);
    const auto ghostHighpassHz = juce::jmap(age, 25.0f, 120.0f);
    const float lofiWear = 0.30f + age * 0.70f;
    const auto lofiLowpassHz = juce::jmap(lofiWear, 9000.0f, 2400.0f);
    const auto lofiHighpassHz = juce::jmap(lofiWear, 45.0f, 170.0f);
    const auto crystalAgeLowpassHz = juce::jmap(age * age, 20000.0f, 4200.0f);
    tapeLowpassCoefficientValue.setTargetValue(onePoleCoefficient(tapeLowpassHz, currentSampleRate));
    tapeHighpassCoefficientValue.setTargetValue(onePoleCoefficient(tapeHighpassHz, currentSampleRate));
    tapeIILowpassCoefficientValue.setTargetValue(
        onePoleCoefficient(tapeIILowpassHz, currentSampleRate));
    tapeIIHighpassCoefficientValue.setTargetValue(
        onePoleCoefficient(tapeIIHighpassHz, currentSampleRate));
    chromeTapeLowpassCoefficientValue.setTargetValue(
        onePoleCoefficient(chromeTapeLowpassHz, currentSampleRate));
    chromeTapeHighpassCoefficientValue.setTargetValue(
        onePoleCoefficient(chromeTapeHighpassHz, currentSampleRate));
    hifiLowpassCoefficientValue.setTargetValue(onePoleCoefficient(hifiLowpassHz, currentSampleRate));
    hifiHighpassCoefficientValue.setTargetValue(onePoleCoefficient(hifiHighpassHz, currentSampleRate));
    ghostLowpassCoefficientValue.setTargetValue(onePoleCoefficient(ghostLowpassHz, currentSampleRate));
    ghostHighpassCoefficientValue.setTargetValue(onePoleCoefficient(ghostHighpassHz, currentSampleRate));
    lofiLowpassCoefficientValue.setTargetValue(onePoleCoefficient(lofiLowpassHz, currentSampleRate));
    lofiHighpassCoefficientValue.setTargetValue(onePoleCoefficient(lofiHighpassHz, currentSampleRate));
    crystalAgeLowpassCoefficientValue.setTargetValue(
        onePoleCoefficient(crystalAgeLowpassHz, currentSampleRate));

    for (int channel = 0; channel < delayChannelCount; ++channel)
    {
        const auto index = static_cast<size_t>(channel);
        const float delayMs = targetDelay[index] * 1000.0f / static_cast<float>(currentSampleRate);
        const float delayWear = std::sqrt(juce::jlimit(0.0f, 1.0f, delayMs / 2000.0f));
        const float bbdWear = juce::jlimit(0.0f, 1.0f,
                                           0.18f + age * 0.62f + delayWear * 0.20f);
        const float bbdLowpassHz = juce::jmap(bbdWear, 11000.0f, 2200.0f);
        const float bbdHighpassHz = juce::jmap(age, 35.0f, 180.0f);
        const float analogWear = juce::jlimit(
            0.0f, 1.0f, 0.08f + age * 0.55f + delayWear * 0.15f);
        const float analogLowpassHz = juce::jmap(
            analogWear, 19000.0f, 4400.0f);
        const float analogHighpassHz = juce::jmap(age, 25.0f, 120.0f);
        bbdLowpassCoefficientValue[index].setTargetValue(
            onePoleCoefficient(bbdLowpassHz, currentSampleRate));
        bbdHighpassCoefficientValue[index].setTargetValue(
            onePoleCoefficient(bbdHighpassHz, currentSampleRate));
        analogLowpassCoefficientValue[index].setTargetValue(
            onePoleCoefficient(analogLowpassHz, currentSampleRate));
        analogHighpassCoefficientValue[index].setTargetValue(
            onePoleCoefficient(analogHighpassHz, currentSampleRate));
    }

    if (selectedMode != currentMode)
    {
        if (selectedMode == 0)
        {
            digitalTriangleState.fill(0.0f);
            multiTapDigitalTriangleState = {};
            for (int channel = 0; channel < delayChannelCount; ++channel)
            {
                auto& state = digitalDelay[static_cast<size_t>(channel)];
                state.previousDelaySamples = straightTargetDelay[static_cast<size_t>(channel)];
                state.targetDelaySamples = straightTargetDelay[static_cast<size_t>(channel)];
                state.requestedDelaySamples = straightTargetDelay[static_cast<size_t>(channel)];
                state.crossfade = 1.0f;
            }
        }
        else if (selectedMode == 1)
        {
            tapeLowpassState.fill(0.0f);
            tapeLowpassState2.fill(0.0f);
            tapeHighpassState.fill(0.0f);
            tapeHighpassInput.fill(0.0f);
        }
        else if (selectedMode == 2)
        {
            bbdLowpassState.fill(0.0f);
            bbdLowpassState2.fill(0.0f);
            bbdHighpassState.fill(0.0f);
            bbdHighpassInput.fill(0.0f);
        }
        else if (selectedMode == 3)
        {
            hifiLowpassState.fill(0.0f);
            hifiHighpassState.fill(0.0f);
            hifiHighpassInput.fill(0.0f);
        }
        else if (selectedMode == 4)
        {
            resetGhostState();
        }
        else if (selectedMode == 5)
        {
            pitchPhase = { 0.0f, 0.0f };
            multiTapPitchPhase = {};
        }
        else if (selectedMode == 6)
        {
            reversePitchPhase = { 0.0f, 0.0f };
            multiTapReversePitchPhase = {};
        }
        else if (selectedMode == 7)
        {
            resetLoFiState();
        }
        else if (selectedMode == 8)
        {
            resetCrystalState();
        }
        else if (selectedMode == 9)
        {
            resetPhaserState();
        }
        else if (selectedMode == 10)
        {
            lushPitchShiftPhase = { 0.0f, 0.0f };
            lushPitchDetunePhase = { 0.25f, 0.75f };
            multiTapLushShiftPhase = {};
            multiTapLushDetunePhase = {{
                { 0.25f, 0.75f }, { 0.25f, 0.75f },
                { 0.25f, 0.75f }, { 0.25f, 0.75f }
            }};
        }
        else if (selectedMode == 11)
        {
            chromeTapeLowpassState.fill(0.0f);
            chromeTapeLowpassState2.fill(0.0f);
            chromeTapeHighpassState.fill(0.0f);
            chromeTapeHighpassInput.fill(0.0f);
        }
        else if (selectedMode == 12)
        {
            analogLowpassState.fill(0.0f);
            analogLowpassState2.fill(0.0f);
            analogHighpassState.fill(0.0f);
            analogHighpassInput.fill(0.0f);
        }
        else if (selectedMode == ModeIndex::tapeII)
        {
            resetTapeIIState();
        }
        else if (selectedMode >= ModeIndex::chaos
                 && selectedMode <= ModeIndex::reverseForward)
        {
            if (currentMode < ModeIndex::chaos)
                resetGranularModeState();
            else if (selectedMode == ModeIndex::chaos)
            {
                chaosPhase.fill(0.0f);
                multiTapChaosPhase = {};
            }
            else if (selectedMode == ModeIndex::pureReverse)
            {
                pureReversePhase.fill(0.0f);
                multiTapPureReversePhase = {};
            }
            else if (selectedMode == ModeIndex::reverseChaos)
            {
                reverseChaosPhase.fill(0.0f);
                multiTapReverseChaosPhase = {};
            }
            else if (selectedMode == ModeIndex::reverseForward)
            {
                reverseForwardPhase.fill(0.0f);
                multiTapReverseForwardPhase = {};
                for (auto& path : reverseForwardLowpassState)
                    path.fill(0.0f);
                for (auto& path : reverseForwardHighpassState)
                    path.fill(0.0f);
                for (auto& path : reverseForwardHighpassInput)
                    path.fill(0.0f);
            }
        }
        currentMode = selectedMode;
    }
    digitalModeBlend.setTargetValue(selectedMode == 0 ? 1.0f : 0.0f);
    tapeModeBlend.setTargetValue(selectedMode == 1 ? 1.0f : 0.0f);
    bbdModeBlend.setTargetValue(selectedMode == 2 ? 1.0f : 0.0f);
    hifiModeBlend.setTargetValue(selectedMode == 3 ? 1.0f : 0.0f);
    ghostModeBlend.setTargetValue(selectedMode == 4 ? 1.0f : 0.0f);
    pitchModeBlend.setTargetValue(selectedMode == 5 ? 1.0f : 0.0f);
    reversePitchModeBlend.setTargetValue(selectedMode == 6 ? 1.0f : 0.0f);
    lofiModeBlend.setTargetValue(selectedMode == 7 ? 1.0f : 0.0f);
    crystalModeBlend.setTargetValue(selectedMode == 8 ? 1.0f : 0.0f);
    phaserModeBlend.setTargetValue(selectedMode == 9 ? 1.0f : 0.0f);
    lushPitchModeBlend.setTargetValue(selectedMode == 10 ? 1.0f : 0.0f);
    chromeTapeModeBlend.setTargetValue(selectedMode == 11 ? 1.0f : 0.0f);
    analogModeBlend.setTargetValue(selectedMode == 12 ? 1.0f : 0.0f);
    chaosModeBlend.setTargetValue(selectedMode == ModeIndex::chaos ? 1.0f : 0.0f);
    pureReverseModeBlend.setTargetValue(selectedMode == ModeIndex::pureReverse ? 1.0f : 0.0f);
    reverseChaosModeBlend.setTargetValue(selectedMode == ModeIndex::reverseChaos ? 1.0f : 0.0f);
    reverseForwardModeBlend.setTargetValue(selectedMode == ModeIndex::reverseForward ? 1.0f : 0.0f);
    tapeIIModeBlend.setTargetValue(selectedMode == ModeIndex::tapeII ? 1.0f : 0.0f);

    const float averageTapDelayMs = [&targetTapDelay, this]
    {
        float total = 0.0f;
        for (const float delaySamples : targetTapDelay)
            total += delaySamples * 1000.0f
                   / static_cast<float>(currentSampleRate);
        return total / static_cast<float>(multiTapCount);
    }();
    const float multiTapDelayWear = std::sqrt(juce::jlimit(
        0.0f, 1.0f, averageTapDelayMs / 2000.0f));
    float multiTapLowpassHz = 20000.0f;
    float multiTapHighpassHz = 20.0f;
    float multiTapHeadroom = 8.0f;
    switch (selectedMode)
    {
        case 1:
        {
            const float wear = juce::jlimit(
                0.0f, 1.0f, age * 0.80f + multiTapDelayWear * 0.20f);
            multiTapLowpassHz = juce::jmap(wear, 12500.0f, 3200.0f);
            multiTapHighpassHz = juce::jmap(age, 35.0f, 180.0f);
            multiTapHeadroom = juce::jmap(age, 1.35f, 0.75f);
            break;
        }
        case 2:
        {
            const float wear = juce::jlimit(
                0.0f, 1.0f,
                0.18f + age * 0.62f + multiTapDelayWear * 0.20f);
            multiTapLowpassHz = juce::jmap(wear, 11000.0f, 2200.0f);
            multiTapHighpassHz = juce::jmap(age, 35.0f, 180.0f);
            multiTapHeadroom = 1.40f;
            break;
        }
        case 3:
            multiTapLowpassHz = hifiLowpassHz;
            multiTapHighpassHz = hifiHighpassHz;
            multiTapHeadroom = juce::jmap(age, 1.80f, 1.15f);
            break;
        case 4:
            multiTapLowpassHz = ghostLowpassHz;
            multiTapHighpassHz = ghostHighpassHz;
            multiTapHeadroom = juce::jmap(age, 1.60f, 0.95f);
            break;
        case 7:
            multiTapLowpassHz = lofiLowpassHz;
            multiTapHighpassHz = lofiHighpassHz;
            multiTapHeadroom = 1.15f;
            break;
        case 11:
            multiTapLowpassHz = chromeTapeLowpassHz;
            multiTapHighpassHz = chromeTapeHighpassHz;
            multiTapHeadroom = juce::jmap(age, 2.40f, 0.90f);
            break;
        case 12:
        {
            const float wear = juce::jlimit(
                0.0f, 1.0f,
                0.08f + age * 0.55f + multiTapDelayWear * 0.15f);
            multiTapLowpassHz = juce::jmap(wear, 19000.0f, 4400.0f);
            multiTapHighpassHz = juce::jmap(age, 25.0f, 120.0f);
            multiTapHeadroom = juce::jmap(age, 1.90f, 1.15f);
            break;
        }
        case ModeIndex::tapeII:
        {
            const float wear = juce::jlimit(
                0.0f, 1.0f,
                age * 0.72f + multiTapDelayWear * 0.18f);
            multiTapLowpassHz = juce::jmap(wear, 15500.0f, 3600.0f);
            multiTapHighpassHz = juce::jmap(age, 25.0f, 145.0f);
            multiTapHeadroom = juce::jmap(age, 1.80f, 0.90f);
            break;
        }
        default:
            break;
    }
    multiTapLowpassCoefficientValue.setTargetValue(
        onePoleCoefficient(multiTapLowpassHz, currentSampleRate));
    multiTapHighpassCoefficientValue.setTargetValue(
        onePoleCoefficient(multiTapHighpassHz, currentSampleRate));
    multiTapHeadroomValue.setTargetValue(multiTapHeadroom);

    const double wowIncrement = juce::MathConstants<double>::twoPi * 0.33 / currentSampleRate;
    const double flutterIncrement = juce::MathConstants<double>::twoPi * 5.2 / currentSampleRate;
    const float sampleRateFloat = static_cast<float>(currentSampleRate);
    const std::array<float, delayChannelCount> tapeDriftScale
    {
        0.65f + 0.70f * std::sqrt(juce::jlimit(0.0f, 1.0f, leftMs / 2000.0f)),
        0.65f + 0.70f * std::sqrt(juce::jlimit(0.0f, 1.0f, rightMs / 2000.0f))
    };
    const float tapeBaseWowDepth = sampleRateFloat * 0.00018f;
    const float tapeBaseFlutterDepth = sampleRateFloat * 0.000030f;
    const float tapeIIBaseWowDepth = sampleRateFloat * 0.00011f;
    const float tapeIIBaseFlutterDepth = sampleRateFloat * 0.000018f;
    const float chromeTapeBaseWowDepth = sampleRateFloat * 0.000050f;
    const float chromeTapeBaseFlutterDepth = sampleRateFloat * 0.000008f;
    const float bbdBaseWowDepth = sampleRateFloat * 0.000070f;
    const float bbdBaseFlutterDepth = sampleRateFloat * 0.000018f;
    const float analogBaseWowDepth = sampleRateFloat * 0.000050f;
    const float analogBaseFlutterDepth = sampleRateFloat * 0.000012f;
    const float hifiBaseWowDepth = sampleRateFloat * 0.000035f;
    const float hifiBaseFlutterDepth = sampleRateFloat * 0.000008f;
    const float ghostBaseWowDepth = sampleRateFloat * 0.000050f;
    const float ghostBaseFlutterDepth = sampleRateFloat * 0.000012f;
    const float lofiBaseWowDepth = sampleRateFloat * 0.00045f;
    const float lofiBaseFlutterDepth = sampleRateFloat * 0.00010f;
    const float digitalModulationFloor = juce::jmax(
        1.0f, sampleRateFloat * 0.00025f);
    const float roundedTriangleSmoothingCoefficient = onePoleCoefficient(
        180.0f, currentSampleRate);
    float* output[delayChannelCount]
    {
        buffer.getWritePointer(0),
        activeChannelCount > 1 ? buffer.getWritePointer(1) : nullptr
    };
    const int selectedDistortionType = juce::jlimit(
        0, distortionTypeCount - 1,
        static_cast<int>(std::lround(load(raw.distortionType))));
    if (selectedDistortionType != currentDistortionType)
    {
        previousDistortionType = currentDistortionType;
        currentDistortionType = selectedDistortionType;
        distortionTypeCrossfade.reset(currentSampleRate, smoothingSeconds);
        distortionTypeCrossfade.setCurrentAndTargetValue(0.0f);
        distortionTypeCrossfade.setTargetValue(1.0f);
    }

    for (int sample = 0; sample < sampleCount; ++sample)
    {
        const float inputL = sanitiseSample(output[0][sample], 64.0f);
        const float inputR = activeChannelCount > 1
                           ? sanitiseSample(output[1][sample], 64.0f)
                           : inputL;
        const float freeze = freezeValue.getNextValue();
        const float normalFeedback = feedbackValue.getNextValue();
        const float feedback = normalFeedback
                             + (1.0f - normalFeedback) * freeze;

        const float duckAmount = duckAmountValue.getNextValue();
        const float duckAttackCoefficient = duckAttackCoefficientValue.getNextValue();
        const float duckReleaseCoefficient = duckReleaseCoefficientValue.getNextValue();
        const float duckDetector = juce::jmax(std::abs(inputL), std::abs(inputR));
        const float duckCoefficient = duckDetector > duckEnvelope
                                    ? duckAttackCoefficient
                                    : duckReleaseCoefficient;
        duckEnvelope = duckCoefficient * duckEnvelope
                     + (1.0f - duckCoefficient) * duckDetector;
        duckEnvelope = juce::jlimit(0.0f, 4.0f,
                                    sanitiseSample(duckEnvelope, 4.0f));
        const float duckActivity = juce::jlimit(0.0f, 1.0f, duckEnvelope * 4.0f);
        const float duckGain = juce::jlimit(0.0f, 1.0f,
                                            1.0f - duckAmount * duckActivity);

        const auto driveAmount = driveValue.getNextValue();
        const float distortionCrossfade = distortionTypeCrossfade.getNextValue();
        float delayInputL = processDistortionType(
            inputL, driveAmount, 0, currentDistortionType);
        float delayInputR = processDistortionType(
            inputR, driveAmount, 1, currentDistortionType);
        if (previousDistortionType != currentDistortionType
            && distortionCrossfade < 1.0f)
        {
            const float previousL = processDistortionType(
                inputL, driveAmount, 0, previousDistortionType);
            const float previousR = processDistortionType(
                inputR, driveAmount, 1, previousDistortionType);
            delayInputL = previousL
                + (delayInputL - previousL) * distortionCrossfade;
            delayInputR = previousR
                + (delayInputR - previousR) * distortionCrossfade;
        }

        // Freeze excludes new input while the captured wet cycle takes over.
        // The live engine also approaches unity underneath, then returns to
        // the user's stored Feedback value when Freeze is released.
        const float liveInputGain = 1.0f - freeze;
        delayInputL *= liveInputGain;
        delayInputR *= liveInputGain;

        const float multiTapAmount = multiTapBlend.getNextValue();
        const float stereoTimingAmount = 1.0f - multiTapAmount;
        const float pingPong = pingPongValue.getNextValue()
                             * stereoTimingAmount;
        const float ratioStyle = ratioStyleValue.getNextValue()
                               * stereoTimingAmount;
        const float pingPongInput = 0.5f * (delayInputL + delayInputR);
        delayInputL += pingPong * (pingPongInput - delayInputL);
        delayInputR *= 1.0f - pingPong;

        const float digitalAmount = digitalModeBlend.getNextValue();
        const float tapeAmount = tapeModeBlend.getNextValue();
        const float bbdAmount = bbdModeBlend.getNextValue();
        const float hifiAmount = hifiModeBlend.getNextValue();
        const float ghostAmount = ghostModeBlend.getNextValue();
        const float pitchAmount = pitchModeBlend.getNextValue();
        const float reversePitchAmount = reversePitchModeBlend.getNextValue();
        const float lofiAmount = lofiModeBlend.getNextValue();
        const float crystalAmount = crystalModeBlend.getNextValue();
        const float phaserAmount = phaserModeBlend.getNextValue();
        const float lushPitchAmount = lushPitchModeBlend.getNextValue();
        const float chromeTapeAmount = chromeTapeModeBlend.getNextValue();
        const float analogAmount = analogModeBlend.getNextValue();
        const float chaosAmount = chaosModeBlend.getNextValue();
        const float pureReverseAmount = pureReverseModeBlend.getNextValue();
        const float reverseChaosAmount = reverseChaosModeBlend.getNextValue();
        const float reverseForwardAmount = reverseForwardModeBlend.getNextValue();
        const float tapeIIAmount = tapeIIModeBlend.getNextValue();
        const float pitchRatioLeft = pitchRatioL.getNextValue();
        const float pitchRatioRight = pitchRatioR.getNextValue();
        const float detuneRatioLeft = detuneRatioL.getNextValue();
        const float detuneRatioRight = detuneRatioR.getNextValue();
        const float phaserResonance = phaserResonanceValue.getNextValue();
        const float phaserStages = phaserStageValue.getNextValue();
        const float pitchMix = pitchMixValue.getNextValue();
        const float ageForSample = ageValue.getNextValue();
        const float converterLevels = floatingConverterLevels(ageForSample);
        const float lofiWearForSample = 0.30f + ageForSample * 0.70f;
        const float wowDepth = wowDepthValue.getNextValue();
        const float flutterDepth = flutterDepthValue.getNextValue();
        const float modulationDepth = modulationAmountValue.getNextValue();
        const float modulationRate = modulationRateValue.getNextValue();
        std::array<float, delayChannelCount> chaosDelayScale { 1.0f, 1.0f };
        std::array<float, delayChannelCount> chaosPitchRatio { 1.0f, 1.0f };
        if (chaosAmount > 0.0001f || reverseChaosAmount > 0.0001f)
        {
            for (int channel = 0; channel < delayChannelCount; ++channel)
            {
                const auto index = static_cast<size_t>(channel);
                const float timeMotion = nextRandomModulation(
                    chaosTimeModulation[index],
                    0.42f + modulationRate
                        * (channel == 0 ? 0.29f : 0.34f));
                const float pitchMotion = nextRandomModulation(
                    chaosPitchModulation[index],
                    0.63f + modulationRate
                        * (channel == 0 ? 0.41f : 0.37f));
                chaosDelayScale[index] = juce::jlimit(
                    0.58f, 1.42f, 1.0f + timeMotion * 0.34f);
                const float semitones = juce::jlimit(
                    -9.0f, 9.0f, pitchMotion * 7.0f + timeMotion * 2.0f);
                chaosPitchRatio[index] = pitchRatioForSemitones(semitones);
            }
        }
        // This is the continuous effective Time position for the current
        // sample. Every secondary delay calculation must follow it rather
        // than the block-rate parameter target, otherwise mouse updates can
        // jump modulation depth or diffusion taps and create broadband ticks.
        const std::array<float, delayChannelCount> tapeDelaySamples
        {
            tapeDelayL.getNextValue(), tapeDelayR.getNextValue()
        };
        std::array<float, delayChannelCount> globalModulation {};
        std::array<float, delayChannelCount> modulationWave {};
        std::array<float, delayChannelCount> digitalModulation {};
        std::array<float, delayChannelCount> digitalModulationWave {};
        const double modulationPhaseTurns = modulationPhase
            / juce::MathConstants<double>::twoPi;
        if (modulationDepth > 0.0001f)
        {
            // A fixed 35 ms range is useful for long delays but turns a 1 ms
            // flange into a wide chorus.  Scale the physical excursion with
            // delay time, reaching the established 35 ms maximum at 250 ms.
            // Centred sine users are additionally kept above the read floor.
            for (int channel = 0; channel < delayChannelCount; ++channel)
            {
                const auto index = static_cast<size_t>(channel);
                double phase = modulationPhaseTurns
                    + modulationPhaseOffsetTurns(selectedDelayStyle, channel);
                phase -= std::floor(phase);

                modulationWave[index] = static_cast<float>(std::sin(
                    juce::MathConstants<double>::twoPi * phase));
                const float requestedDepth = modulationDepthForDelay(
                    modulationDepth, tapeDelaySamples[index], sampleRateFloat);
                const float centredDepth = juce::jmin(
                    requestedDepth,
                    juce::jmax(0.0f,
                               tapeDelaySamples[index]
                                   - digitalModulationFloor));
                globalModulation[index] = modulationWave[index]
                                        * centredDepth;

                const float triangleTarget = 1.0f - 4.0f
                    * std::abs(static_cast<float>(phase) - 0.5f);
                digitalTriangleState[index] =
                    (1.0f - roundedTriangleSmoothingCoefficient) * triangleTarget
                    + roundedTriangleSmoothingCoefficient
                        * digitalTriangleState[index];
                digitalModulationWave[index] = digitalTriangleState[index];

                const float maximumCentredDepth = juce::jmax(
                    0.0f, tapeDelaySamples[index]
                              - digitalModulationFloor);
                const float forwardBias = juce::jmax(
                    0.0f, requestedDepth - maximumCentredDepth);
                digitalModulation[index] = forwardBias
                    + digitalModulationWave[index] * requestedDepth;
            }

            modulationPhase += juce::MathConstants<double>::twoPi
                             * static_cast<double>(modulationRate)
                             / currentSampleRate;
            if (modulationPhase >= juce::MathConstants<double>::twoPi)
                modulationPhase -= juce::MathConstants<double>::twoPi;
        }
        float wowSine = 0.0f;
        std::array<float, delayChannelCount> flutterSine {};
        const float analogueAmount = tapeAmount + bbdAmount + hifiAmount
                                   + ghostAmount + lofiAmount + chromeTapeAmount
                                   + analogAmount + tapeIIAmount;
        if (wowDepth > 0.0001f || flutterDepth > 0.0001f || analogueAmount > 0.0001f)
        {
            wowSine = static_cast<float>(std::sin(wowPhase));
            flutterSine[0] = static_cast<float>(std::sin(flutterPhase));
            flutterSine[1] = static_cast<float>(std::sin(flutterPhase + 0.65));

            wowPhase += wowIncrement;
            flutterPhase += flutterIncrement;
            if (wowPhase >= juce::MathConstants<double>::twoPi)
                wowPhase -= juce::MathConstants<double>::twoPi;
            if (flutterPhase >= juce::MathConstants<double>::twoPi)
                flutterPhase -= juce::MathConstants<double>::twoPi;
        }

        const std::array<float, delayChannelCount> userModulation
        {
            globalModulation[0] + wowSine * wowDepth + flutterSine[0] * flutterDepth,
            globalModulation[1] + wowSine * wowDepth + flutterSine[1] * flutterDepth
        };
        digitalModulation[0] += wowSine * wowDepth
                              + flutterSine[0] * flutterDepth;
        digitalModulation[1] += wowSine * wowDepth
                              + flutterSine[1] * flutterDepth;
        std::array<float, delayChannelCount> crystalModulation {};
        if (crystalAmount > 0.0001f)
        {
            for (int channel = 0; channel < delayChannelCount; ++channel)
            {
                const auto index = static_cast<size_t>(channel);
                double phase = crystalPhase
                    + modulationPhaseOffsetTurns(selectedDelayStyle, channel);
                phase -= std::floor(phase);
                const float triangleTarget = 1.0f - 4.0f
                    * std::abs(static_cast<float>(phase) - 0.5f);
                crystalTriangleState[index] =
                    (1.0f - roundedTriangleSmoothingCoefficient) * triangleTarget
                    + roundedTriangleSmoothingCoefficient * crystalTriangleState[index];
                const float requestedDepth = modulationDepthForDelay(
                    modulationDepth, tapeDelaySamples[index], sampleRateFloat);
                const float maximumCentredDepth = juce::jmax(
                    0.0f, tapeDelaySamples[index]
                              - digitalModulationFloor);
                const float forwardBias = juce::jmax(
                    0.0f, requestedDepth - maximumCentredDepth);
                crystalModulation[index] = forwardBias
                                        + crystalTriangleState[index]
                                            * requestedDepth
                                        + wowSine * wowDepth
                                        + flutterSine[index] * flutterDepth;
            }

            crystalPhase += static_cast<double>(modulationRate) / currentSampleRate;
            if (crystalPhase >= 1.0)
                crystalPhase -= 1.0;
        }
        std::array<float, delayChannelCount> tapeModulation {};
        std::array<float, delayChannelCount> tapeIIModulation {};
        std::array<float, delayChannelCount> chromeTapeModulation {};
        std::array<float, delayChannelCount> bbdModulation {};
        std::array<float, delayChannelCount> analogModulation {};
        std::array<float, delayChannelCount> hifiModulation {};
        std::array<float, delayChannelCount> ghostModulation {};
        std::array<float, delayChannelCount> lofiModulation {};
        std::array<float, delayChannelCount> lofiSlowMotion {};
        std::array<float, delayChannelCount> lofiScrapeMotion {};
        for (int channel = 0; channel < delayChannelCount; ++channel)
        {
            const auto index = static_cast<size_t>(channel);
            tapeModulation[index] = userModulation[index]
                                  + wowSine * tapeBaseWowDepth * tapeDriftScale[index]
                                  + flutterSine[index] * tapeBaseFlutterDepth;
            tapeIIModulation[index] = userModulation[index]
                                    + wowSine * tapeIIBaseWowDepth
                                        * tapeDriftScale[index]
                                    + flutterSine[index]
                                        * tapeIIBaseFlutterDepth;
            chromeTapeModulation[index] = userModulation[index]
                                        + wowSine * chromeTapeBaseWowDepth
                                        + flutterSine[index]
                                            * chromeTapeBaseFlutterDepth;
            bbdModulation[index] = userModulation[index]
                                 + wowSine * bbdBaseWowDepth
                                 + flutterSine[index] * bbdBaseFlutterDepth;
            analogModulation[index] = userModulation[index]
                                    + wowSine * analogBaseWowDepth
                                    + flutterSine[index]
                                        * analogBaseFlutterDepth;
            hifiModulation[index] = userModulation[index]
                                  + wowSine * hifiBaseWowDepth
                                  + flutterSine[index] * hifiBaseFlutterDepth;
            ghostModulation[index] = userModulation[index]
                                   + wowSine * ghostBaseWowDepth
                                   + flutterSine[index] * ghostBaseFlutterDepth;

            if (lofiAmount > 0.0001f)
            {
                const float slowRate = modulationRate * (channel == 0 ? 0.91f : 1.09f);
                const float scrapeRate = 600.0f + ageForSample * 5400.0f
                                       + static_cast<float>(channel) * 173.0f;
                const float slowRandom = nextRandomModulation(
                    lofiSlowModulation[index], slowRate);
                const float scrapeRandom = nextRandomModulation(
                    lofiScrapeModulation[index], scrapeRate);
                const float slowDepth = juce::jmin(
                    tapeDelaySamples[index] * 0.35f,
                    modulationDepth * 3.2f + sampleRateFloat * 0.00055f);
                const float scrapeDepth = juce::jmin(
                    tapeDelaySamples[index] * 0.08f,
                    sampleRateFloat * (0.000015f
                        + lofiWearForSample * lofiWearForSample * 0.00025f));
                lofiSlowMotion[index] = slowRandom;
                lofiScrapeMotion[index] = scrapeRandom;
                lofiModulation[index] = wowSine * (wowDepth + lofiBaseWowDepth)
                                      + flutterSine[index]
                                        * (flutterDepth + lofiBaseFlutterDepth)
                                      + slowRandom * slowDepth
                                      + scrapeRandom * scrapeDepth;
            }
        }
        std::array<float, delayChannelCount> digitalWet {};
        std::array<float, delayChannelCount> tapeWet {};
        std::array<float, delayChannelCount> tapeIIWet {};
        std::array<float, delayChannelCount> bbdWet {};
        std::array<float, delayChannelCount> analogWet {};
        std::array<float, delayChannelCount> hifiWet {};
        std::array<float, delayChannelCount> ghostWet {};
        std::array<float, delayChannelCount> pitchWet {};
        std::array<float, delayChannelCount> reversePitchWet {};
        std::array<float, delayChannelCount> lofiWet {};
        std::array<float, delayChannelCount> crystalWet {};
        std::array<float, delayChannelCount> phaserWet {};
        std::array<float, delayChannelCount> lushPitchWet {};
        std::array<float, delayChannelCount> chromeTapeWet {};
        std::array<float, delayChannelCount> chaosWet {};
        std::array<float, delayChannelCount> pureReverseWet {};
        std::array<float, delayChannelCount> reverseChaosWet {};
        std::array<float, delayChannelCount> reverseForwardWet {};
        std::array<float, delayChannelCount> reverseForwardLoopWet {};

        if (digitalAmount > 0.0001f)
        {
            digitalWet[0] = quantiseFloatingConverter(
                readDigitalDelay(0, digitalModulation[0]), converterLevels);
            digitalWet[1] = quantiseFloatingConverter(
                readDigitalDelay(1, digitalModulation[1]), converterLevels);
        }

        if (crystalAmount > 0.0001f)
        {
            for (int channel = 0; channel < delayChannelCount; ++channel)
            {
                const auto index = static_cast<size_t>(channel);
                const float delayed = readDelaySample(
                    channel, tapeDelaySamples[index] + crystalModulation[index]);
                crystalWet[index] = applyFeedbackSaturation(delayed, 3.5f);
            }
        }

        if (phaserAmount > 0.0001f)
        {
            const float normalisedModulationDepth = std::sqrt(juce::jlimit(
                0.0f, 1.0f, modulationDepth
                    / juce::jmax(1.0f, sampleRateFloat
                                * maximumModulationDelaySeconds)));
            const float internalResonance = 0.82f * phaserResonance;
            const float stagePosition = juce::jlimit(
                0.0f, 5.0f, (phaserStages - 2.0f) * 0.5f);
            const int lowerStageChoice = juce::jlimit(
                0, 5, static_cast<int>(std::floor(stagePosition)));
            const int upperStageChoice = juce::jmin(5, lowerStageChoice + 1);
            const float stageBlend = stagePosition
                                   - static_cast<float>(lowerStageChoice);
            for (int channel = 0; channel < delayChannelCount; ++channel)
            {
                const auto index = static_cast<size_t>(channel);
                const float delayMovement = wowSine * wowDepth
                                          + flutterSine[index] * flutterDepth;
                const float delayed = readDelaySample(
                    channel, tapeDelaySamples[index] + delayMovement);
                const float sweepOctaves = 3.25f * normalisedModulationDepth;
                const float phaserFrequency = juce::jlimit(
                    70.0f, juce::jmin(9000.0f, sampleRateFloat * 0.42f),
                    900.0f * std::pow(2.0f,
                        modulationWave[index] * sweepOctaves));
                const float tangent = std::tan(
                    juce::MathConstants<float>::pi * phaserFrequency
                    / sampleRateFloat);
                const float allpassCoefficient = (tangent - 1.0f)
                                               / (tangent + 1.0f);
                float phased = delayed
                             + phaserFeedbackState[index] * internalResonance;
                std::array<float, phaserStageCount / 2> stageOutputs {};
                for (size_t stage = 0; stage < phaserStageCount; ++stage)
                {
                    phased = processAllpass(
                        phased, allpassCoefficient,
                        phaserAllpassState[index][stage]);
                    if ((stage & 1u) != 0u)
                        stageOutputs[stage / 2u] = phased;
                }

                const float selectedPhased = stageOutputs[
                    static_cast<size_t>(lowerStageChoice)]
                    + stageBlend * (stageOutputs[
                        static_cast<size_t>(upperStageChoice)]
                        - stageOutputs[static_cast<size_t>(lowerStageChoice)]);
                phaserFeedbackState[index] = applyFeedbackSaturation(
                    selectedPhased, 1.8f);
                phaserWet[index] = sanitiseSample(
                    0.5f * (delayed + selectedPhased), 16.0f);
            }
        }

        const float tapeLowpassCoefficient = tapeLowpassCoefficientValue.getNextValue();
        const float tapeHighpassCoefficient = tapeHighpassCoefficientValue.getNextValue();
        if (tapeAmount > 0.0001f)
        {
            tapeWet[0] = readDelaySample(0, tapeDelaySamples[0] + tapeModulation[0]);
            tapeWet[1] = readDelaySample(1, tapeDelaySamples[1] + tapeModulation[1]);

            for (int channel = 0; channel < delayChannelCount; ++channel)
            {
                const auto index = static_cast<size_t>(channel);
                tapeLowpassState[index] = (1.0f - tapeLowpassCoefficient) * tapeWet[index]
                                        + tapeLowpassCoefficient * tapeLowpassState[index];
                tapeLowpassState2[index] = (1.0f - tapeLowpassCoefficient)
                                         * tapeLowpassState[index]
                                         + tapeLowpassCoefficient * tapeLowpassState2[index];
                tapeHighpassState[index] = tapeHighpassCoefficient
                                         * (tapeHighpassState[index] + tapeLowpassState2[index]
                                            - tapeHighpassInput[index]);
                tapeHighpassInput[index] = tapeLowpassState2[index];
                const float tapeHeadroom = juce::jmap(ageForSample, 1.35f, 0.75f);
                tapeWet[index] = applyFeedbackSaturation(tapeHighpassState[index],
                                                         tapeHeadroom);
            }
        }

        const float tapeIILowpassCoefficient =
            tapeIILowpassCoefficientValue.getNextValue();
        const float tapeIIHighpassCoefficient =
            tapeIIHighpassCoefficientValue.getNextValue();
        if (tapeIIAmount > 0.0001f)
        {
            for (int channel = 0; channel < delayChannelCount; ++channel)
            {
                const auto index = static_cast<size_t>(channel);
                const float transportDelay = advanceTapeIITransport(
                    tapeIITransport[index], straightTargetDelay[index]);
                const float delayedSample = readDelaySample(
                    channel, transportDelay + tapeIIModulation[index]);

                tapeIILowpassState[index] =
                    (1.0f - tapeIILowpassCoefficient) * delayedSample
                    + tapeIILowpassCoefficient * tapeIILowpassState[index];
                tapeIILowpassState2[index] =
                    (1.0f - tapeIILowpassCoefficient)
                        * tapeIILowpassState[index]
                    + tapeIILowpassCoefficient * tapeIILowpassState2[index];
                tapeIIHighpassState[index] = tapeIIHighpassCoefficient
                    * (tapeIIHighpassState[index]
                        + tapeIILowpassState2[index]
                        - tapeIIHighpassInput[index]);
                tapeIIHighpassInput[index] = tapeIILowpassState2[index];

                const float saturationGain = 1.10f
                                           + ageForSample * 1.40f;
                const float saturationBias = 0.02f
                                           + ageForSample * 0.07f;
                const float zeroOffset = std::tanh(saturationBias);
                const float slope = juce::jmax(
                    0.05f, 1.0f - zeroOffset * zeroOffset);
                const float magnetic = (std::tanh(
                    tapeIIHighpassState[index] * saturationGain
                        + saturationBias) - zeroOffset)
                    / (saturationGain * slope);
                const float headroom = juce::jmap(
                    ageForSample, 1.80f, 0.90f);
                tapeIIWet[index] = applyFeedbackSaturation(
                    magnetic, headroom);
            }
        }

        const float chromeTapeLowpassCoefficient =
            chromeTapeLowpassCoefficientValue.getNextValue();
        const float chromeTapeHighpassCoefficient =
            chromeTapeHighpassCoefficientValue.getNextValue();
        if (chromeTapeAmount > 0.0001f)
        {
            for (int channel = 0; channel < delayChannelCount; ++channel)
            {
                const auto index = static_cast<size_t>(channel);
                const float delayedSample = readDelaySample(
                    channel, tapeDelaySamples[index]
                        + chromeTapeModulation[index]);
                chromeTapeLowpassState[index] =
                    (1.0f - chromeTapeLowpassCoefficient) * delayedSample
                    + chromeTapeLowpassCoefficient
                        * chromeTapeLowpassState[index];
                chromeTapeLowpassState2[index] =
                    (1.0f - chromeTapeLowpassCoefficient)
                        * chromeTapeLowpassState[index]
                    + chromeTapeLowpassCoefficient
                        * chromeTapeLowpassState2[index];
                chromeTapeHighpassState[index] =
                    chromeTapeHighpassCoefficient
                    * (chromeTapeHighpassState[index]
                        + chromeTapeLowpassState2[index]
                        - chromeTapeHighpassInput[index]);
                chromeTapeHighpassInput[index] =
                    chromeTapeLowpassState2[index];

                const float chromeHeadroom = juce::jmap(
                    ageForSample, 2.40f, 0.90f);
                chromeTapeWet[index] = applyFeedbackSaturation(
                    chromeTapeHighpassState[index], chromeHeadroom);
            }
        }

        if (bbdAmount > 0.0001f)
        {
            const float quantisationLevels = juce::jmap(ageForSample, 8192.0f, 768.0f);
            const float noiseLevel = 0.00020f + ageForSample * ageForSample * 0.0035f;

            for (int channel = 0; channel < delayChannelCount; ++channel)
            {
                const auto index = static_cast<size_t>(channel);
                const float delayedSample = readDelaySample(
                    channel, tapeDelaySamples[index] + bbdModulation[index]);
                const float compressed = std::copysign(std::sqrt(std::abs(delayedSample)),
                                                       delayedSample);
                const float quantised = std::round(compressed * quantisationLevels)
                                      / quantisationLevels;
                // Keep the compander itself memoryless. Filtering between
                // square-root compression and square expansion is not
                // energy-neutral and can manufacture loop gain from certain
                // waveforms. Expand the coloured/quantised signal first, then
                // let the physical BBD filters provide predictable loss.
                const float signalTrackedNoise = noiseLevel
                    * noiseExcitationGain(quantised, normalFeedback);
                const float noisy = quantised
                                  + nextNoiseSample(bbdNoiseState[index])
                                        * signalTrackedNoise;
                const float expanded = std::copysign(noisy * noisy, noisy);
                const float lowpassCoefficient = bbdLowpassCoefficientValue[index].getNextValue();
                const float highpassCoefficient = bbdHighpassCoefficientValue[index].getNextValue();
                bbdLowpassState[index] = (1.0f - lowpassCoefficient) * expanded
                                       + lowpassCoefficient * bbdLowpassState[index];
                bbdLowpassState2[index] = (1.0f - lowpassCoefficient)
                                        * bbdLowpassState[index]
                                        + lowpassCoefficient * bbdLowpassState2[index];
                bbdHighpassState[index] = highpassCoefficient
                                        * (bbdHighpassState[index] + bbdLowpassState2[index]
                                           - bbdHighpassInput[index]);
                bbdHighpassInput[index] = bbdLowpassState2[index];
                bbdWet[index] = applyFeedbackSaturation(
                    bbdHighpassState[index], 1.40f);
            }
        }

        if (analogAmount > 0.0001f)
        {
            const float quantisationLevels = juce::jmap(
                ageForSample, 16384.0f, 1536.0f);
            const float noiseLevel = 0.00010f
                                   + ageForSample * ageForSample * 0.00175f;

            for (int channel = 0; channel < delayChannelCount; ++channel)
            {
                const auto index = static_cast<size_t>(channel);
                const float delayedSample = readDelaySample(
                    channel, tapeDelaySamples[index] + analogModulation[index]);
                const float compressed = std::copysign(
                    std::sqrt(std::abs(delayedSample)), delayedSample);
                const float quantised = std::round(compressed * quantisationLevels)
                                      / quantisationLevels;
                const float signalTrackedNoise = noiseLevel
                    * noiseExcitationGain(quantised, normalFeedback);
                const float noisy = quantised
                    + nextNoiseSample(analogNoiseState[index])
                        * signalTrackedNoise;
                const float expanded = std::copysign(noisy * noisy, noisy);
                const float lowpassCoefficient =
                    analogLowpassCoefficientValue[index].getNextValue();
                const float highpassCoefficient =
                    analogHighpassCoefficientValue[index].getNextValue();
                analogLowpassState[index] =
                    (1.0f - lowpassCoefficient) * expanded
                    + lowpassCoefficient * analogLowpassState[index];
                analogLowpassState2[index] =
                    (1.0f - lowpassCoefficient) * analogLowpassState[index]
                    + lowpassCoefficient * analogLowpassState2[index];
                analogHighpassState[index] = highpassCoefficient
                    * (analogHighpassState[index]
                        + analogLowpassState2[index]
                        - analogHighpassInput[index]);
                analogHighpassInput[index] = analogLowpassState2[index];
                const float saturationGain = 1.35f + ageForSample * 2.40f;
                const float saturationBias = 0.035f + ageForSample * 0.10f;
                const float zeroOffset = std::tanh(saturationBias);
                const float slope = juce::jmax(
                    0.05f, 1.0f - zeroOffset * zeroOffset);
                const float coloured =
                    (std::tanh(analogHighpassState[index] * saturationGain
                               + saturationBias)
                        - zeroOffset)
                    / (saturationGain * slope);
                analogWet[index] = applyFeedbackSaturation(
                    coloured, juce::jmap(ageForSample, 1.90f, 1.15f));
            }
        }

        const float hifiLowpassCoefficient = hifiLowpassCoefficientValue.getNextValue();
        const float hifiHighpassCoefficient = hifiHighpassCoefficientValue.getNextValue();
        if (hifiAmount > 0.0001f)
        {
            const float asperityLevel = 0.000015f
                                      + ageForSample * ageForSample * 0.00055f;
            for (int channel = 0; channel < delayChannelCount; ++channel)
            {
                const auto index = static_cast<size_t>(channel);
                const float delayedSample = readDelaySample(
                    channel, tapeDelaySamples[index] + hifiModulation[index]);
                hifiLowpassState[index] = (1.0f - hifiLowpassCoefficient) * delayedSample
                                        + hifiLowpassCoefficient * hifiLowpassState[index];
                hifiHighpassState[index] = hifiHighpassCoefficient
                                         * (hifiHighpassState[index] + hifiLowpassState[index]
                                            - hifiHighpassInput[index]);
                hifiHighpassInput[index] = hifiLowpassState[index];
                const float asperity = nextNoiseSample(hifiNoiseState[index])
                                      * asperityLevel
                                      * noiseExcitationGain(
                                          hifiHighpassState[index],
                                          normalFeedback);
                const float hifiHeadroom = juce::jmap(ageForSample, 1.80f, 1.15f);
                hifiWet[index] = applyFeedbackSaturation(hifiHighpassState[index] + asperity,
                                                         hifiHeadroom);
            }
        }

        const float lofiLowpassCoefficient = lofiLowpassCoefficientValue.getNextValue();
        const float lofiHighpassCoefficient = lofiHighpassCoefficientValue.getNextValue();
        if (lofiAmount > 0.0001f)
        {
            const float quantisationLevels = juce::jmap(
                lofiWearForSample * lofiWearForSample, 1536.0f, 128.0f);
            const float noiseLevel = 0.0008f
                                   + lofiWearForSample * lofiWearForSample * 0.0060f;
            const float saturationGain = 1.75f + lofiWearForSample * 4.0f;
            const float saturationBias = 0.08f + lofiWearForSample * 0.28f;
            const float zeroOffset = std::tanh(saturationBias);
            const float biasSlope = juce::jmax(0.05f,
                1.0f - zeroOffset * zeroOffset);
            const float outputCompensation = 0.95f
                                           / (saturationGain * biasSlope);

            for (int channel = 0; channel < delayChannelCount; ++channel)
            {
                const auto index = static_cast<size_t>(channel);
                const float delayedSample = readDelaySample(
                    channel, tapeDelaySamples[index] + lofiModulation[index]);
                lofiLowpassState[index] = (1.0f - lofiLowpassCoefficient)
                                        * delayedSample
                                        + lofiLowpassCoefficient
                                        * lofiLowpassState[index];
                lofiLowpassState2[index] = (1.0f - lofiLowpassCoefficient)
                                         * lofiLowpassState[index]
                                         + lofiLowpassCoefficient
                                         * lofiLowpassState2[index];
                lofiHighpassState[index] = lofiHighpassCoefficient
                                         * (lofiHighpassState[index]
                                            + lofiLowpassState2[index]
                                            - lofiHighpassInput[index]);
                lofiHighpassInput[index] = lofiLowpassState2[index];

                const float motionGain = juce::jlimit(
                    0.72f, 1.28f,
                    1.0f
                        + lofiSlowMotion[index]
                            * (0.035f + lofiWearForSample * 0.080f)
                        + lofiScrapeMotion[index]
                            * (0.010f + lofiWearForSample * 0.025f));
                const float worn = lofiHighpassState[index] * motionGain;
                const float noisy = worn
                                  + nextNoiseSample(lofiNoiseState[index])
                                    * noiseLevel
                                    * noiseExcitationGain(
                                        worn, normalFeedback, 12.0f);
                const float quantised = std::round(noisy * quantisationLevels)
                                      / quantisationLevels;
                const float asymmetric = (std::tanh(
                    quantised * saturationGain + saturationBias) - zeroOffset)
                    * outputCompensation;
                lofiWet[index] = applyFeedbackSaturation(asymmetric, 1.15f);
            }
        }

        const float ghostLowpassCoefficient = ghostLowpassCoefficientValue.getNextValue();
        const float ghostHighpassCoefficient = ghostHighpassCoefficientValue.getNextValue();
        if (ghostAmount > 0.0001f)
        {
            for (int channel = 0; channel < delayChannelCount; ++channel)
            {
                const auto index = static_cast<size_t>(channel);
                const float delayedSample = readDelaySample(
                    channel, tapeDelaySamples[index] + ghostModulation[index]);
                const float shifted = processGhostFrequencyShift(delayedSample, channel);
                ghostLowpassState[index] = (1.0f - ghostLowpassCoefficient) * shifted
                                         + ghostLowpassCoefficient * ghostLowpassState[index];
                ghostHighpassState[index] = ghostHighpassCoefficient
                                          * (ghostHighpassState[index]
                                             + ghostLowpassState[index]
                                             - ghostHighpassInput[index]);
                ghostHighpassInput[index] = ghostLowpassState[index];
                const float ghostHeadroom = juce::jmap(ageForSample, 1.60f, 0.95f);
                ghostWet[index] = applyFeedbackSaturation(ghostHighpassState[index],
                                                          ghostHeadroom);
            }

        }

        if (pitchAmount > 0.0001f)
        {
            pitchWet[0] = quantiseFloatingConverter(
                readPitchDelay(0,
                               tapeDelaySamples[0] + userModulation[0],
                               pitchRatioLeft),
                converterLevels);
            pitchWet[1] = quantiseFloatingConverter(
                readPitchDelay(1,
                               tapeDelaySamples[1] + userModulation[1],
                               pitchRatioRight),
                converterLevels);
        }

        if (reversePitchAmount > 0.0001f)
        {
            reversePitchWet[0] = quantiseFloatingConverter(
                readReversePitchDelay(
                    0, tapeDelaySamples[0] + userModulation[0],
                    pitchRatioLeft),
                converterLevels);
            reversePitchWet[1] = quantiseFloatingConverter(
                readReversePitchDelay(
                    1, tapeDelaySamples[1] + userModulation[1],
                    pitchRatioRight),
                converterLevels);
        }

        if (pureReverseAmount > 0.0001f)
        {
            for (int channel = 0; channel < delayChannelCount; ++channel)
            {
                const auto index = static_cast<size_t>(channel);
                pureReverseWet[index] = readReversePitchDelayWithPhase(
                    channel, tapeDelaySamples[index] + userModulation[index],
                    1.0f, pureReversePhase);
            }
        }

        if (reverseForwardAmount > 0.0001f)
        {
            for (int channel = 0; channel < delayChannelCount; ++channel)
            {
                const auto index = static_cast<size_t>(channel);
                const float delaySamples = tapeDelaySamples[index]
                                         + userModulation[index];
                reverseForwardWet[index] = readDelaySample(
                    channel, delaySamples);
                reverseForwardLoopWet[index] = readReversePitchDelayWithPhase(
                    channel, delaySamples, 1.0f, reverseForwardPhase);
            }
        }

        if (chaosAmount > 0.0001f || reverseChaosAmount > 0.0001f)
        {
            for (int channel = 0; channel < delayChannelCount; ++channel)
            {
                const auto index = static_cast<size_t>(channel);
                const float chaoticDelay = tapeDelaySamples[index]
                    * chaosDelayScale[index] + userModulation[index];
                if (chaosAmount > 0.0001f)
                    chaosWet[index] = readPitchDelayWithPhase(
                        channel, chaoticDelay, chaosPitchRatio[index],
                        chaosPhase);
                if (reverseChaosAmount > 0.0001f)
                    reverseChaosWet[index] = readReversePitchDelayWithPhase(
                        channel, chaoticDelay, chaosPitchRatio[index],
                        reverseChaosPhase);
            }
        }

        if (lushPitchAmount > 0.0001f)
        {
            const std::array<float, delayChannelCount> shifted
            {
                readPitchDelayWithPhase(
                    0, tapeDelaySamples[0] + userModulation[0],
                    pitchRatioLeft, lushPitchShiftPhase),
                readPitchDelayWithPhase(
                    1, tapeDelaySamples[1] + userModulation[1],
                    pitchRatioRight, lushPitchShiftPhase)
            };
            const std::array<float, delayChannelCount> detuned
            {
                readPitchDelayWithPhase(
                    0, tapeDelaySamples[0] + userModulation[0],
                    detuneRatioLeft, lushPitchDetunePhase),
                readPitchDelayWithPhase(
                    1, tapeDelaySamples[1] + userModulation[1],
                    detuneRatioRight, lushPitchDetunePhase)
            };
            for (int channel = 0; channel < delayChannelCount; ++channel)
            {
                const auto index = static_cast<size_t>(channel);
                lushPitchWet[index] = sanitiseSample(
                    detuned[index] + pitchMix
                        * (shifted[index] - detuned[index]),
                    16.0f);
            }
        }

        const float multiTapLowpassCoefficient =
            multiTapLowpassCoefficientValue.getNextValue();
        const float multiTapHighpassCoefficient =
            multiTapHighpassCoefficientValue.getNextValue();
        const float multiTapHeadroomForSample =
            multiTapHeadroomValue.getNextValue();
        std::array<float, multiTapCount> tapDelaySamples {};
        std::array<float, multiTapCount> tapPan {};
        std::array<float, multiTapCount> tapGain {};
        float tapGainSquareSum = 0.0f;
        size_t repeatTapIndex = 0;
        float repeatTapDelay = -1.0f;
        for (size_t tap = 0; tap < multiTapCount; ++tap)
        {
            tapDelaySamples[tap] = multiTapDelayValue[tap].getNextValue();
            tapPan[tap] = multiTapPanValue[tap].getNextValue();
            tapGain[tap] = multiTapLevelValue[tap].getNextValue()
                         * multiTapEnabledValue[tap].getNextValue();
            tapGainSquareSum += tapGain[tap] * tapGain[tap];
            if (tapGain[tap] > 0.0001f
                && tapDelaySamples[tap] >= repeatTapDelay)
            {
                repeatTapDelay = tapDelaySamples[tap];
                repeatTapIndex = tap;
            }
        }

        std::array<float, delayChannelCount> multiTapWet {};
        std::array<float, delayChannelCount> multiTapRepeatWet {};
        if (multiTapAmount > 0.0001f)
        {
            const float tapNormalisation = 1.0f
                / std::sqrt(juce::jmax(1.0f, tapGainSquareSum));
            const bool doubleStageFilter = selectedMode == 1
                || selectedMode == 2 || selectedMode == 7
                || selectedMode == 11 || selectedMode == 12
                || selectedMode == ModeIndex::tapeII;

            for (size_t tap = 0; tap < multiTapCount; ++tap)
            {
                float tapBaseDelay = tapDelaySamples[tap];
                if (selectedMode == ModeIndex::tapeII)
                    tapBaseDelay = advanceTapeIITransport(
                        multiTapTapeIITransport[tap], tapDelaySamples[tap]);

                std::array<float, delayChannelCount> tapUserModulation {};
                std::array<float, delayChannelCount> tapDigitalModulation {};
                std::array<float, delayChannelCount> tapCrystalModulation {};
                for (int channel = 0; channel < delayChannelCount; ++channel)
                {
                    const auto index = static_cast<size_t>(channel);
                    const float directMotion = wowSine * wowDepth
                        + flutterSine[index] * flutterDepth;
                    tapUserModulation[index] = directMotion;
                    tapDigitalModulation[index] = directMotion;
                    tapCrystalModulation[index] = directMotion;

                    if (modulationDepth > 0.0001f)
                    {
                        const double tapOffset = selectedDelayStyle
                                                       == DelayStyleIndex::quad
                            ? static_cast<double>(tap) * 0.25
                            : 0.0;
                        double phase = modulationPhaseTurns + tapOffset
                            + modulationPhaseOffsetTurns(
                                selectedDelayStyle, channel);
                        phase -= std::floor(phase);

                        const float requestedDepth = modulationDepthForDelay(
                            modulationDepth, tapBaseDelay, sampleRateFloat);
                        const float maximumCentredDepth = juce::jmax(
                            0.0f, tapBaseDelay - digitalModulationFloor);
                        const float centredDepth = juce::jmin(
                            requestedDepth, maximumCentredDepth);
                        const float sine = static_cast<float>(std::sin(
                            juce::MathConstants<double>::twoPi * phase));
                        tapUserModulation[index] += sine * centredDepth;

                        const float triangleTarget = 1.0f - 4.0f
                            * std::abs(static_cast<float>(phase) - 0.5f);
                        auto& digitalTriangle =
                            multiTapDigitalTriangleState[tap][index];
                        digitalTriangle =
                            (1.0f - roundedTriangleSmoothingCoefficient)
                                * triangleTarget
                            + roundedTriangleSmoothingCoefficient
                                * digitalTriangle;
                        const float forwardBias = juce::jmax(
                            0.0f, requestedDepth - maximumCentredDepth);
                        tapDigitalModulation[index] += forwardBias
                            + digitalTriangle * requestedDepth;

                        double crystalTapPhase = crystalPhase + tapOffset
                            + modulationPhaseOffsetTurns(
                                selectedDelayStyle, channel);
                        crystalTapPhase -= std::floor(crystalTapPhase);
                        const float crystalTarget = 1.0f - 4.0f
                            * std::abs(static_cast<float>(crystalTapPhase)
                                       - 0.5f);
                        auto& crystalTriangle =
                            multiTapCrystalTriangleState[tap][index];
                        crystalTriangle =
                            (1.0f - roundedTriangleSmoothingCoefficient)
                                * crystalTarget
                            + roundedTriangleSmoothingCoefficient
                                * crystalTriangle;
                        tapCrystalModulation[index] += forwardBias
                            + crystalTriangle * requestedDepth;
                    }
                }

                std::array<float, delayChannelCount> tapModulation =
                    tapUserModulation;
                if (selectedMode == ModeIndex::digital)
                    tapModulation = tapDigitalModulation;
                else if (selectedMode == 1)
                {
                    for (int channel = 0; channel < delayChannelCount; ++channel)
                    {
                        const auto index = static_cast<size_t>(channel);
                        tapModulation[index] = tapeModulation[index]
                            + tapUserModulation[index] - userModulation[index];
                    }
                }
                else if (selectedMode == ModeIndex::tapeII)
                {
                    for (int channel = 0; channel < delayChannelCount; ++channel)
                    {
                        const auto index = static_cast<size_t>(channel);
                        tapModulation[index] = tapeIIModulation[index]
                            + tapUserModulation[index] - userModulation[index];
                    }
                }
                else if (selectedMode == 2)
                {
                    for (int channel = 0; channel < delayChannelCount; ++channel)
                    {
                        const auto index = static_cast<size_t>(channel);
                        tapModulation[index] = bbdModulation[index]
                            + tapUserModulation[index] - userModulation[index];
                    }
                }
                else if (selectedMode == 3)
                {
                    for (int channel = 0; channel < delayChannelCount; ++channel)
                    {
                        const auto index = static_cast<size_t>(channel);
                        tapModulation[index] = hifiModulation[index]
                            + tapUserModulation[index] - userModulation[index];
                    }
                }
                else if (selectedMode == 4)
                {
                    for (int channel = 0; channel < delayChannelCount; ++channel)
                    {
                        const auto index = static_cast<size_t>(channel);
                        tapModulation[index] = ghostModulation[index]
                            + tapUserModulation[index] - userModulation[index];
                    }
                }
                else if (selectedMode == 7)
                    tapModulation = lofiModulation;
                else if (selectedMode == 8)
                    tapModulation = tapCrystalModulation;
                else if (selectedMode == 11)
                {
                    for (int channel = 0; channel < delayChannelCount; ++channel)
                    {
                        const auto index = static_cast<size_t>(channel);
                        tapModulation[index] = chromeTapeModulation[index]
                            + tapUserModulation[index] - userModulation[index];
                    }
                }
                else if (selectedMode == 12)
                {
                    for (int channel = 0; channel < delayChannelCount; ++channel)
                    {
                        const auto index = static_cast<size_t>(channel);
                        tapModulation[index] = analogModulation[index]
                            + tapUserModulation[index] - userModulation[index];
                    }
                }

                float tapSample = 0.0f;
                if (selectedMode == ModeIndex::ghost)
                {
                    const float left = processGhostFrequencyShift(
                        readDelaySample(
                            0, tapBaseDelay + tapModulation[0]),
                        0, multiTapGhostHilbertA[tap][0],
                        multiTapGhostHilbertB[tap][0]);
                    const float right = processGhostFrequencyShift(
                        readDelaySample(
                            1, tapBaseDelay + tapModulation[1]),
                        1, multiTapGhostHilbertA[tap][1],
                        multiTapGhostHilbertB[tap][1]);
                    tapSample = 0.5f * (left + right);
                }
                else if (selectedMode == 5)
                {
                    const float left = readPitchDelayWithPhase(
                        0, tapDelaySamples[tap] + tapModulation[0],
                        pitchRatioLeft, multiTapPitchPhase[tap]);
                    const float right = readPitchDelayWithPhase(
                        1, tapDelaySamples[tap] + tapModulation[1],
                        pitchRatioRight, multiTapPitchPhase[tap]);
                    tapSample = 0.5f * (left + right);
                }
                else if (selectedMode == 6)
                {
                    const float left = readReversePitchDelayWithPhase(
                        0, tapDelaySamples[tap], pitchRatioLeft,
                        multiTapReversePitchPhase[tap], true);
                    const float right = readReversePitchDelayWithPhase(
                        1, tapDelaySamples[tap], pitchRatioRight,
                        multiTapReversePitchPhase[tap], true);
                    tapSample = 0.5f * (left + right);
                }
                else if (selectedMode == ModeIndex::chaos)
                {
                    const float left = readPitchDelayWithPhase(
                        0, tapDelaySamples[tap] * chaosDelayScale[0]
                            + tapModulation[0],
                        chaosPitchRatio[0], multiTapChaosPhase[tap]);
                    const float right = readPitchDelayWithPhase(
                        1, tapDelaySamples[tap] * chaosDelayScale[1]
                            + tapModulation[1],
                        chaosPitchRatio[1], multiTapChaosPhase[tap]);
                    tapSample = 0.5f * (left + right);
                }
                else if (selectedMode == ModeIndex::pureReverse)
                {
                    const float left = readReversePitchDelayWithPhase(
                        0, tapDelaySamples[tap], 1.0f,
                        multiTapPureReversePhase[tap]);
                    const float right = readReversePitchDelayWithPhase(
                        1, tapDelaySamples[tap], 1.0f,
                        multiTapPureReversePhase[tap]);
                    tapSample = 0.5f * (left + right);
                }
                else if (selectedMode == ModeIndex::reverseChaos)
                {
                    const float left = readReversePitchDelayWithPhase(
                        0, tapDelaySamples[tap] * chaosDelayScale[0]
                            + tapModulation[0],
                        chaosPitchRatio[0], multiTapReverseChaosPhase[tap]);
                    const float right = readReversePitchDelayWithPhase(
                        1, tapDelaySamples[tap] * chaosDelayScale[1]
                            + tapModulation[1],
                        chaosPitchRatio[1], multiTapReverseChaosPhase[tap]);
                    tapSample = 0.5f * (left + right);
                }
                else if (selectedMode == ModeIndex::reverseForward)
                {
                    const float forwardLeft = readDelaySample(
                        0, tapDelaySamples[tap] + tapModulation[0]);
                    const float forwardRight = readDelaySample(
                        1, tapDelaySamples[tap] + tapModulation[1]);
                    const float reverseLeft = readReversePitchDelayWithPhase(
                        0, tapDelaySamples[tap] + tapModulation[0], 1.0f,
                        multiTapReverseForwardPhase[tap]);
                    const float reverseRight = readReversePitchDelayWithPhase(
                        1, tapDelaySamples[tap] + tapModulation[1], 1.0f,
                        multiTapReverseForwardPhase[tap]);
                    tapSample = 0.25f * (forwardLeft + forwardRight
                                       + reverseLeft + reverseRight);
                }
                else if (selectedMode == 10)
                {
                    const float shiftedLeft = readPitchDelayWithPhase(
                        0, tapDelaySamples[tap] + tapModulation[0],
                        pitchRatioLeft, multiTapLushShiftPhase[tap]);
                    const float shiftedRight = readPitchDelayWithPhase(
                        1, tapDelaySamples[tap] + tapModulation[1],
                        pitchRatioRight, multiTapLushShiftPhase[tap]);
                    const float detunedLeft = readPitchDelayWithPhase(
                        0, tapDelaySamples[tap] + tapModulation[0],
                        detuneRatioLeft, multiTapLushDetunePhase[tap]);
                    const float detunedRight = readPitchDelayWithPhase(
                        1, tapDelaySamples[tap] + tapModulation[1],
                        detuneRatioRight, multiTapLushDetunePhase[tap]);
                    const float shifted = 0.5f
                        * (shiftedLeft + shiftedRight);
                    const float detuned = 0.5f
                        * (detunedLeft + detunedRight);
                    tapSample = detuned + pitchMix * (shifted - detuned);
                }
                else
                {
                    const float left = readDelaySample(
                        0, tapBaseDelay + tapModulation[0]);
                    const float right = readDelaySample(
                        1, tapBaseDelay + tapModulation[1]);
                    tapSample = 0.5f * (left + right);
                }

                if (selectedMode == ModeIndex::digital
                    || selectedMode == ModeIndex::pitch
                    || selectedMode == ModeIndex::reversePitch)
                    tapSample = quantiseFloatingConverter(
                        tapSample, converterLevels);

                const bool companderMode = selectedMode == 2
                                        || selectedMode == 12;
                if (companderMode)
                {
                    // Match the stable stereo BBD/Analogue topology: colour
                    // and expand first, then enter the stateful filters.
                    tapSample = std::copysign(
                        std::sqrt(std::abs(tapSample)), tapSample);
                    const float levels = selectedMode == 2
                        ? juce::jmap(ageForSample, 8192.0f, 768.0f)
                        : juce::jmap(ageForSample, 16384.0f, 1536.0f);
                    tapSample = std::round(tapSample * levels) / levels;
                    const float companderNoiseLevel = selectedMode == 2
                        ? 0.00020f
                            + ageForSample * ageForSample * 0.0035f
                        : 0.00010f
                            + ageForSample * ageForSample * 0.00175f;
                    const float trackedNoise = companderNoiseLevel
                        * noiseExcitationGain(tapSample, normalFeedback);
                    tapSample += nextNoiseSample(multiTapNoiseState[tap])
                               * trackedNoise;
                    tapSample = std::copysign(
                        tapSample * tapSample, tapSample);
                }
                else if (selectedMode == 7)
                {
                    const float levels = juce::jmap(
                        lofiWearForSample * lofiWearForSample,
                        1536.0f, 128.0f);
                    tapSample = std::round(tapSample * levels) / levels;
                }

                multiTapLowpassState[tap] =
                    (1.0f - multiTapLowpassCoefficient) * tapSample
                    + multiTapLowpassCoefficient
                        * multiTapLowpassState[tap];
                float filteredTap = multiTapLowpassState[tap];
                if (doubleStageFilter)
                {
                    multiTapLowpassState2[tap] =
                        (1.0f - multiTapLowpassCoefficient) * filteredTap
                        + multiTapLowpassCoefficient
                            * multiTapLowpassState2[tap];
                    filteredTap = multiTapLowpassState2[tap];
                }
                else
                {
                    multiTapLowpassState2[tap] = filteredTap;
                }
                multiTapHighpassState[tap] = multiTapHighpassCoefficient
                    * (multiTapHighpassState[tap] + filteredTap
                        - multiTapHighpassInput[tap]);
                multiTapHighpassInput[tap] = filteredTap;
                filteredTap = multiTapHighpassState[tap];

                float noiseLevel = 0.0f;
                if (selectedMode == 3)
                    noiseLevel = 0.000015f
                        + ageForSample * ageForSample * 0.00055f;
                else if (selectedMode == 7)
                    noiseLevel = 0.00050f
                        + lofiWearForSample * lofiWearForSample * 0.0060f;
                if (noiseLevel > 0.0f)
                {
                    const float trackedNoise = noiseLevel
                        * noiseExcitationGain(filteredTap, normalFeedback);
                    filteredTap += nextNoiseSample(multiTapNoiseState[tap])
                                 * trackedNoise;
                }
                if (selectedMode == 7 || selectedMode == 12)
                {
                    const float gain = selectedMode == 7
                        ? 1.75f + lofiWearForSample * 4.0f
                        : 1.35f + ageForSample * 2.40f;
                    const float bias = selectedMode == 7
                        ? 0.08f + lofiWearForSample * 0.28f
                        : 0.035f + ageForSample * 0.10f;
                    const float zeroOffset = std::tanh(bias);
                    const float slope = juce::jmax(
                        0.05f, 1.0f - zeroOffset * zeroOffset);
                    filteredTap = (std::tanh(filteredTap * gain + bias)
                                   - zeroOffset) / (gain * slope);
                }
                else if (selectedMode == ModeIndex::tapeII)
                {
                    const float gain = 1.10f + ageForSample * 1.40f;
                    const float bias = 0.02f + ageForSample * 0.07f;
                    const float zeroOffset = std::tanh(bias);
                    const float slope = juce::jmax(
                        0.05f, 1.0f - zeroOffset * zeroOffset);
                    filteredTap = (std::tanh(filteredTap * gain + bias)
                                   - zeroOffset) / (gain * slope);
                }
                filteredTap = applyFeedbackSaturation(
                    filteredTap, multiTapHeadroomForSample);

                const float gain = tapGain[tap] * tapNormalisation;
                const float pan = juce::jlimit(-1.0f, 1.0f, tapPan[tap]);
                const float leftGain = std::sqrt(0.5f * (1.0f - pan));
                const float rightGain = std::sqrt(0.5f * (1.0f + pan));
                multiTapWet[0] += filteredTap * gain * leftGain;
                multiTapWet[1] += filteredTap * gain * rightGain;
                if (tap == repeatTapIndex)
                {
                    // REPEAT feedback comes from one complete head rather
                    // than the audible sum.  This preserves full-strength
                    // modulation and metallic feedback colour while several
                    // coincident heads cannot multiply the loop gain.  Tap
                    // pan belongs only to the audible output: applying it to
                    // the recirculating signal silently weakens Feedback.
                    const float repeatSignal = filteredTap * tapGain[tap];
                    multiTapRepeatWet[0] = repeatSignal;
                    multiTapRepeatWet[1] = repeatSignal;
                }
            }
        }

        if (ghostAmount > 0.0001f)
            advanceGhostOscillators();

        const float lowpassCoefficient = lowpassCoefficientValue.getNextValue();
        const float highpassCoefficient = highpassCoefficientValue.getNextValue();
        std::array<float, delayChannelCount> wet {};

        for (int channel = 0; channel < delayChannelCount; ++channel)
        {
            const auto index = static_cast<size_t>(channel);
            const float stereoWet = digitalWet[index] * digitalAmount
                           + tapeWet[index] * tapeAmount
                           + tapeIIWet[index] * tapeIIAmount
                           + bbdWet[index] * bbdAmount
                           + hifiWet[index] * hifiAmount
                           + ghostWet[index] * ghostAmount
                           + pitchWet[index] * pitchAmount
                           + reversePitchWet[index] * reversePitchAmount
                           + lofiWet[index] * lofiAmount
                           + phaserWet[index] * phaserAmount
                           + lushPitchWet[index] * lushPitchAmount
                           + chromeTapeWet[index] * chromeTapeAmount
                           + analogWet[index] * analogAmount
                           + chaosWet[index] * chaosAmount
                           + pureReverseWet[index] * pureReverseAmount
                           + reverseChaosWet[index] * reverseChaosAmount;
            float filtered = stereoWet * stereoTimingAmount
                           + multiTapWet[index] * multiTapAmount;

            if (lowpassEnabled)
            {
                lowpassState[index] = (1.0f - lowpassCoefficient) * filtered
                                    + lowpassCoefficient * lowpassState[index];
                filtered = lowpassState[index];
            }
            else
            {
                lowpassState[index] = filtered;
            }

            if (highpassEnabled)
            {
                highpassState[index] = highpassCoefficient
                                     * (highpassState[index] + filtered - highpassInput[index]);
                highpassInput[index] = filtered;
                filtered = highpassState[index];
            }
            else
            {
                highpassState[index] = 0.0f;
                highpassInput[index] = filtered;
            }

            wet[index] = sanitiseSample(filtered, 16.0f);
        }

        const float diffusionAmount = diffusionAmountValue.getNextValue();
        const float diffusionSize = diffusionSizeValue.getNextValue();
        const float diffusionSizeFraction = minimumDiffusionSizeFraction
            + (1.0f - minimumDiffusionSizeFraction) * diffusionSize;
        float currentMultiTapReferenceDelay = maximumDelaySamples;
        bool hasCurrentTap = false;
        for (size_t tap = 0; tap < multiTapCount; ++tap)
        {
            if (tapGain[tap] > 0.0001f)
            {
                currentMultiTapReferenceDelay = juce::jmin(
                    currentMultiTapReferenceDelay, tapDelaySamples[tap]);
                hasCurrentTap = true;
            }
        }
        if (! hasCurrentTap)
            currentMultiTapReferenceDelay = 1.0f;
        const std::array<float, delayChannelCount> diffusionReferenceDelay
        {
            tapeDelaySamples[0]
                + multiTapAmount
                    * (currentMultiTapReferenceDelay - tapeDelaySamples[0]),
            tapeDelaySamples[1]
                + multiTapAmount
                    * (currentMultiTapReferenceDelay - tapeDelaySamples[1])
        };
        const std::array<float, delayChannelCount> diffusionSpan
        {
            (diffusionReferenceDelay[0] - 1.0f) * diffusionSizeFraction,
            (diffusionReferenceDelay[1] - 1.0f) * diffusionSizeFraction
        };
        const std::array<float, delayChannelCount> diffused
        {
            processDiffusion(wet[0], 0, diffusionAmount,
                             diffusionSpan[0], 0),
            processDiffusion(wet[1], 1, diffusionAmount,
                             diffusionSpan[1], 0)
        };
        for (int channel = 0; channel < delayChannelCount; ++channel)
        {
            const auto index = static_cast<size_t>(channel);
            wet[index] = sanitiseSample(diffused[index], 16.0f);
        }

        std::array<float, delayChannelCount> reverseForwardAudible {};
        std::array<float, delayChannelCount> reverseForwardLoop {};
        if (reverseForwardAmount > 0.0001f)
        {
            for (int path = 0; path < 2; ++path)
            {
                const auto pathIndex = static_cast<size_t>(path);
                for (int channel = 0; channel < delayChannelCount; ++channel)
                {
                    const auto index = static_cast<size_t>(channel);
                    const float source = path == 0
                        ? reverseForwardWet[index]
                        : reverseForwardLoopWet[index];
                    float filtered = source;
                    if (lowpassEnabled)
                    {
                        reverseForwardLowpassState[pathIndex][index] =
                            (1.0f - lowpassCoefficient) * filtered
                            + lowpassCoefficient
                                * reverseForwardLowpassState[pathIndex][index];
                        filtered = reverseForwardLowpassState[pathIndex][index];
                    }
                    else
                    {
                        reverseForwardLowpassState[pathIndex][index] = filtered;
                    }

                    if (highpassEnabled)
                    {
                        reverseForwardHighpassState[pathIndex][index] =
                            highpassCoefficient
                            * (reverseForwardHighpassState[pathIndex][index]
                                + filtered
                                - reverseForwardHighpassInput[pathIndex][index]);
                        reverseForwardHighpassInput[pathIndex][index] = filtered;
                        filtered = reverseForwardHighpassState[pathIndex][index];
                    }
                    else
                    {
                        reverseForwardHighpassState[pathIndex][index] = 0.0f;
                        reverseForwardHighpassInput[pathIndex][index] = filtered;
                    }

                    const float diffusedPath = processDiffusion(
                        filtered, channel, diffusionAmount,
                        diffusionSpan[index], 2 + path);
                    const float processed = applyFeedbackSaturation(
                        diffusedPath, 3.0f);
                    if (path == 0)
                        reverseForwardAudible[index] = processed;
                    else
                        reverseForwardLoop[index] = processed;
                }
            }
        }

        const std::array<float, delayChannelCount> crystalDiffused
        {
            processDiffusion(crystalWet[0], 0, diffusionAmount,
                             diffusionSpan[0], 1),
            processDiffusion(crystalWet[1], 1, diffusionAmount,
                             diffusionSpan[1], 1)
        };
        std::array<float, delayChannelCount> crystalLoopWet {};
        std::array<float, delayChannelCount> feedbackWet {};
        const float crystalAgeLowpassCoefficient =
            crystalAgeLowpassCoefficientValue.getNextValue();
        for (int channel = 0; channel < delayChannelCount; ++channel)
        {
            const auto index = static_cast<size_t>(channel);
            crystalLoopWet[index] = sanitiseSample(
                crystalDiffused[index], 16.0f);

            float crystalAudible = crystalLoopWet[index];
            if (lowpassEnabled)
            {
                crystalLowpassState[index] = (1.0f - lowpassCoefficient)
                                           * crystalAudible
                                           + lowpassCoefficient
                                           * crystalLowpassState[index];
                crystalAudible = crystalLowpassState[index];
            }
            else
            {
                crystalLowpassState[index] = crystalAudible;
            }

            if (highpassEnabled)
            {
                crystalHighpassState[index] = highpassCoefficient
                    * (crystalHighpassState[index] + crystalAudible
                        - crystalHighpassInput[index]);
                crystalHighpassInput[index] = crystalAudible;
                crystalAudible = crystalHighpassState[index];
            }
            else
            {
                crystalHighpassState[index] = 0.0f;
                crystalHighpassInput[index] = crystalAudible;
            }

            float crystalAged = crystalAudible;
            for (auto& stage : crystalAgeLowpassState[index])
            {
                stage = (1.0f - crystalAgeLowpassCoefficient) * crystalAged
                      + crystalAgeLowpassCoefficient * stage;
                crystalAged = stage;
            }
            crystalAudible += ageForSample * (crystalAged - crystalAudible);

            feedbackWet[index] = sanitiseSample(
                wet[index] + crystalLoopWet[index] * crystalAmount
                    * stereoTimingAmount
                    + reverseForwardLoop[index] * reverseForwardAmount
                    * stereoTimingAmount,
                16.0f);
            wet[index] = sanitiseSample(
                wet[index] + crystalAudible * crystalAmount
                    * stereoTimingAmount
                    + reverseForwardAudible[index] * reverseForwardAmount
                    * stereoTimingAmount,
                16.0f);
        }

        // Freeze is a wet-loop capture rather than a request for nominal
        // unity gain inside every mode.  Nominal unity still passes through
        // mode filters, diffusion and nonlinearities, so many characterful
        // presets otherwise lose their loop.  Replaying one complete recent
        // delay cycle makes the hold independent of those losses; the live
        // delay engine continues underneath for a natural release.
        if (freezePlaybackActive && freezeBuffer.getNumSamples() > 1)
        {
            const int freezeBufferLength = freezeBuffer.getNumSamples();
            for (int channel = 0; channel < delayChannelCount; ++channel)
            {
                const auto index = static_cast<size_t>(channel);
                int read = freezeLoopStart[index]
                         + freezeReadPosition[index];
                if (read >= freezeBufferLength)
                    read -= freezeBufferLength;
                const float frozen = freezeBuffer.getSample(channel, read);
                wet[index] = sanitiseSample(
                    wet[index] + freeze * (frozen - wet[index]), 16.0f);
                feedbackWet[index] = sanitiseSample(
                    feedbackWet[index]
                        + freeze * (frozen - feedbackWet[index]),
                    16.0f);

                if (++freezeReadPosition[index] >= freezeLoopLength[index])
                    freezeReadPosition[index] = 0;
            }

            if (! freezeRequested && freeze <= 0.0001f)
                freezePlaybackActive = false;
        }

        if (! freezePlaybackActive && freezeBuffer.getNumSamples() > 1)
        {
            freezeBuffer.setSample(0, freezeCaptureWritePosition, wet[0]);
            freezeBuffer.setSample(1, freezeCaptureWritePosition, wet[1]);
            if (++freezeCaptureWritePosition >= freezeBuffer.getNumSamples())
                freezeCaptureWritePosition = 0;
            freezeCaptureValidSamples = juce::jmin(
                freezeCaptureValidSamples + 1,
                freezeBuffer.getNumSamples() - 1);
        }
        for (int stage = 0; stage < diffusionStageCount; ++stage)
        {
            const auto stageIndex = static_cast<size_t>(stage);
            if (++diffusionWritePosition[stageIndex]
                >= diffusionStageBuffer[stageIndex].getNumSamples())
                diffusionWritePosition[stageIndex] = 0;
        }
        diffusionPhaseA += 0.113 / currentSampleRate;
        diffusionPhaseB += 0.137 / currentSampleRate;
        if (diffusionPhaseA >= 1.0)
            diffusionPhaseA -= 1.0;
        if (diffusionPhaseB >= 1.0)
            diffusionPhaseB -= 1.0;

        const float cross = crossValue.getNextValue();
        const std::array<float, delayChannelCount> loopFeedbackWet
        {
            feedbackWet[0] + multiTapAmount
                * (multiTapRepeatWet[0] - feedbackWet[0]),
            feedbackWet[1] + multiTapAmount
                * (multiTapRepeatWet[1] - feedbackWet[1])
        };
        const std::array<float, delayChannelCount> stereoFeedbackSource
        {
            (1.0f - cross) * loopFeedbackWet[0]
                + cross * loopFeedbackWet[1],
            (1.0f - cross) * loopFeedbackWet[1]
                + cross * loopFeedbackWet[0]
        };
        const std::array<float, delayChannelCount> pingPongLoopInput
        {
            feedback * loopFeedbackWet[1],
            loopFeedbackWet[0]
        };
        constexpr float inverseSqrtTwo = 0.7071067811865475f;
        const std::array<float, delayChannelCount> ratioLoopInput
        {
            feedback * (loopFeedbackWet[0] + loopFeedbackWet[1])
                * inverseSqrtTwo,
            feedback * (loopFeedbackWet[0] - loopFeedbackWet[1])
                * inverseSqrtTwo
        };
        std::array<float, delayChannelCount> feedbackReturn {};
        for (int channel = 0; channel < delayChannelCount; ++channel)
        {
            const auto index = static_cast<size_t>(channel);
            const float stereoLoopInput = feedback
                                        * stereoFeedbackSource[index];
            // In Ping-Pong the first Left-to-Right transfer is direct; FDBK
            // is applied only when Right returns to Left.  The previous
            // cross-feedback approximation multiplied both legs, producing
            // feedback^2 loss on every complete stereo circuit.  Interpolate
            // the two complete topologies so toggling remains click-free.
            const float ratioLoop = stereoLoopInput
                                  + ratioStyle
                                    * (ratioLoopInput[index]
                                       - stereoLoopInput);
            const float loopInput = ratioLoop
                                  + pingPong
                                    * (pingPongLoopInput[index] - ratioLoop);

            // Large downward transposition naturally moves energy out of the
            // audible band. The generic unity-slope tanh below was cancelling
            // that loss above 100%, allowing both Pitch and Pitch Rev to lock
            // very short delays into a permanent sub-bass/DC limit cycle.
            // Restore a small amount of transposition-dependent loop loss only
            // in the regenerative range. At still-higher settings the loop
            // remains capable of intentional self-oscillation.
            const float pitchOctaves = juce::jlimit(
                0.0f, 2.0f,
                std::abs(std::log2(index == 0u
                    ? pitchRatioLeft : pitchRatioRight)));
            const float pitchTrimPosition = juce::jlimit(
                0.0f, 1.0f, (normalFeedback - 1.0f) / 0.067f);
            const float pitchTrimBlend = pitchTrimPosition
                * pitchTrimPosition
                * (3.0f - 2.0f * pitchTrimPosition)
                * (1.0f - freeze);
            const float pitchLoopTrim = 1.0f
                - pitchTrimBlend * pitchOctaves * 0.043f;

            const float digitalReturn = applyFeedbackSaturation(loopInput, 8.0f);
            const float tapeReturn = applyFeedbackSaturation(loopInput, 2.5f);
            const float bbdReturn = applyFeedbackSaturation(loopInput, 1.5f);
            const float hifiReturn = applyFeedbackSaturation(loopInput, 4.5f);
            const float ghostReturn = applyFeedbackSaturation(loopInput, 2.0f);
            const float pitchReturn = applyFeedbackSaturation(
                loopInput * pitchLoopTrim, 2.5f);
            const float reversePitchReturn = applyFeedbackSaturation(
                loopInput * pitchLoopTrim, 2.2f);
            const float lofiReturn = applyFeedbackSaturation(loopInput, 1.35f);
            const float crystalReturn = applyFeedbackSaturation(loopInput, 6.0f);
            const float phaserReturn = applyFeedbackSaturation(loopInput, 2.2f);
            const float lushPitchReturn = applyFeedbackSaturation(loopInput, 2.5f);
            const float chromeTapeReturn = applyFeedbackSaturation(loopInput, 4.0f);
            const float analogReturn = applyFeedbackSaturation(loopInput, 2.2f);
            const float chaosReturn = applyFeedbackSaturation(loopInput, 2.0f);
            const float pureReverseReturn = applyFeedbackSaturation(loopInput, 2.8f);
            const float reverseChaosReturn = applyFeedbackSaturation(loopInput, 1.8f);
            const float reverseForwardReturn = applyFeedbackSaturation(loopInput, 2.8f);
            const float tapeIIReturn = applyFeedbackSaturation(loopInput, 2.8f);
            const float modeReturn = digitalReturn * digitalAmount
                                   + tapeReturn * tapeAmount
                                   + bbdReturn * bbdAmount
                                   + hifiReturn * hifiAmount
                                   + ghostReturn * ghostAmount
                                   + pitchReturn * pitchAmount
                                   + reversePitchReturn * reversePitchAmount
                                   + lofiReturn * lofiAmount
                                   + crystalReturn * crystalAmount
                                   + phaserReturn * phaserAmount
                                   + lushPitchReturn * lushPitchAmount
                                   + chromeTapeReturn * chromeTapeAmount
                                   + analogReturn * analogAmount
                                   + chaosReturn * chaosAmount
                                   + pureReverseReturn * pureReverseAmount
                                   + reverseChaosReturn * reverseChaosAmount
                                   + reverseForwardReturn * reverseForwardAmount
                                   + tapeIIReturn * tapeIIAmount;
            feedbackReturn[index] = modeReturn;
        }
        const std::array<float, delayChannelCount> next
        {
            delayInputL + feedbackReturn[0],
            delayInputR + feedbackReturn[1]
        };
        delayBuffer.setSample(0, writePosition, sanitiseSample(next[0], 16.0f));
        delayBuffer.setSample(1, writePosition, sanitiseSample(next[1], 16.0f));

        const float spread = spreadValue.getNextValue();
        const float wetMid = 0.5f * (wet[0] + wet[1]);
        const float wetSide = 0.5f * (wet[0] - wet[1]) * spread;
        const std::array<float, delayChannelCount> spatialWet
        {
            wetMid + wetSide,
            wetMid - wetSide
        };
        // Normal feedback must not make the whole wet path quieter.  Only the
        // genuinely regenerative region receives fixed output compensation;
        // it is deliberately outside the loop, so it cannot shorten repeats,
        // suppress time-movement pitch effects or create compressor pumping.
        const float regenerativeAmount = juce::jmax(0.0f, feedback - 1.0f);
        const float feedbackOutputGain = 1.0f / (1.0f + regenerativeAmount);
        const std::array<float, delayChannelCount> audibleWet
        {
            applyOutputSafety(spatialWet[0]) * feedbackOutputGain * duckGain,
            applyOutputSafety(spatialWet[1]) * feedbackOutputGain * duckGain
        };
        const float mix = mixValue.getNextValue();
        const float outputGain = outputGainValue.getNextValue();
        const float processedL = outputGain * (inputL * (1.0f - mix) + audibleWet[0] * mix);
        const float processedR = outputGain * (inputR * (1.0f - mix) + audibleWet[1] * mix);
        const float bypass = bypassValue.getNextValue();
        output[0][sample] = sanitiseSample(processedL + (inputL - processedL) * bypass, 64.0f);
        if (activeChannelCount > 1)
            output[1][sample] = sanitiseSample(processedR + (inputR - processedR) * bypass, 64.0f);

        if (++writePosition >= delayLength)
            writePosition = 0;
    }

    if (! distortionTypeCrossfade.isSmoothing())
        previousDistortionType = currentDistortionType;
}

juce::AudioProcessorEditor* AfterglowAudioProcessor::createEditor()
{
    return new AfterglowAudioProcessorEditor(*this);
}

void AfterglowAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    auto state = parameters.copyState();
    state.setProperty("schemaVersion", stateSchemaVersion, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destination);
}

void AfterglowAudioProcessor::setStateInformation(const void* data, int size)
{
    auto xml = getXmlFromBinary(data, size);
    if (xml == nullptr || ! xml->hasTagName(parameters.state.getType()))
        return;

    auto restored = juce::ValueTree::fromXml(*xml);
    if (! migrateStateToCurrentSchema(restored))
        return;

    auto savedState = restored.createCopy();
    const auto presetPath = restored.getProperty(
        "currentPresetPath", juce::String()).toString();
    const juce::File presetFile(presetPath);
    const auto presetsDirectory = getPresetsDirectory();
    const auto presetParent = presetFile.getParentDirectory();
    if (presetFile.existsAsFile()
        && (presetParent == presetsDirectory
            || presetParent.isAChildOf(presetsDirectory)))
    {
        if (auto presetXml = juce::XmlDocument::parse(presetFile))
            if (presetXml->hasTagName(parameters.state.getType()))
            {
                auto presetState = juce::ValueTree::fromXml(*presetXml);
                if (migrateStateToCurrentSchema(presetState))
                    savedState = presetState;
            }
    }

    setSavedParameterState(savedState);
    parameters.replaceState(restored);
    pendingProcessingReset.store(true, std::memory_order_release);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AfterglowAudioProcessor();
}
