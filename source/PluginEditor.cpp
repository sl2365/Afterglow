#include "PluginEditor.h"
#include <BinaryData.h>
#include <algorithm>
#include <cmath>

namespace
{
const juce::Colour knobTrackColour(0xff343940);
const juce::Colour knobBodyColour(0xff2b3036);
const juce::Colour knobLabelColour(0xff8d949e);
const juce::Colour knobValueColour(knobLabelColour.brighter(0.18f));
const juce::Colour controlAccentColour(0xff65cbb9);
const juce::Colour knobIndicatorColour(0xffd2b84f);
const juce::Colour tempoSyncLedColour(0xff55c96b);
const juce::Colour linkLedColour(0xffffad42);
const juce::Colour bipolarLedColour(0xffd96b72);
const juce::Colour unipolarNegativeLedColour(0xff8f68cf);
const juce::Colour powerLedColour(0xff82d16a);
const juce::Colour presetAccentColour(0xff72b9a9);
constexpr auto presetFileExtension = ".agpreset";
constexpr auto sequencerTargetDragPrefix = "afterglow-sequencer-target:";
constexpr int delayModeCount = 21;
constexpr std::array<const char*, 4> tapTimeParameterIds
{
    "tap1Time", "tap2Time", "tap3Time", "tap4Time"
};
constexpr std::array<const char*, 4> tapDivisionParameterIds
{
    "tap1Division", "tap2Division", "tap3Division", "tap4Division"
};
constexpr std::array<const char*, 4> tapPanParameterIds
{
    "tap1Pan", "tap2Pan", "tap3Pan", "tap4Pan"
};
constexpr std::array<const char*, 4> tapLevelParameterIds
{
    "tap1Level", "tap2Level", "tap3Level", "tap4Level"
};
constexpr std::array<const char*, 4> tapEnabledParameterIds
{
    "tap1Enabled", "tap2Enabled", "tap3Enabled", "tap4Enabled"
};

juce::Array<juce::File> presetFilesInFolder(const juce::File& folder)
{
    juce::Array<juce::File> files;
    if (! folder.isDirectory())
        return files;

    folder.findChildFiles(files, juce::File::findFiles, false);
    files.removeIf([](const juce::File& file)
    {
        return ! file.hasFileExtension(presetFileExtension);
    });
    std::sort(files.begin(), files.end(),
              [](const juce::File& first, const juce::File& second)
              {
                  return first.getFileNameWithoutExtension().compareNatural(
                             second.getFileNameWithoutExtension(), true) < 0;
              });
    return files;
}

juce::Path makeStarPath(juce::Rectangle<float> bounds)
{
    juce::Path path;
    const auto centre = bounds.getCentre();
    const float outerRadius = 0.5f * juce::jmin(bounds.getWidth(),
                                                bounds.getHeight());
    const float innerRadius = outerRadius * 0.43f;
    for (int point = 0; point < 10; ++point)
    {
        const float radius = (point % 2 == 0) ? outerRadius : innerRadius;
        const float angle = -juce::MathConstants<float>::halfPi
            + static_cast<float>(point)
                * juce::MathConstants<float>::pi / 5.0f;
        const juce::Point<float> position(
            centre.x + std::cos(angle) * radius,
            centre.y + std::sin(angle) * radius);
        if (point == 0)
            path.startNewSubPath(position);
        else
            path.lineTo(position);
    }
    path.closeSubPath();
    return path;
}

constexpr std::array<const char*, 17> distortionSummaries
{
    "Dense tanh clipping with strong static level compensation",
    "Open arctangent saturation with brighter, gentler harmonics",
    "Asymmetric valve-style soft saturation with even harmonics",
    "Full-wave folding followed by DC-safe filtering",
    "Signal-responsive broken-contact pops over compact saturation",
    "Sharp hard clipping charged with signal-tracked white noise",
    "Cascaded Class-A-style preamp and power-stage overdrive",
    "Uneven frequency-shaped waveshaping with nonlinear ripples",
    "Different positive and negative curves for strong even harmonics",
    "Upper-mid pre-emphasis feeding sharp ridged saturation",
    "Bit-depth and sample-rate reduction with quiet deterministic dither",
    "Short resonant comb feedback feeding bounded saturation",
    "Compressed wavefolding with a strained asymmetric edge",
    "Sliced, upper-mid-emphasised clipping with a metallic blade",
    "Bright high-band excitation over restrained clean saturation",
    "Clean flat-threshold clipping without added noise or tone filtering",
    "High-gain asymmetric fuzz with compressed sustain and a restrained top end"
};
constexpr std::array<const char*, 17> distortionDetails
{
    "DIST increases density and compression without repeatedly re-driving feedback.",
    "DIST moves smoothly from clean into open saturation without darkening the top end.",
    "DIST moves from clean warmth into rounded compression and asymmetric colour.",
    "DIST moves from the dry signal into octave-rich electronic crunch.",
    "DIST increases crack density, broken-contact edge and compact grit.",
    "DIST increases electrical bite and noise while keeping the input bounded.",
    "DIST moves from warm edge into raw drive with a restrained amp-like top end.",
    "DIST increases shifting ridges, asymmetric grit and midrange emphasis.",
    "DIST progressively separates the two polarities without leaving a DC offset.",
    "DIST intensifies high-frequency teeth and narrow, cutting harmonic ridges.",
    "DIST lowers effective resolution and update rate for increasingly coarse digital texture.",
    "DIST shortens and strengthens the resonant path for metallic, pitched colour.",
    "DIST pushes a smooth curve into folded, tense and increasingly unstable harmonics.",
    "DIST widens the dead zone and sharpens the bright, sliced transient edge.",
    "DIST adds clear upper harmonics and presence while leaving the low band comparatively intact.",
    "DIST lowers the clipping threshold for progressively harder, squared-off transients.",
    "DIST increases asymmetric clipping, dense sustain and dark post-clip fuzz colour."
};

juce::Colour modeColourForId(int modeId)
{
    switch (modeId)
    {
        case 2:  return juce::Colour(0xff3d6406); // Tape I
        case 3:  return juce::Colour(0xff642346); // Brigade
        case 4:  return juce::Colour(0xffc14521); // HiFi
        case 5:  return juce::Colour(0xff82436e); // Ghost
        case 6:  return juce::Colour(0xffc23d82); // Pitch
        case 7:  return juce::Colour(0xffb14fc8); // RevPitch
        case 8:  return juce::Colour(0xff159b91); // LoFi
        case 9:  return juce::Colour(0xff258e9d); // Crystal
        case 10: return juce::Colour(0xff5968d8); // Phaser
        case 11: return juce::Colour(0xffd05a9b); // LushPitch
        case 12: return juce::Colour(0xff7f9ca7); // Chrome Tape
        case 13: return juce::Colour(0xffa46c3f); // Analogue
        case 14: return juce::Colour(0xffd65a3a); // Chaos
        case 15: return juce::Colour(0xff4168b5); // Pure Reverse
        case 16: return juce::Colour(0xffb04378); // Reverse Chaos
        case 17: return juce::Colour(0xff59476b); // Forward Rev
        case 18: return juce::Colour(0xff6f8f36); // Tape II
        case 19: return juce::Colour(0xff7a6138); // Colour
        case 20: return juce::Colour(0xff8b5f68); // Twin Tape
        case 21: return juce::Colour(0xff6e5fa8); // Grain
        default: return juce::Colour(0xff1479b8); // Digital
    }
}

struct InterfaceSettings
{
    int scalePercent = 100;
    double themePercent = 100.0;
    bool sequencerOpen = false;
    bool sequencerAnimate = false;
};

InterfaceSettings loadInterfaceSettings()
{
    InterfaceSettings settings;
    const auto file = AfterglowAudioProcessor::getSettingsDirectory()
        .getChildFile("Settings.ini");
    if (! file.existsAsFile())
        return settings;

    juce::StringArray lines;
    lines.addLines(file.loadFileAsString());
    bool inGuiSection = false;
    for (auto line : lines)
    {
        line = line.trim();
        if (line.startsWithChar('['))
        {
            inGuiSection = line.equalsIgnoreCase("[GUI]");
            continue;
        }
        if (! inGuiSection)
            continue;

        const int separator = line.indexOfChar('=');
        if (separator <= 0)
            continue;
        const auto key = line.substring(0, separator).trim();
        const auto value = line.substring(separator + 1).trim();
        if (key.equalsIgnoreCase("ScalePercent"))
            settings.scalePercent = juce::jlimit(75, 300, value.getIntValue());
        else if (key.equalsIgnoreCase("ThemePercent"))
            settings.themePercent = juce::jlimit(0.0, 100.0,
                                                  static_cast<double>(value.getFloatValue()));
        else if (key.equalsIgnoreCase("SequencerOpen"))
            settings.sequencerOpen = value.getIntValue() != 0;
        else if (key.equalsIgnoreCase("SequencerAnimate"))
            settings.sequencerAnimate = value.getIntValue() != 0;
    }
    return settings;
}

class PresetPopupList final : public juce::PopupMenu::CustomComponent,
                              private juce::ListBoxModel
{
public:
    PresetPopupList(juce::Array<juce::File> filesToUse,
                    const juce::File& currentFile,
                    std::function<void(const juce::File&)> selectionCallback)
        : juce::PopupMenu::CustomComponent(false),
          files(std::move(filesToUse)),
          onSelected(std::move(selectionCallback)),
          list("Preset quick list", this)
    {
        list.setRowHeight(rowHeight);
        list.setColour(juce::ListBox::backgroundColourId,
                       juce::Colour(0xff14181d));
        list.setColour(juce::ListBox::outlineColourId,
                       juce::Colours::transparentBlack);
        list.setOutlineThickness(0);
        addAndMakeVisible(list);
        list.updateContent();

        const int currentRow = files.indexOf(currentFile);
        if (currentRow >= 0)
        {
            list.selectRow(currentRow, false, false);
            rowToReveal = currentRow;
        }
    }

    void getIdealSize(int& idealWidth, int& idealHeight) override
    {
        idealWidth = 224;
        idealHeight = juce::jmax(1, juce::jmin(maximumVisibleRows,
                                               files.size())) * rowHeight + 4;
    }

    void resized() override
    {
        // Leave the same two-pixel frame visible as the Mode and Type menus.
        list.setBounds(getLocalBounds().reduced(2));
        if (rowToReveal >= 0)
        {
            list.scrollToEnsureRowIsOnscreen(rowToReveal);
            rowToReveal = -1;
        }
    }

private:
    int getNumRows() override
    {
        return files.size();
    }

    void paintListBoxItem(int row, juce::Graphics& g, int width, int height,
                          bool rowIsSelected) override
    {
        if (! juce::isPositiveAndBelow(row, files.size()))
            return;

        if (rowIsSelected)
        {
            g.setColour(juce::Colour(0xff2a3037));
            g.fillRoundedRectangle(3.0f, 1.0f,
                                   static_cast<float>(width - 6),
                                   static_cast<float>(height - 2), 2.0f);
        }

        g.setColour(rowIsSelected
            ? knobIndicatorColour.brighter(0.28f)
            : knobValueColour);
        g.setFont(getLookAndFeel().getPopupMenuFont());
        g.drawFittedText(files.getReference(row).getFileNameWithoutExtension(),
                         8, 0, juce::jmax(0, width - 16), height,
                         juce::Justification::centredLeft, 1, 0.75f);
    }

    void listBoxItemClicked(int row, const juce::MouseEvent&) override
    {
        chooseRow(row);
    }

    void returnKeyPressed(int row) override
    {
        chooseRow(row);
    }

    void chooseRow(int row)
    {
        if (! juce::isPositiveAndBelow(row, files.size()))
            return;

        if (onSelected)
            onSelected(files.getReference(row));
        triggerMenuItem();
    }

    static constexpr int rowHeight = 20;
    static constexpr int maximumVisibleRows = 20;
    juce::Array<juce::File> files;
    std::function<void(const juce::File&)> onSelected;
    juce::ListBox list;
    int rowToReveal = -1;
};
}

class AfterglowAudioProcessorEditor::FolderTreeItem final
    : public juce::TreeViewItem
{
public:
    FolderTreeItem(juce::File directoryToUse,
                   std::function<void(const juce::File&, bool)>
                       selectionCallback,
                   std::function<void(const juce::File&, bool,
                                      juce::Point<int>)> contextCallback,
                   bool favouriteItem = false,
                   int depth = 0)
        : directory(std::move(directoryToUse)),
          onSelected(std::move(selectionCallback)),
          onContext(std::move(contextCallback)),
          isFavouriteItem(favouriteItem)
    {
        if (isFavouriteItem || depth >= 32)
            return;

        if (depth == 0)
            addSubItem(new FolderTreeItem(
                directory, onSelected, onContext, true, depth + 1));

        juce::Array<juce::File> childDirectories;
        directory.findChildFiles(childDirectories, juce::File::findDirectories,
                                 false);
        std::sort(childDirectories.begin(), childDirectories.end(),
                  [](const juce::File& first, const juce::File& second)
                  {
                      return first.getFileName().compareNatural(
                                 second.getFileName(), true) < 0;
                  });
        for (const auto& child : childDirectories)
            if (! child.isSymbolicLink())
                addSubItem(new FolderTreeItem(
                    child, onSelected, onContext, false, depth + 1));
    }

    bool mightContainSubItems() override
    {
        return getNumSubItems() > 0;
    }

    juce::String getUniqueName() const override
    {
        return isFavouriteItem
            ? directory.getFullPathName() + "::Favourites"
            : directory.getFullPathName();
    }

    int getItemHeight() const override
    {
        return 22;
    }

    void paintItem(juce::Graphics& g, int width, int height) override
    {
        if (isSelected())
        {
            g.setColour(presetAccentColour.withAlpha(0.30f));
            g.fillRoundedRectangle(0.0f, 1.0f, static_cast<float>(width),
                                   static_cast<float>(height - 2), 3.0f);
        }

        if (isFavouriteItem)
        {
            g.setColour(isSelected() ? knobIndicatorColour
                                     : knobLabelColour.brighter(0.12f));
            g.fillPath(makeStarPath({ 2.0f,
                static_cast<float>(height) * 0.5f - 6.0f,
                12.0f, 12.0f }));
        }
        else
        {
            const auto folderBounds = juce::Rectangle<float>(
                2.0f, static_cast<float>(height) * 0.5f - 5.0f,
                13.0f, 10.0f);
            g.setColour(isSelected() ? knobIndicatorColour
                                     : knobLabelColour.brighter(0.12f));
            g.fillRoundedRectangle(folderBounds, 1.5f);
            g.fillRect(folderBounds.getX() + 1.5f,
                       folderBounds.getY() - 2.0f, 5.0f, 3.0f);
        }
        g.setColour(isSelected() ? juce::Colours::white
                                 : knobValueColour);
        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        g.drawFittedText(isFavouriteItem ? "Favourites"
                                         : directory.getFileName(),
                         21, 0, juce::jmax(0, width - 24), height,
                         juce::Justification::centredLeft, 1);
    }

    void itemSelectionChanged(bool isNowSelected) override
    {
        if (isNowSelected && onSelected)
            onSelected(directory, isFavouriteItem);
        repaintItem();
    }

    void itemClicked(const juce::MouseEvent& event) override
    {
        if (event.mods.isPopupMenu() && onContext)
            onContext(directory, isFavouriteItem,
                      event.getScreenPosition());
    }

    FolderTreeItem* findItemForDirectory(const juce::File& target,
                                         bool findFavourites = false)
    {
        if (isFavouriteItem == findFavourites
            && (findFavourites || directory == target))
            return this;

        for (int index = 0; index < getNumSubItems(); ++index)
            if (auto* child = dynamic_cast<FolderTreeItem*>(getSubItem(index)))
                if (auto* match = child->findItemForDirectory(
                        target, findFavourites))
                    return match;
        return nullptr;
    }

private:
    juce::File directory;
    std::function<void(const juce::File&, bool)> onSelected;
    std::function<void(const juce::File&, bool, juce::Point<int>)> onContext;
    bool isFavouriteItem = false;
};

namespace
{
void setParameterActualValue(juce::RangedAudioParameter* parameter,
                             float actualValue)
{
    if (parameter == nullptr)
        return;
    parameter->beginChangeGesture();
    parameter->setValueNotifyingHost(parameter->convertTo0to1(actualValue));
    parameter->endChangeGesture();
}
}

class AfterglowAudioProcessorEditor::SequencerPanel final
    : public juce::Component
{
public:
    class StepGrid final : public juce::Component,
                           public juce::SettableTooltipClient
    {
    public:
        StepGrid(AfterglowAudioProcessor& audioProcessor,
                 LookAndFeel& sharedLookAndFeelToUse,
                 LookAndFeel& compactLookAndFeelToUse)
            : processor(audioProcessor),
              sharedPopupLookAndFeel(sharedLookAndFeelToUse),
              compactPopupLookAndFeel(compactLookAndFeelToUse)
        {
            for (int step = 0; step < afterglow::sequencer::stepCount; ++step)
            {
                steps[static_cast<size_t>(step)]
                    = processor.parameters.getParameter(
                        afterglow::sequencer::stepParameterId(step));
                gateSteps[static_cast<size_t>(step)]
                    = processor.parameters.getParameter(
                        afterglow::sequencer::gateStepParameterId(step));
            }
            start = processor.parameters.getRawParameterValue("sequencerStart");
            end = processor.parameters.getRawParameterValue("sequencerEnd");
            peak = processor.parameters.getRawParameterValue("sequencerPeak");
            move = processor.parameters.getRawParameterValue("sequencerMove");
            attack = processor.parameters.getRawParameterValue(
                "sequencerAttack");
            release = processor.parameters.getRawParameterValue(
                "sequencerRelease");
            setMouseCursor(juce::MouseCursor::CrosshairCursor);
            setTooltip("Click or drag vertically to set the sequencer steps");
        }

        void setAccent(juce::Colour colour)
        {
            accent = colour;
            repaint();
        }

        void setPolarityMode(int newMode)
        {
            newMode = juce::jlimit(0, 2, newMode);
            if (polarityMode == newMode)
                return;

            polarityMode = newMode;
            repaint();
        }

        void paint(juce::Graphics& graphics) override
        {
            const auto whole = getLocalBounds().toFloat();
            graphics.setColour(juce::Colour(0xff10151a));
            graphics.fillRoundedRectangle(whole, 5.0f);
            const auto graph = graphBounds();
            const float columnWidth = graph.getWidth()
                                    / afterglow::sequencer::stepCount;
            const bool isBipolar = polarityMode == 1;
            const bool isNegative = polarityMode == 2;
            const int first = juce::jlimit(0, 30,
                juce::roundToInt(start != nullptr ? start->load() : 1.0f) - 1);
            const int last = juce::jlimit(first + 1, 31,
                juce::roundToInt(end != nullptr ? end->load() : 32.0f) - 1);
            const int active = processor.getActiveSequencerStep();

            std::array<float, afterglow::sequencer::stepCount> values {};
            float patternMinimum = 1.0f;
            float patternMaximum = 0.0f;
            for (int step = 0; step < afterglow::sequencer::stepCount; ++step)
            {
                const auto index = static_cast<size_t>(step);
                values[index] = steps[index] != nullptr
                    ? steps[index]->convertFrom0to1(steps[index]->getValue())
                    : 1.0f;
                patternMinimum = juce::jmin(patternMinimum, values[index]);
                patternMaximum = juce::jmax(patternMaximum, values[index]);
            }
            const float peakValue = peak != nullptr ? peak->load() : 0.0f;
            const float moveValue = move != nullptr ? move->load() : 0.0f;
            for (auto& value : values)
                value = afterglow::sequencer::applyDepthOffset(
                    value, patternMinimum, patternMaximum,
                    peakValue, moveValue, isBipolar);

            const auto displayedMagnitude = [isBipolar](float canonical)
            {
                return isBipolar ? juce::jlimit(0.0f, 1.0f, canonical)
                    : juce::jlimit(0.0f, 1.0f, 2.0f * canonical - 1.0f);
            };
            const auto displayY = [graph, isBipolar, isNegative,
                                   &displayedMagnitude](float canonical)
            {
                const float displayed = displayedMagnitude(canonical);
                if (isBipolar)
                    return graph.getBottom() - graph.getHeight() * displayed;
                return isNegative ? graph.getY()
                                      + graph.getHeight() * displayed
                                  : graph.getBottom()
                                      - graph.getHeight() * displayed;
            };
            if (isBipolar)
            {
                graphics.setColour(accent.withAlpha(0.22f));
                graphics.drawHorizontalLine(
                    juce::roundToInt(graph.getCentreY()), graph.getX(),
                    graph.getRight());
            }

            graphics.setFont(juce::FontOptions(10.5f, juce::Font::bold));
            for (int step = 0; step < afterglow::sequencer::stepCount; ++step)
            {
                const float x = graph.getX() + columnWidth * step;
                const auto cell = juce::Rectangle<float>(
                    x, graph.getY(), columnWidth, graph.getHeight());
                if ((step / 4) % 2 != 0)
                {
                    graphics.setColour(juce::Colour(0x0dffffff));
                    graphics.fillRect(cell);
                }
                if (step == active)
                {
                    graphics.setColour(accent.withAlpha(0.22f));
                    graphics.fillRect(cell);
                }
                const float displayed = displayedMagnitude(
                    values[static_cast<size_t>(step)]);
                graphics.setColour(accent.withAlpha(
                    step == active ? 0.95f : 0.68f));
                if (isBipolar)
                {
                    const float y = graph.getBottom()
                                  - graph.getHeight() * displayed;
                    graphics.fillRect(x + 1.0f,
                        juce::jmin(y, graph.getCentreY()),
                        juce::jmax(1.0f, columnWidth - 2.0f),
                        std::abs(y - graph.getCentreY()));
                }
                else
                {
                    const float y = displayY(values[static_cast<size_t>(step)]);
                    graphics.fillRect(x + 1.0f,
                        isNegative ? graph.getY() : y,
                        juce::jmax(1.0f, columnWidth - 2.0f),
                        juce::jmax(0.0f, graph.getHeight() * displayed));
                }
                graphics.setColour(juce::Colour(0x553b4650));
                graphics.drawVerticalLine(juce::roundToInt(x + columnWidth),
                                           graph.getY(), graph.getBottom());
                if (step % 4 == 0)
                {
                    graphics.setColour(knobLabelColour);
                    graphics.drawText(juce::String(step + 1),
                        juce::roundToInt(x), 0,
                        juce::roundToInt(columnWidth * 4.0f), 12,
                        juce::Justification::centredLeft);
                }
            }

            const float attackValue = juce::jlimit(
                0.0f, 1.0f, attack != nullptr ? attack->load() : 0.0f);
            const float releaseValue = juce::jlimit(
                0.0f, 1.0f, release != nullptr ? release->load() : 0.0f);
            juce::Path envelope;
            constexpr int pointsPerStep = 16;
            for (int step = 0; step < afterglow::sequencer::stepCount; ++step)
            {
                const int previous = (step - 1
                    + afterglow::sequencer::stepCount)
                    % afterglow::sequencer::stepCount;
                for (int point = 0; point <= pointsPerStep; ++point)
                {
                    const float position = static_cast<float>(point)
                                         / static_cast<float>(pointsPerStep);
                    const float value = afterglow::sequencer::transition(
                        values[static_cast<size_t>(previous)],
                        values[static_cast<size_t>(step)], position,
                        attackValue, releaseValue);
                    const float x = graph.getX() + columnWidth
                        * (static_cast<float>(step) + position);
                    const float y = displayY(value);
                    if (step == 0 && point == 0)
                        envelope.startNewSubPath(x, y);
                    else
                        envelope.lineTo(x, y);
                }
            }
            graphics.setColour(juce::Colour(0xb0000000));
            graphics.strokePath(envelope, juce::PathStrokeType(3.2f));
            graphics.setColour(accent.brighter(0.50f));
            graphics.strokePath(envelope, juce::PathStrokeType(1.35f));
            for (int step = 0; step < afterglow::sequencer::stepCount; ++step)
            {
                if (step >= first && step <= last)
                    continue;
                const float x = graph.getX() + columnWidth * step;
                graphics.setColour(juce::Colour(0xb0101419));
                graphics.fillRect(x, graph.getY(), columnWidth,
                                  graph.getHeight());
            }
            graphics.setColour(accent.withAlpha(0.72f));
            graphics.drawRoundedRectangle(graph, 4.0f, 1.1f);

            const auto gateRow = gateBounds();
            const float gateColumnWidth = gateRow.getWidth()
                                        / afterglow::sequencer::stepCount;
            for (int step = 0; step < afterglow::sequencer::stepCount; ++step)
            {
                const auto index = static_cast<size_t>(step);
                const float x = gateRow.getX() + gateColumnWidth * step;
                const auto cell = juce::Rectangle<float>(
                    x + 1.0f, gateRow.getY(),
                    juce::jmax(1.0f, gateColumnWidth - 2.0f),
                    gateRow.getHeight());
                const auto mode = gateSteps[index] != nullptr
                    ? afterglow::sequencer::gateStepModeFromChoice(
                        gateSteps[index]->convertFrom0to1(
                            gateSteps[index]->getValue()))
                    : afterglow::sequencer::GateStepMode::rest;
                const auto cellAccent = accent.withAlpha(
                    step == active ? 0.95f : 0.72f);

                graphics.setColour(juce::Colour(0xff111820));
                graphics.fillRoundedRectangle(cell, 2.0f);
                if (mode == afterglow::sequencer::GateStepMode::shortStep)
                {
                    auto fill = cell.reduced(2.0f);
                    fill.setWidth(fill.getWidth() * 0.5f);
                    graphics.setColour(cellAccent);
                    graphics.fillRoundedRectangle(fill, 1.5f);
                }
                else if (mode == afterglow::sequencer::GateStepMode::longStep
                         || mode == afterglow::sequencer::GateStepMode::rest)
                {
                    graphics.setColour(cellAccent);
                    graphics.fillRoundedRectangle(cell.reduced(2.0f), 1.5f);
                    if (mode == afterglow::sequencer::GateStepMode::rest)
                    {
                        const auto centre = cell.getCentre();
                        const float halfWidth = cell.getWidth() * 0.22f;
                        const float halfHeight = cell.getHeight() * 0.19f;
                        juce::Path arrow;
                        arrow.startNewSubPath(centre.x - halfWidth, centre.y);
                        arrow.lineTo(centre.x + halfWidth, centre.y);
                        arrow.startNewSubPath(centre.x + halfWidth, centre.y);
                        arrow.lineTo(centre.x + halfWidth - halfHeight,
                                     centre.y - halfHeight);
                        arrow.startNewSubPath(centre.x + halfWidth, centre.y);
                        arrow.lineTo(centre.x + halfWidth - halfHeight,
                                     centre.y + halfHeight);
                        graphics.setColour(juce::Colour(0xff10151a)
                                               .withAlpha(0.88f));
                        graphics.strokePath(
                            arrow, juce::PathStrokeType(1.25f));
                    }
                }

                graphics.setColour(
                    mode == afterglow::sequencer::GateStepMode::off
                        ? knobTrackColour : cellAccent.brighter(0.18f));
                graphics.drawRoundedRectangle(
                    cell, 2.0f, step == active ? 1.4f : 0.8f);
                if (step < first || step > last)
                {
                    graphics.setColour(juce::Colour(0xb0101419));
                    graphics.fillRoundedRectangle(cell, 2.0f);
                }
            }
        }

        void mouseDown(const juce::MouseEvent& event) override
        {
            if (event.mods.isPopupMenu())
            {
                if (gateBounds().contains(event.position))
                    showGateMenu();
                else
                    showPatternMenu();
                return;
            }
            lastEditedCell = -1;
            editingGateRow = gateBounds().contains(event.position);
            if (editingGateRow)
            {
                cycleGateMode(event.position);
                return;
            }
            editAt(event.position);
        }

        void mouseDrag(const juce::MouseEvent& event) override
        {
            if (editingGateRow)
                return;
            editAt(event.position);
        }

        void mouseUp(const juce::MouseEvent&) override
        {
            lastEditedCell = -1;
            editingGateRow = false;
        }

        void applyWaveform(afterglow::sequencer::Waveform waveform)
        {
            const bool isBipolar = polarityMode == 1;
            const auto values = afterglow::sequencer::makeWaveform(
                waveform, isBipolar);
            for (int step = 0; step < afterglow::sequencer::stepCount; ++step)
                setParameterActualValue(steps[static_cast<size_t>(step)],
                                        values[static_cast<size_t>(step)]);
            repaint();
        }

        void nudge(int direction)
        {
            std::array<float, afterglow::sequencer::stepCount> values {};
            for (int step = 0; step < afterglow::sequencer::stepCount; ++step)
            {
                const auto index = static_cast<size_t>(step);
                values[index] = steps[index]->convertFrom0to1(
                    steps[index]->getValue());
            }
            for (int step = 0; step < afterglow::sequencer::stepCount; ++step)
            {
                const int source = (step - direction
                    + afterglow::sequencer::stepCount)
                    % afterglow::sequencer::stepCount;
                setParameterActualValue(steps[static_cast<size_t>(step)],
                                        values[static_cast<size_t>(source)]);
            }
            repaint();
        }

        void nudgeGates(int direction)
        {
            std::array<float, afterglow::sequencer::stepCount> values {};
            for (int step = 0; step < afterglow::sequencer::stepCount; ++step)
            {
                const auto index = static_cast<size_t>(step);
                values[index] = gateSteps[index]->convertFrom0to1(
                    gateSteps[index]->getValue());
            }
            for (int step = 0; step < afterglow::sequencer::stepCount; ++step)
            {
                const int source = (step - direction
                    + afterglow::sequencer::stepCount)
                    % afterglow::sequencer::stepCount;
                setParameterActualValue(gateSteps[static_cast<size_t>(step)],
                                        values[static_cast<size_t>(source)]);
            }
            repaint();
        }

        std::function<void()> onSaveRequested;

    private:
        juce::Rectangle<float> graphBounds() const
        {
            const auto gate = gateBounds();
            auto graph = getLocalBounds().toFloat().reduced(5.0f)
                .withTrimmedTop(12.0f);
            graph.setBottom(gate.getY() - 4.0f);
            return graph;
        }

        juce::Rectangle<float> gateBounds() const
        {
            const auto inner = getLocalBounds().toFloat().reduced(5.0f);
            const float columnWidth = inner.getWidth()
                                    / afterglow::sequencer::stepCount;
            const float height = juce::jmax(
                8.0f, std::floor(columnWidth * 0.60f));
            return { inner.getX(), inner.getBottom() - height,
                     inner.getWidth(), height };
        }

        void cycleGateMode(juce::Point<float> position)
        {
            const auto row = gateBounds();
            const int step = juce::jlimit(
                0, afterglow::sequencer::stepCount - 1,
                static_cast<int>(std::floor(
                    (position.x - row.getX()) / row.getWidth()
                        * afterglow::sequencer::stepCount)));
            auto* parameter = gateSteps[static_cast<size_t>(step)];
            if (parameter == nullptr)
                return;
            const int current = static_cast<int>(
                afterglow::sequencer::gateStepModeFromChoice(
                    parameter->convertFrom0to1(parameter->getValue())));
            const int next = (current + 1)
                % (static_cast<int>(
                    afterglow::sequencer::GateStepMode::rest) + 1);
            setParameterActualValue(parameter, static_cast<float>(next));
            repaint();
        }

        void editAt(juce::Point<float> position)
        {
            const auto graph = graphBounds();
            if (! graph.contains(position))
                return;
            const float column = (position.x - graph.getX()) / graph.getWidth()
                               * afterglow::sequencer::stepCount;
            const int step = juce::jlimit(0,
                afterglow::sequencer::stepCount - 1,
                static_cast<int>(std::floor(column)));
            const bool isBipolar = polarityMode == 1;
            const bool isNegative = polarityMode == 2;
            const float displayed = isBipolar
                ? juce::jlimit(0.0f, 1.0f,
                    (graph.getBottom() - position.y) / graph.getHeight())
                : juce::jlimit(0.0f, 1.0f,
                    (isNegative ? position.y - graph.getY()
                                : graph.getBottom() - position.y)
                        / graph.getHeight());
            const float canonical = isBipolar
                ? displayed : 0.5f + 0.5f * displayed;
            setParameterActualValue(steps[static_cast<size_t>(step)],
                                    canonical);
            lastEditedCell = step;
            repaint();
        }

        void showPatternMenu()
        {
            compactPopupLookAndFeel.setPopupBorderColour(
                sharedPopupLookAndFeel.getPopupBorderColour());
            juce::PopupMenu menu;
            menu.setLookAndFeel(&compactPopupLookAndFeel);
            menu.addItem(1, "Zero");
            menu.addItem(2, "Max");
            menu.addItem(3, "Min");
            menu.addItem(4, "Random");
            menu.addSeparator();
            menu.addItem(5, "Reset");
            menu.addItem(6, "Save Sequence...");
            menu.addSeparator();
            menu.addItem(7, "Copy");
            menu.addItem(8, "Paste", clipboardValid);
            const juce::Component::SafePointer<StepGrid> safeThis(this);
            menu.showMenuAsync(
                juce::PopupMenu::Options()
                    .withTargetComponent(this)
                    .withMinimumWidth(128)
                    .withStandardItemHeight(20),
                [safeThis](int result)
                {
                    if (safeThis == nullptr || result == 0)
                        return;
                    auto& grid = *safeThis;
                    const bool isBipolar = grid.polarityMode == 1;
                    if (result >= 1 && result <= 4)
                    {
                        juce::Random random;
                        for (auto* parameter : grid.steps)
                        {
                            const float value = result == 1 ? 0.5f
                                : result == 2 ? 1.0f
                                : result == 3 ? (isBipolar ? 0.0f : 0.5f)
                                : (isBipolar ? random.nextFloat()
                                             : 0.5f
                                                 + 0.5f * random.nextFloat());
                            setParameterActualValue(parameter, value);
                        }
                    }
                    else if (result == 5)
                    {
                        for (int step = 0;
                             step < afterglow::sequencer::stepCount; ++step)
                        {
                            auto* parameter = grid.steps[static_cast<size_t>(step)];
                            const auto saved = grid.processor.getSavedParameterValue(
                                afterglow::sequencer::stepParameterId(step));
                            const float value = saved.has_value()
                                ? *saved
                                : parameter->convertFrom0to1(
                                      parameter->getDefaultValue());
                            setParameterActualValue(parameter, value);
                        }
                    }
                    else if (result == 6
                             && grid.onSaveRequested != nullptr)
                    {
                        grid.onSaveRequested();
                    }
                    else if (result == 7)
                    {
                        for (size_t index = 0; index < grid.steps.size(); ++index)
                            clipboard[index]
                                = grid.steps[index]->convertFrom0to1(
                                    grid.steps[index]->getValue());
                        clipboardValid = true;
                    }
                    else if (result == 8 && clipboardValid)
                    {
                        for (size_t index = 0; index < grid.steps.size(); ++index)
                            setParameterActualValue(grid.steps[index],
                                                    clipboard[index]);
                    }
                    grid.repaint();
                });
        }

        void showGateMenu()
        {
            compactPopupLookAndFeel.setPopupBorderColour(
                sharedPopupLookAndFeel.getPopupBorderColour());
            juce::PopupMenu menu;
            menu.setLookAndFeel(&compactPopupLookAndFeel);
            menu.addItem(1, "Short");
            menu.addItem(2, "Long");
            menu.addItem(3, "Rest");
            menu.addItem(4, "Off");
            menu.addItem(5, "Random");
            menu.addSeparator();
            menu.addItem(6, "Reset");
            menu.addSeparator();
            menu.addItem(7, "Copy");
            menu.addItem(8, "Paste", gateClipboardValid);
            const juce::Component::SafePointer<StepGrid> safeThis(this);
            menu.showMenuAsync(
                juce::PopupMenu::Options()
                    .withTargetComponent(this)
                    .withMinimumWidth(128)
                    .withStandardItemHeight(20),
                [safeThis](int result)
                {
                    if (safeThis == nullptr || result == 0)
                        return;
                    auto& grid = *safeThis;
                    if (result >= 1 && result <= 4)
                    {
                        const std::array<afterglow::sequencer::GateStepMode, 4>
                            modes
                            {
                                afterglow::sequencer::GateStepMode::shortStep,
                                afterglow::sequencer::GateStepMode::longStep,
                                afterglow::sequencer::GateStepMode::rest,
                                afterglow::sequencer::GateStepMode::off
                            };
                        const auto mode = modes[static_cast<size_t>(result - 1)];
                        for (auto* parameter : grid.gateSteps)
                            setParameterActualValue(
                                parameter, static_cast<float>(mode));
                    }
                    else if (result == 5)
                    {
                        juce::Random random;
                        for (auto* parameter : grid.gateSteps)
                            setParameterActualValue(parameter,
                                static_cast<float>(random.nextInt(4)));
                    }
                    else if (result == 6)
                    {
                        for (int step = 0;
                             step < afterglow::sequencer::stepCount; ++step)
                        {
                            auto* parameter = grid.gateSteps[
                                static_cast<size_t>(step)];
                            const auto saved = grid.processor
                                .getSavedParameterValue(
                                    afterglow::sequencer::gateStepParameterId(
                                        step));
                            const float value = saved.has_value()
                                ? *saved
                                : parameter->convertFrom0to1(
                                      parameter->getDefaultValue());
                            setParameterActualValue(parameter, value);
                        }
                    }
                    else if (result == 7)
                    {
                        for (size_t index = 0;
                             index < grid.gateSteps.size(); ++index)
                            gateClipboard[index] = grid.gateSteps[index]
                                ->convertFrom0to1(
                                    grid.gateSteps[index]->getValue());
                        gateClipboardValid = true;
                    }
                    else if (result == 8 && gateClipboardValid)
                    {
                        for (size_t index = 0;
                             index < grid.gateSteps.size(); ++index)
                            setParameterActualValue(grid.gateSteps[index],
                                                    gateClipboard[index]);
                    }
                    grid.repaint();
                });
        }

        AfterglowAudioProcessor& processor;
        LookAndFeel& sharedPopupLookAndFeel;
        LookAndFeel& compactPopupLookAndFeel;
        std::array<juce::RangedAudioParameter*,
                   afterglow::sequencer::stepCount> steps {};
        std::array<juce::RangedAudioParameter*,
                   afterglow::sequencer::stepCount> gateSteps {};
        std::atomic<float>* start = nullptr;
        std::atomic<float>* end = nullptr;
        std::atomic<float>* peak = nullptr;
        std::atomic<float>* move = nullptr;
        std::atomic<float>* attack = nullptr;
        std::atomic<float>* release = nullptr;
        juce::Colour accent { controlAccentColour };
        int polarityMode = 0;
        int lastEditedCell = -1;
        bool editingGateRow = false;
        inline static std::array<float,
                                 afterglow::sequencer::stepCount> clipboard {};
        inline static bool clipboardValid = false;
        inline static std::array<float,
                                 afterglow::sequencer::stepCount>
            gateClipboard {};
        inline static bool gateClipboardValid = false;
    };

    class TargetList final : public juce::Component,
                             public juce::DragAndDropTarget,
                             public juce::SettableTooltipClient
    {
    public:
        TargetList(AfterglowAudioProcessor& audioProcessor,
                   std::function<void(const juce::String&)> helpCallback)
            : processor(audioProcessor), showHelp(std::move(helpCallback))
        {
            mode = processor.parameters.getRawParameterValue("mode");
            for (int slot = 0; slot < afterglow::sequencer::targetSlotCount;
                 ++slot)
            {
                choices[static_cast<size_t>(slot)]
                    = processor.parameters.getParameter(
                        afterglow::sequencer::targetParameterId(slot));
                enabled[static_cast<size_t>(slot)]
                    = processor.parameters.getParameter(
                        afterglow::sequencer::targetEnabledParameterId(slot));
                depths[static_cast<size_t>(slot)]
                    = processor.parameters.getParameter(
                        afterglow::sequencer::targetDepthParameterId(slot));
            }
            setTooltip("Drop an Afterglow knob label here. The small knob sets that target's individual modulation depth.");
        }

        bool isInterestedInDragSource(const SourceDetails& details) override
        {
            const auto description = details.description.toString();
            return description.startsWith(sequencerTargetDragPrefix)
                && afterglow::sequencer::targetChoiceForParameter(
                    description.fromFirstOccurrenceOf(
                        sequencerTargetDragPrefix, false, false)) > 0;
        }

        void itemDragEnter(const SourceDetails&) override
        {
            dragOver = true;
            repaint();
        }

        void itemDragExit(const SourceDetails&) override
        {
            dragOver = false;
            repaint();
        }

        void itemDropped(const SourceDetails& details) override
        {
            dragOver = false;
            const int requested = afterglow::sequencer::targetChoiceForParameter(
                details.description.toString().fromFirstOccurrenceOf(
                    sequencerTargetDragPrefix, false, false));
            if (requested <= 0)
                return;
            int freeSlot = -1;
            for (int slot = 0; slot < afterglow::sequencer::targetSlotCount;
                 ++slot)
            {
                const int existing = choiceForSlot(slot);
                if (existing == requested)
                {
                    setParameterActualValue(enabled[static_cast<size_t>(slot)], 1.0f);
                    repaint();
                    return;
                }
                if (existing == 0 && freeSlot < 0)
                    freeSlot = slot;
            }
            if (freeSlot >= 0)
            {
                setParameterActualValue(choices[static_cast<size_t>(freeSlot)],
                                        static_cast<float>(requested));
                setParameterActualValue(enabled[static_cast<size_t>(freeSlot)],
                                        1.0f);
            }
            repaint();
        }

        void paint(juce::Graphics& graphics) override
        {
            graphics.setColour(juce::Colour(0xff10151a));
            graphics.fillRoundedRectangle(getLocalBounds().toFloat(), 5.0f);
            graphics.setColour(dragOver
                ? controlAccentColour.brighter(0.25f)
                : controlAccentColour.withAlpha(0.65f));
            graphics.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f),
                                           5.0f, dragOver ? 2.0f : 1.0f);
            graphics.setColour(knobLabelColour);
            graphics.setFont(juce::FontOptions(10.5f, juce::Font::bold));
            graphics.drawText("TARGETS", 5, 3, getWidth() - 10, 14,
                              juce::Justification::centred);
            graphics.setColour(knobIndicatorColour);
            graphics.setFont(juce::FontOptions(12.5f, juce::Font::bold));
            graphics.drawText("!!!", randomButtonBounds(),
                              juce::Justification::centred, false);
            const auto slots = assignedSlots();
            const auto depthColour = modeColourForId(
                juce::jlimit(1, delayModeCount,
                    juce::roundToInt(mode != nullptr ? mode->load() : 0.0f)
                        + 1));
            if (slots.empty())
            {
                graphics.setColour(knobLabelColour.darker(0.25f));
                graphics.setFont(juce::FontOptions(10.5f));
                graphics.drawFittedText("DROP PARAMETER LABELS HERE",
                    8, 28, getWidth() - 16, getHeight() - 36,
                    juce::Justification::centred, 2);
                return;
            }
            const int visibleRows = juce::jmax(1, (getHeight() - 20) / rowHeight);
            const int maximumFirst = juce::jmax(
                0, static_cast<int>(slots.size()) - visibleRows);
            firstVisibleRow = juce::jlimit(0, maximumFirst, firstVisibleRow);
            for (int visibleRow = 0;
                 visibleRow < visibleRows
                    && firstVisibleRow + visibleRow
                        < static_cast<int>(slots.size());
                 ++visibleRow)
            {
                const int slot = slots[static_cast<size_t>(
                    firstVisibleRow + visibleRow)];
                const int y = 20 + visibleRow * rowHeight;
                const bool isEnabled = enabledForSlot(slot);
                graphics.setColour(isEnabled
                                             ? controlAccentColour.withAlpha(0.18f)
                                             : juce::Colour(0xff171c21));
                graphics.fillRoundedRectangle(4.0f, static_cast<float>(y),
                    static_cast<float>(getWidth() - 8),
                    static_cast<float>(rowHeight - 2), 2.5f);
                graphics.setColour(isEnabled ? controlAccentColour
                                             : knobTrackColour);
                graphics.fillEllipse(8.0f, static_cast<float>(y + 6), 6.0f, 6.0f);

                const auto knob = depthBoundsForVisibleRow(visibleRow).toFloat();
                const float rowAlpha = isEnabled ? 1.0f : 0.42f;
                graphics.setColour(juce::Colour(0xff090c0f).withMultipliedAlpha(
                    rowAlpha));
                graphics.fillEllipse(knob);
                graphics.setColour(depthColour.withAlpha(
                    0.42f * rowAlpha));
                graphics.drawEllipse(knob.reduced(0.5f), 1.0f);
                const float depth = depthForSlot(slot);
                if (depth > 0.0001f)
                {
                    constexpr float startAngle =
                        juce::MathConstants<float>::pi * 1.20f;
                    constexpr float arcLength =
                        juce::MathConstants<float>::pi * 1.60f;
                    juce::Path arc;
                    const auto arcBounds = knob.reduced(2.5f);
                    arc.addCentredArc(
                        arcBounds.getCentreX(), arcBounds.getCentreY(),
                        arcBounds.getWidth() * 0.5f,
                        arcBounds.getHeight() * 0.5f, 0.0f,
                        startAngle, startAngle + arcLength * depth, true);
                    graphics.setColour(depthColour.withAlpha(
                        0.90f * rowAlpha));
                    graphics.strokePath(
                        arc, juce::PathStrokeType(
                            1.7f, juce::PathStrokeType::curved,
                            juce::PathStrokeType::rounded));
                }

                graphics.setColour(isEnabled ? knobValueColour
                                             : knobLabelColour.darker(0.35f));
                graphics.setFont(juce::FontOptions(10.5f,
                    isEnabled ? juce::Font::bold : juce::Font::plain));
                graphics.drawFittedText(afterglow::sequencer::targetName(
                        choiceForSlot(slot)), 38, y + 1, getWidth() - 61,
                    rowHeight - 4, juce::Justification::centredLeft, 1);
                graphics.setColour(knobLabelColour);
                const float closeX = static_cast<float>(getWidth() - 12);
                const float closeY = static_cast<float>(y)
                                   + 0.5f * static_cast<float>(rowHeight - 2);
                graphics.drawLine(closeX - 3.0f, closeY - 3.0f,
                                  closeX + 3.0f, closeY + 3.0f, 1.25f);
                graphics.drawLine(closeX + 3.0f, closeY - 3.0f,
                                  closeX - 3.0f, closeY + 3.0f, 1.25f);
            }
            if (canScrollUp())
            {
                graphics.setColour(controlAccentColour);
                const auto bounds = scrollUpBounds().toFloat();
                const auto centre = bounds.getCentre();
                graphics.drawLine(centre.x - 4.0f, centre.y + 2.0f,
                                  centre.x, centre.y - 2.0f, 1.6f);
                graphics.drawLine(centre.x, centre.y - 2.0f,
                                  centre.x + 4.0f, centre.y + 2.0f, 1.6f);
            }
            if (canScrollDown())
            {
                graphics.setColour(controlAccentColour);
                const auto bounds = scrollDownBounds().toFloat();
                const auto centre = bounds.getCentre();
                graphics.drawLine(centre.x - 4.0f, centre.y - 2.0f,
                                  centre.x, centre.y + 2.0f, 1.6f);
                graphics.drawLine(centre.x, centre.y + 2.0f,
                                  centre.x + 4.0f, centre.y - 2.0f, 1.6f);
            }
        }

        void mouseDown(const juce::MouseEvent& event) override
        {
            if (randomButtonBounds().contains(event.getPosition()))
            {
                randomiseTargets();
                return;
            }
            if (canScrollUp() && scrollUpBounds().contains(event.getPosition()))
            {
                --firstVisibleRow;
                repaint();
                return;
            }
            if (canScrollDown()
                && scrollDownBounds().contains(event.getPosition()))
            {
                ++firstVisibleRow;
                repaint();
                return;
            }
            if (event.y < 20)
                return;
            const int row = (event.y - 20) / rowHeight + firstVisibleRow;
            const auto slots = assignedSlots();
            if (! juce::isPositiveAndBelow(row,
                                           static_cast<int>(slots.size())))
                return;
            const int slot = slots[static_cast<size_t>(row)];
            const int visibleRow = row - firstVisibleRow;
            if (depthBoundsForVisibleRow(visibleRow).contains(event.getPosition()))
            {
                draggedDepthParameter = depths[static_cast<size_t>(slot)];
                if (draggedDepthParameter != nullptr)
                {
                    dragStartDepth = depthForSlot(slot);
                    dragStartY = event.y;
                    draggedDepthParameter->beginChangeGesture();
                }
            }
            else if (event.x >= getWidth() - 24)
                setParameterActualValue(choices[static_cast<size_t>(slot)], 0.0f);
            else
                setParameterActualValue(enabled[static_cast<size_t>(slot)],
                                        enabledForSlot(slot) ? 0.0f : 1.0f);
            repaint();
        }

        void mouseDrag(const juce::MouseEvent& event) override
        {
            if (draggedDepthParameter == nullptr)
                return;

            const float sensitivity = event.mods.isShiftDown() ? 360.0f
                                                                : 120.0f;
            const float depth = juce::jlimit(
                0.0f, 1.0f,
                dragStartDepth
                    + static_cast<float>(dragStartY - event.y) / sensitivity);
            draggedDepthParameter->setValueNotifyingHost(
                draggedDepthParameter->convertTo0to1(depth));
            setDepthTooltip(depth);
            repaint();
        }

        void mouseUp(const juce::MouseEvent&) override
        {
            if (draggedDepthParameter != nullptr)
            {
                draggedDepthParameter->endChangeGesture();
                draggedDepthParameter = nullptr;
            }
        }

        void mouseDoubleClick(const juce::MouseEvent& event) override
        {
            const int row = (event.y - 20) / rowHeight + firstVisibleRow;
            const auto slots = assignedSlots();
            if (event.y < 20
                || ! juce::isPositiveAndBelow(
                    row, static_cast<int>(slots.size())))
                return;

            const int visibleRow = row - firstVisibleRow;
            if (! depthBoundsForVisibleRow(visibleRow).contains(
                    event.getPosition()))
                return;

            auto* parameter = depths[static_cast<size_t>(
                slots[static_cast<size_t>(row)])];
            if (parameter == nullptr)
                return;
            if (draggedDepthParameter != nullptr)
            {
                draggedDepthParameter->endChangeGesture();
                draggedDepthParameter = nullptr;
            }
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost(parameter->getDefaultValue());
            parameter->endChangeGesture();
            setDepthTooltip(parameter->convertFrom0to1(
                parameter->getValue()));
            repaint();
        }

        void mouseMove(const juce::MouseEvent& event) override
        {
            if (randomButtonBounds().contains(event.getPosition()))
            {
                setMouseCursor(juce::MouseCursor::PointingHandCursor);
                const auto tooltip =
                    "Randomise target assignments, enabled states and individual Depths";
                setTooltip(tooltip);
                if (showHelp != nullptr)
                    showHelp("RANDOMISE TARGETS\n" + juce::String(tooltip));
                return;
            }
            if ((canScrollUp()
                    && scrollUpBounds().contains(event.getPosition()))
                || (canScrollDown()
                    && scrollDownBounds().contains(event.getPosition())))
            {
                setMouseCursor(juce::MouseCursor::PointingHandCursor);
                const bool upwards = scrollUpBounds().contains(
                    event.getPosition());
                const auto tooltip = upwards ? "Show earlier targets"
                                             : "Show later targets";
                setTooltip(tooltip);
                if (showHelp != nullptr)
                    showHelp("TARGETS\n" + juce::String(tooltip));
                return;
            }

            const int row = (event.y - 20) / rowHeight + firstVisibleRow;
            const auto slots = assignedSlots();
            const int visibleRow = row - firstVisibleRow;
            if (event.y >= 20
                && juce::isPositiveAndBelow(
                    row, static_cast<int>(slots.size()))
                && depthBoundsForVisibleRow(visibleRow).contains(
                    event.getPosition()))
            {
                setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
                setDepthTooltip(depthForSlot(
                    slots[static_cast<size_t>(row)]));
            }
            else
            {
                setMouseCursor(juce::MouseCursor::NormalCursor);
                setTooltip("Drop an Afterglow knob label here. The small knob sets that target's individual modulation depth.");
            }
        }

        void mouseWheelMove(const juce::MouseEvent& event,
                            const juce::MouseWheelDetails& wheel) override
        {
            const auto slots = assignedSlots();
            const int row = (event.y - 20) / rowHeight + firstVisibleRow;
            const int visibleRow = row - firstVisibleRow;
            if (event.y >= 20
                && juce::isPositiveAndBelow(
                    row, static_cast<int>(slots.size()))
                && depthBoundsForVisibleRow(visibleRow).contains(
                    event.getPosition()))
            {
                auto* parameter = depths[static_cast<size_t>(
                    slots[static_cast<size_t>(row)])];
                if (parameter != nullptr)
                {
                    const float depth = juce::jlimit(
                        0.0f, 1.0f,
                        parameter->convertFrom0to1(parameter->getValue())
                            + wheel.deltaY * 0.12f);
                    parameter->beginChangeGesture();
                    parameter->setValueNotifyingHost(
                        parameter->convertTo0to1(depth));
                    parameter->endChangeGesture();
                    setDepthTooltip(depth);
                    repaint();
                }
                return;
            }

            const int visibleRows = juce::jmax(1, (getHeight() - 20) / rowHeight);
            const int maximumFirst = juce::jmax(
                0, static_cast<int>(slots.size()) - visibleRows);
            firstVisibleRow = juce::jlimit(
                0, maximumFirst,
                firstVisibleRow + (wheel.deltaY > 0.0f ? -1 : 1));
            repaint();
        }

    private:
        void randomiseTargets()
        {
            juce::Random random;
            const auto randomise = [&random](juce::RangedAudioParameter* parameter)
            {
                if (parameter == nullptr)
                    return;
                parameter->beginChangeGesture();
                parameter->setValueNotifyingHost(random.nextFloat());
                parameter->endChangeGesture();
            };

            for (int slot = 0; slot < afterglow::sequencer::targetSlotCount;
                 ++slot)
            {
                const auto index = static_cast<size_t>(slot);
                randomise(choices[index]);
                randomise(enabled[index]);
                randomise(depths[index]);
            }

            firstVisibleRow = 0;
            repaint();
            if (showHelp != nullptr)
                showHelp(
                    "RANDOMISE TARGETS\nTarget assignments, enabled states and individual Depths were randomised.");
        }

        juce::Rectangle<int> randomButtonBounds() const
        {
            return { 0, 2, 24, 16 };
        }

        int choiceForSlot(int slot) const
        {
            auto* parameter = choices[static_cast<size_t>(slot)];
            return parameter != nullptr ? juce::roundToInt(
                parameter->convertFrom0to1(parameter->getValue())) : 0;
        }

        bool enabledForSlot(int slot) const
        {
            auto* parameter = enabled[static_cast<size_t>(slot)];
            return parameter != nullptr
                && parameter->convertFrom0to1(parameter->getValue()) >= 0.5f;
        }

        float depthForSlot(int slot) const
        {
            auto* parameter = depths[static_cast<size_t>(slot)];
            return parameter != nullptr
                ? juce::jlimit(0.0f, 1.0f,
                    parameter->convertFrom0to1(parameter->getValue()))
                : 1.0f;
        }

        juce::Rectangle<int> depthBoundsForVisibleRow(int visibleRow) const
        {
            return { 18, 21 + visibleRow * rowHeight, 16, 16 };
        }

        void setDepthTooltip(float depth)
        {
            const auto tooltip = "Target Depth: modulation amount ("
                + juce::String(juce::roundToInt(depth * 100.0f)) + "%)";
            setTooltip(tooltip);
            if (showHelp != nullptr)
                showHelp("TARGET DEPTH\n" + tooltip);
        }

        std::vector<int> assignedSlots() const
        {
            std::vector<int> result;
            for (int slot = 0; slot < afterglow::sequencer::targetSlotCount;
                 ++slot)
                if (choiceForSlot(slot) > 0)
                    result.push_back(slot);
            return result;
        }

        int visibleRowCount() const
        {
            return juce::jmax(1, (getHeight() - 20) / rowHeight);
        }

        bool canScrollUp() const
        {
            return firstVisibleRow > 0;
        }

        bool canScrollDown() const
        {
            return firstVisibleRow + visibleRowCount()
                < static_cast<int>(assignedSlots().size());
        }

        juce::Rectangle<int> scrollUpBounds() const
        {
            return { getWidth() - 22, 1, 20, 18 };
        }

        juce::Rectangle<int> scrollDownBounds() const
        {
            return { getWidth() - 22, getHeight() - 14, 20, 13 };
        }

        AfterglowAudioProcessor& processor;
        std::function<void(const juce::String&)> showHelp;
        std::atomic<float>* mode = nullptr;
        std::array<juce::RangedAudioParameter*,
                   afterglow::sequencer::targetSlotCount> choices {};
        std::array<juce::RangedAudioParameter*,
                   afterglow::sequencer::targetSlotCount> enabled {};
        std::array<juce::RangedAudioParameter*,
                   afterglow::sequencer::targetSlotCount> depths {};
        juce::RangedAudioParameter* draggedDepthParameter = nullptr;
        float dragStartDepth = 1.0f;
        int dragStartY = 0;
        bool dragOver = false;
        int firstVisibleRow = 0;
        static constexpr int rowHeight = 20;
    };

    SequencerPanel(AfterglowAudioProcessor& audioProcessor,
                   LookAndFeel& sharedLookAndFeel,
                   std::function<juce::Colour()> surroundColourProvider,
                   std::function<void(const juce::String&)> helpCallback)
        : processor(audioProcessor),
          surroundColour(std::move(surroundColourProvider)),
          showHelp(std::move(helpCallback)),
          grid(audioProcessor, sharedLookAndFeel, patternMenuLookAndFeel),
          targets(audioProcessor, showHelp)
    {
        setLookAndFeel(&sharedLookAndFeel);
        addMouseListener(this, true);
        addAndMakeVisible(grid);
        addAndMakeVisible(targets);
        const std::array<juce::Component*, 33> components
        {{ &hostSync, &enabledButton, &bipolarButton,
           &animateButton, &gateModeButton,
           &waveformBox, &userSequenceBox, &directionBox,
           &rateSlider, &startSlider, &endSlider, &attackSlider,
           &releaseSlider, &peakSlider, &moveSlider, &colourSlider,
           &saturationSlider, &luminanceSlider,
           &startLabel, &endLabel, &attackLabel, &releaseLabel,
           &peakLabel, &moveLabel, &shortGateLabel, &longGateLabel,
           &nudgeLeft, &nudgeRight, &gateNudgeLeft, &gateNudgeRight,
           &rangeLinkButton,
           &shortGateSlider, &longGateSlider }};
        for (auto* component : components)
            addAndMakeVisible(component);

        hostSync.setButtonText("HOST SYNC");
        enabledButton.setButtonText("ON");
        bipolarButton.setButtonText("UNIPOLAR +");
        animateButton.setButtonText("ANIMATE");
        hostSync.setName("Host Sync");
        enabledButton.setName("Sequencer On");
        bipolarButton.setName("Sequencer Polarity");
        animateButton.setName("Animate");
        gateModeButton.setName("Gate Mode");
        waveformBox.setName("Waveform");
        userSequenceBox.setName("User Sequence");
        directionBox.setName("Direction");
        rateSlider.setName("Rate");
        startSlider.setName("Start");
        endSlider.setName("End");
        rangeLinkButton.setName("Start / End Link");
        colourSlider.setName("Hue");
        saturationSlider.setName("Saturation");
        luminanceSlider.setName("Luminance");
        attackSlider.setName("Attack");
        releaseSlider.setName("Release");
        peakSlider.setName("Depth");
        moveSlider.setName("Offset");
        shortGateSlider.setName("Short");
        longGateSlider.setName("Long");
        grid.setName("Steps");
        targets.setName("Targets");
        for (auto* button : { &hostSync, &enabledButton, &bipolarButton,
                              &animateButton })
        {
            button->setClickingTogglesState(true);
            button->setColour(juce::ToggleButton::textColourId,
                              knobLabelColour);
            button->getProperties().set("activeColour",
                                        controlAccentColour.toString());
            button->getProperties().set("sequencerLabelStyle", true);
        }
        hostSync.getProperties().set("activeColour",
                                     tempoSyncLedColour.toString());
        enabledButton.getProperties().set("activeColour",
                                          linkLedColour.toString());
        enabledButton.getProperties().set("fixedLedCentreX", 10.0f);
        bipolarButton.setClickingTogglesState(false);
        bipolarButton.setToggleState(true, juce::dontSendNotification);
        bipolarButton.getProperties().set("fixedLedCentreX", 10.0f);
        gateModeButton.setClickingTogglesState(false);
        gateModeButton.setToggleState(true, juce::dontSendNotification);
        gateModeButton.getProperties().set("fixedLedCentreX", 10.0f);
        gateModeButton.setColour(juce::ToggleButton::textColourId,
                                 knobLabelColour);
        gateModeButton.getProperties().set("sequencerLabelStyle", true);
        nudgeLeft.setName("Nudge pattern left");
        nudgeRight.setName("Nudge pattern right");
        gateNudgeLeft.setName("Nudge Gates left");
        gateNudgeRight.setName("Nudge Gates right");
        for (auto* button : { &nudgeLeft, &nudgeRight,
                              &gateNudgeLeft, &gateNudgeRight })
            button->setClickingTogglesState(false);
        nudgeLeft.getProperties().set("presetArrowDirection", -1);
        nudgeRight.getProperties().set("presetArrowDirection", 1);
        gateNudgeLeft.getProperties().set("presetArrowDirection", -1);
        gateNudgeRight.getProperties().set("presetArrowDirection", 1);
        for (auto* button : { &nudgeLeft, &nudgeRight,
                              &gateNudgeLeft, &gateNudgeRight })
            button->getProperties().set("compactPresetArrow", true);
        rangeLinkButton.setButtonText({});
        rangeLinkButton.setClickingTogglesState(true);
        rangeLinkButton.getProperties().set("rangeLinkIcon", true);
        rangeLinkButton.getProperties().set("compactRangeLink", true);
        rangeLinkButton.getProperties().set("activeColour",
                                             controlAccentColour.toString());
        nudgeLeft.onClick = [this] { grid.nudge(-1); };
        nudgeRight.onClick = [this] { grid.nudge(1); };
        gateNudgeLeft.onClick = [this] { grid.nudgeGates(-1); };
        gateNudgeRight.onClick = [this] { grid.nudgeGates(1); };

        hostSync.setTooltip("Synchronise the sequencer clock to the host tempo");
        enabledButton.setTooltip("Enable or disable sequencer modulation");
        bipolarButton.setTooltip(
            "Cycle target movement between Unipolar +, Bipolar and Unipolar -");
        animateButton.setTooltip(
            "Animate assigned controls without changing their stored values");
        animateButton.onClick = [this]
        {
            updateAnimatedControls();
            if (onAnimateChanged != nullptr)
                onAnimateChanged();
        };
        waveformBox.setTooltip("Replace the steps with a generated waveform");
        userSequenceBox.setTooltip("Load a saved user sequence");
        rateSlider.setTooltip("Set the sequencer rate");
        startSlider.setTooltip("Set the first step in the playback range");
        rangeLinkButton.setTooltip("Link Start and End so the range moves without changing length");
        endSlider.setTooltip("Set the last step in the playback range");
        directionBox.setTooltip("Choose the sequencer playback direction");
        colourSlider.setTooltip("Choose the hue used by the steps and assigned target labels");
        saturationSlider.setTooltip("Adjust the saturation of the sequencer colour");
        luminanceSlider.setTooltip("Adjust only the luminance of the sequencer steps");
        attackSlider.setTooltip("Smooth the rising edge between sequence steps");
        releaseSlider.setTooltip("Smooth the falling edge between sequence steps");
        peakSlider.setTooltip("Shrink or expand the sequence shape; 0% leaves it unchanged");
        moveSlider.setTooltip("Offset the complete sequence up or down");
        nudgeLeft.setTooltip("Move the step values one position left");
        nudgeRight.setTooltip("Move the step values one position right");
        gateNudgeLeft.setTooltip("Move the Gate states one position left");
        gateNudgeRight.setTooltip("Move the Gate states one position right");
        shortGateSlider.setTooltip(
            "Set how much of a step remains open in Short Gate mode");
        longGateSlider.setTooltip(
            "Set how much of a step remains open in Long Gate mode");
        gateModeButton.setTooltip(
            "Choose whether Gate interrupts delay input, wet output or feedback regeneration, or bypass Gate processing");

        const auto configureTargetLabel = [](ParameterDragLabel& label,
                                             const juce::String& text,
                                             const juce::String& parameterId)
        {
            label.setText(text, juce::dontSendNotification);
            label.setJustificationType(juce::Justification::centred);
            label.setFont(juce::Font(juce::FontOptions(
                10.5f, juce::Font::bold)));
            label.setColour(juce::Label::textColourId, knobLabelColour);
            label.setInterceptsMouseClicks(true, false);
            label.parameterIdProvider = [parameterId] { return parameterId; };
            label.setTooltip("Drag " + text
                             + " onto a sequencer target slot");
        };
        configureTargetLabel(startLabel, "START", "sequencerStart");
        configureTargetLabel(endLabel, "END", "sequencerEnd");
        configureTargetLabel(attackLabel, "ATTACK", "sequencerAttack");
        configureTargetLabel(releaseLabel, "RELEASE", "sequencerRelease");
        configureTargetLabel(peakLabel, "DEPTH", "sequencerPeak");
        configureTargetLabel(moveLabel, "OFFSET", "sequencerMove");
        configureTargetLabel(shortGateLabel, "SHORT",
                             "sequencerGateShortLength");
        configureTargetLabel(longGateLabel, "LONG",
                             "sequencerGateLongLength");
        startLabel.setTooltip(startSlider.getTooltip());
        endLabel.setTooltip(endSlider.getTooltip());
        attackLabel.setTooltip(attackSlider.getTooltip());
        releaseLabel.setTooltip(releaseSlider.getTooltip());
        peakLabel.setTooltip(peakSlider.getTooltip());
        moveLabel.setTooltip(moveSlider.getTooltip());
        shortGateLabel.setTooltip(shortGateSlider.getTooltip());
        longGateLabel.setTooltip(longGateSlider.getTooltip());

        waveformBox.addItem("Saw", 1);
        waveformBox.addItem("Saw Double", 2);
        waveformBox.addItem("Saw Down", 3);
        waveformBox.addItem("Saw Down Double", 4);
        waveformBox.addItem("Sine", 5);
        waveformBox.addItem("Sine Double", 6);
        waveformBox.addItem("Triangle", 7);
        waveformBox.addItem("Triangle Double", 8);
        waveformBox.addItem("Pulse 25", 9);
        waveformBox.addItem("Pulse 25 Double", 10);
        waveformBox.addItem("Square", 11);
        waveformBox.addItem("Square Double", 12);
        waveformBox.addItem("Pulse 75", 13);
        waveformBox.addItem("Pulse 75 Double", 14);
        waveformBox.setTextWhenNothingSelected("WAVEFORM");
        waveformBox.onChange = [this]
        {
            const int selected = waveformBox.getSelectedItemIndex();
            if (selected >= 0)
                grid.applyWaveform(
                    static_cast<afterglow::sequencer::Waveform>(selected));
            waveformBox.setSelectedId(0, juce::dontSendNotification);
        };

        directionBox.addItemList(
            { "Loop", "Bounce", "Reverse", "Played", "Random" }, 1);
        for (auto* box : { &waveformBox, &userSequenceBox, &directionBox })
        {
            box->setJustificationType(juce::Justification::centred);
            box->setColour(juce::ComboBox::backgroundColourId,
                           juce::Colour(0xff19151f));
            box->setColour(juce::ComboBox::textColourId, knobValueColour);
            box->setColour(juce::ComboBox::outlineColourId, knobTrackColour);
            box->setColour(juce::ComboBox::arrowColourId,
                           knobIndicatorColour);
            box->setScrollWheelEnabled(true);
            box->getProperties().set("compactClosedCombo", true);
        }
        directionAttachment = std::make_unique<
            juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                processor.parameters, "sequencerDirection", directionBox);
        hostSyncAttachment = std::make_unique<
            juce::AudioProcessorValueTreeState::ButtonAttachment>(
                processor.parameters, "sequencerHostSync", hostSync);
        enabledAttachment = std::make_unique<
            juce::AudioProcessorValueTreeState::ButtonAttachment>(
                processor.parameters, "sequencerEnabled", enabledButton);
        enabledButton.onStateChange = [this]
        {
            updateSequencerControlEnablement();
        };
        updateSequencerControlEnablement();
        auto* bipolarParameter = processor.parameters.getParameter(
            "sequencerBipolar");
        auto* negativeParameter = processor.parameters.getParameter(
            "sequencerUnipolarNegative");
        jassert(bipolarParameter != nullptr && negativeParameter != nullptr);
        if (bipolarParameter != nullptr && negativeParameter != nullptr)
        {
            bipolarModeAttachment = std::make_unique<juce::ParameterAttachment>(
                *bipolarParameter,
                [this](float)
                {
                    if (! updatingPolarityParameters)
                        syncPolarityFromParameters();
                }, nullptr);
            negativeModeAttachment = std::make_unique<juce::ParameterAttachment>(
                *negativeParameter,
                [this](float)
                {
                    if (! updatingPolarityParameters)
                        syncPolarityFromParameters();
                }, nullptr);
            bipolarButton.onClick = [this] { cyclePolarityMode(); };
            bipolarModeAttachment->sendInitialUpdate();
            negativeModeAttachment->sendInitialUpdate();
            syncPolarityFromParameters();
        }
        rangeLinkAttachment = std::make_unique<
            juce::AudioProcessorValueTreeState::ButtonAttachment>(
                processor.parameters, "sequencerRangeLink", rangeLinkButton);
        if (auto* gateModeParameter = processor.parameters.getParameter(
                "sequencerGateMode"))
        {
            gateModeAttachment = std::make_unique<juce::ParameterAttachment>(
                *gateModeParameter,
                [this](float)
                {
                    if (! updatingGateModeParameter)
                        syncGateModeFromParameter();
                }, nullptr);
            gateModeButton.onClick = [this] { cycleGateMode(); };
            gateModeAttachment->sendInitialUpdate();
            syncGateModeFromParameter();
        }

        configureKnob(rateSlider, 0.0, 13.0, 1.0);
        configureKnob(startSlider, 1.0, 31.0, 1.0);
        configureKnob(endSlider, 2.0, 32.0, 1.0);
        configureKnob(attackSlider, 0.0, 1.0, 0.001);
        configureKnob(releaseSlider, 0.0, 1.0, 0.001);
        attackSlider.textFromValueFunction
            = releaseSlider.textFromValueFunction
            = [](double value)
            {
                return juce::String(juce::roundToInt(value * 100.0)) + "%";
            };
        attackSlider.valueFromTextFunction
            = releaseSlider.valueFromTextFunction
            = [](const juce::String& text)
            {
                const double entered = text.getDoubleValue();
                return juce::jlimit(0.0, 1.0,
                                    entered > 1.0 || text.containsChar('%')
                                        ? entered * 0.01 : entered);
            };
        attackSlider.setTextBoxStyle(
            juce::Slider::TextBoxBelow, false, 52, 14);
        releaseSlider.setTextBoxStyle(
            juce::Slider::TextBoxBelow, false, 52, 14);
        configureKnob(shortGateSlider, 0.10, 0.60, 0.01);
        configureKnob(longGateSlider, 0.65, 0.95, 0.01);
        shortGateSlider.setDoubleClickReturnValue(true, 0.50);
        longGateSlider.setDoubleClickReturnValue(true, 0.90);
        configureVerticalSlider(peakSlider, -1.0, 1.0, 0.001);
        configureVerticalSlider(moveSlider, -1.0, 1.0, 0.001);
        peakSlider.setScrollWheelEnabled(true);
        moveSlider.setScrollWheelEnabled(true);
        peakSlider.setCentreDetent(0.0, 0.03);
        peakSlider.setDoubleClickReturnValue(true, 0.0);
        moveSlider.setCentreDetent(0.0, 0.03);
        colourSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        colourSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        colourSlider.setRange(0.0, 1.0, 0.001);
        colourSlider.setValue(0.5, juce::dontSendNotification);
        colourSlider.onValueChange = [this] { updateAccent(); };
        colourSlider.setDoubleClickReturnValue(true, 0.5);
        saturationSlider.setSliderStyle(
            juce::Slider::RotaryHorizontalVerticalDrag);
        saturationSlider.setTextBoxStyle(
            juce::Slider::NoTextBox, false, 0, 0);
        saturationSlider.setRange(0.0, 1.0, 0.001);
        saturationSlider.setValue(0.5, juce::dontSendNotification);
        saturationSlider.onValueChange = [this] { updateAccent(); };
        saturationSlider.setDoubleClickReturnValue(true, 0.5);
        luminanceSlider.setSliderStyle(
            juce::Slider::RotaryHorizontalVerticalDrag);
        luminanceSlider.setTextBoxStyle(
            juce::Slider::NoTextBox, false, 0, 0);
        luminanceSlider.setRange(0.2, 1.0, 0.001);
        luminanceSlider.setValue(0.5, juce::dontSendNotification);
        luminanceSlider.onValueChange = [this] { updateAccent(); };
        luminanceSlider.setDoubleClickReturnValue(true, 0.5);

        colourAttachment = makeSliderAttachment(
            "sequencerHue", colourSlider);
        saturationAttachment = makeSliderAttachment(
            "sequencerSaturation", saturationSlider);
        luminanceAttachment = makeSliderAttachment(
            "sequencerLuminance", luminanceSlider);

        rateAttachment = makeSliderAttachment("sequencerRate", rateSlider);
        startAttachment = makeSliderAttachment("sequencerStart", startSlider);
        endAttachment = makeSliderAttachment("sequencerEnd", endSlider);
        attackAttachment = makeSliderAttachment("sequencerAttack", attackSlider);
        releaseAttachment = makeSliderAttachment("sequencerRelease", releaseSlider);
        peakAttachment = makeSliderAttachment("sequencerPeak", peakSlider);
        moveAttachment = makeSliderAttachment("sequencerMove", moveSlider);
        shortGateAttachment = makeSliderAttachment(
            "sequencerGateShortLength", shortGateSlider);
        longGateAttachment = makeSliderAttachment(
            "sequencerGateLongLength", longGateSlider);
        configureSavedReset(rateSlider, "sequencerRate");
        configureSavedReset(startSlider, "sequencerStart");
        configureSavedReset(endSlider, "sequencerEnd");
        configureSavedReset(attackSlider, "sequencerAttack");
        configureSavedReset(releaseSlider, "sequencerRelease");
        configureSavedReset(peakSlider, "sequencerPeak");
        configureSavedReset(moveSlider, "sequencerMove");
        configureSavedReset(shortGateSlider, "sequencerGateShortLength");
        configureSavedReset(longGateSlider, "sequencerGateLongLength");
        linkedRangeLength = juce::jmax(
            1, juce::roundToInt(endSlider.getValue()
                               - startSlider.getValue()));
        rangeLinkButton.onClick = [this]
        {
            if (rangeLinkButton.getToggleState())
                linkedRangeLength = juce::jmax(
                    1, juce::roundToInt(endSlider.getValue()
                                       - startSlider.getValue()));
        };
        startSlider.onValueChange = [this]
        {
            if (! rangeLinkButton.getToggleState() || updatingLinkedRange)
                return;
            const juce::ScopedValueSetter<bool> guard(updatingLinkedRange, true);
            const int startValue = juce::roundToInt(startSlider.getValue());
            const int requestedEnd = startValue + linkedRangeLength;
            if (requestedEnd <= afterglow::sequencer::stepCount)
                endSlider.setValue(requestedEnd, juce::sendNotificationSync);
            else
                startSlider.setValue(afterglow::sequencer::stepCount
                                         - linkedRangeLength,
                                     juce::sendNotificationSync);
        };
        endSlider.onValueChange = [this]
        {
            if (! rangeLinkButton.getToggleState() || updatingLinkedRange)
                return;
            const juce::ScopedValueSetter<bool> guard(updatingLinkedRange, true);
            const int endValue = juce::roundToInt(endSlider.getValue());
            const int requestedStart = endValue - linkedRangeLength;
            if (requestedStart >= 1)
                startSlider.setValue(requestedStart, juce::sendNotificationSync);
            else
                endSlider.setValue(1 + linkedRangeLength,
                                   juce::sendNotificationSync);
        };
        rateSlider.textFromValueFunction = [](double value)
        {
            static const std::array<const char*, 14> names
            { "1/128", "1/64T", "1/64", "1/32T", "1/32", "1/16T",
              "1/16", "1/8T", "1/8", "1/4T", "1/4", "1/2T", "1/2", "1/1" };
            return juce::String(names[static_cast<size_t>(juce::jlimit(
                0, 13, juce::roundToInt(value)))]);
        };
        peakSlider.textFromValueFunction = moveSlider.textFromValueFunction
            = [](double value) { return juce::String(value * 100.0, 0) + "%"; };
        shortGateSlider.textFromValueFunction
            = longGateSlider.textFromValueFunction
            = [](double value) { return juce::String(value * 100.0, 0) + "%"; };
        attackSlider.updateText();
        releaseSlider.updateText();

        grid.onSaveRequested = [this] { saveUserSequence(); };
        userSequenceBox.setTextWhenNothingSelected("USER");
        userSequenceBox.onChange = [this] { loadSelectedUserSequence(); };
        refreshUserSequences();
        updateAccent();
    }

    ~SequencerPanel() override
    {
        removeMouseListener(this);
        setLookAndFeel(nullptr);
    }

    double getHue() const noexcept { return colourSlider.getValue(); }
    juce::Colour getAccent() const noexcept { return accent; }
    bool isTargetAssigned(const juce::String& parameterId) const
    {
        const int requested = afterglow::sequencer::targetChoiceForParameter(
            parameterId);
        if (requested <= 0)
            return false;

        for (int slot = 0; slot < afterglow::sequencer::targetSlotCount;
             ++slot)
        {
            auto* parameter = processor.parameters.getParameter(
                afterglow::sequencer::targetParameterId(slot));
            if (parameter != nullptr
                && juce::roundToInt(parameter->convertFrom0to1(
                       parameter->getValue())) == requested)
                return true;
        }
        return false;
    }
    double getSaturation() const noexcept
    {
        return saturationSlider.getValue();
    }
    double getLuminance() const noexcept
    {
        return luminanceSlider.getValue();
    }
    bool getAnimate() const noexcept
    {
        return animateButton.getToggleState();
    }
    void setAnimate(bool shouldAnimate)
    {
        animateButton.setToggleState(shouldAnimate,
                                     juce::dontSendNotification);
        updateAnimatedControls();
    }
    std::function<void()> onAnimateChanged;
    void setHue(double hue)
    {
        colourSlider.setValue(juce::jlimit(0.0, 1.0, hue),
                              juce::dontSendNotification);
        updateAccent();
    }
    void setSaturation(double saturation)
    {
        saturationSlider.setValue(juce::jlimit(0.0, 1.0, saturation),
                                  juce::dontSendNotification);
        updateAccent();
    }
    void setLuminance(double luminance)
    {
        luminanceSlider.setValue(juce::jlimit(0.2, 1.0, luminance),
                                 juce::dontSendNotification);
        updateAccent();
    }
    void updateTargetHighlights()
    {
        const auto update = [this](ParameterDragLabel& label)
        {
            const auto parameterId = label.parameterIdProvider != nullptr
                ? label.parameterIdProvider() : juce::String();
            label.setTargetAppearance(isTargetAssigned(parameterId), accent);
        };
        update(startLabel);
        update(endLabel);
        update(attackLabel);
        update(releaseLabel);
        update(peakLabel);
        update(moveLabel);
        update(shortGateLabel);
        update(longGateLabel);
    }
    void timerTick()
    {
        updateSequencerControlEnablement();
        if (! updatingPolarityParameters)
            syncPolarityFromParameters();
        if (! updatingGateModeParameter)
            syncGateModeFromParameter();
        grid.repaint();
        targets.repaint();
        updateAnimatedControls();
    }

    void paint(juce::Graphics& graphics) override
    {
        const auto surround = surroundColour != nullptr
            ? surroundColour() : juce::Colour(0xff24292f);
        graphics.setColour(surround);
        graphics.fillRoundedRectangle(getLocalBounds().toFloat(), 12.0f);
        const auto panelColour = juce::Colour(0xff171b20);
        const auto drawPanel = [&graphics, panelColour](juce::Rectangle<float> area)
        {
            graphics.setColour(panelColour);
            graphics.fillRoundedRectangle(area, 8.0f);
            graphics.setColour(knobTrackColour);
            graphics.drawRoundedRectangle(area, 8.0f, 1.0f);
        };
        drawPanel({ 10.0f, 10.0f, static_cast<float>(getWidth() - 20), 68.0f });
        drawPanel({ 10.0f, 84.0f, static_cast<float>(getWidth() - 20), 152.0f });
        graphics.setColour(sequencerControlsEnabled
            ? knobLabelColour : knobLabelColour.darker(0.48f));
        graphics.setFont(juce::FontOptions(10.5f, juce::Font::bold));
        graphics.drawText("RATE", 243, 11, 60, 13,
                          juce::Justification::centred);
        graphics.drawText("DIRECTION", 443, 11, 106, 13,
                          juce::Justification::centred);
        graphics.drawText("HUE", 773, 11, 46, 13,
                          juce::Justification::centred);
        graphics.drawText("SAT", 815, 11, 38, 13,
                          juce::Justification::centred);
        graphics.drawText("LUM", 853, 11, 38, 13,
                          juce::Justification::centred);
        graphics.drawText("STEPS", 16, 190, 68, 13,
                          juce::Justification::centred);
        graphics.drawText("GATE", 82, 190, 68, 13,
                          juce::Justification::centred);
    }

    void resized() override
    {
        hostSync.setBounds(24, 14, 88, 24);
        animateButton.setBounds(150, 14, 68, 24);
        waveformBox.setBounds(23, 45, 90, 20);
        userSequenceBox.setBounds(139, 45, 90, 20);
        rateSlider.setBounds(251, 26, 44, 48);
        startLabel.setBounds(299, 11, 60, 13);
        startSlider.setBounds(307, 26, 44, 48);
        rangeLinkButton.setBounds(358, 33, 18, 18);
        endLabel.setBounds(379, 11, 60, 13);
        endSlider.setBounds(387, 26, 44, 48);
        directionBox.setBounds(451, 33, 90, 20);
        shortGateLabel.setBounds(553, 11, 60, 13);
        shortGateSlider.setBounds(561, 26, 44, 48);
        longGateLabel.setBounds(611, 11, 60, 13);
        longGateSlider.setBounds(619, 26, 44, 48);
        gateModeButton.setBounds(676, 31, 96, 24);
        colourSlider.setBounds(781, 29, 30, 30);
        saturationSlider.setBounds(819, 29, 30, 30);
        luminanceSlider.setBounds(857, 29, 30, 30);
        enabledButton.setBounds(20, 88, 34, 24);
        bipolarButton.setBounds(56, 88, 84, 24);
        attackLabel.setBounds(16, 118, 68, 13);
        releaseLabel.setBounds(82, 118, 68, 13);
        peakLabel.setBounds(149, 88, 64, 13);
        moveLabel.setBounds(149, 156, 64, 13);
        attackSlider.setBounds(24, 131, 52, 49);
        releaseSlider.setBounds(90, 131, 52, 49);
        peakSlider.setBounds(167, 102, 28, 58);
        moveSlider.setBounds(167, 170, 28, 62);
        nudgeLeft.setBounds(27, 204, 20, 20);
        nudgeRight.setBounds(53, 204, 20, 20);
        gateNudgeLeft.setBounds(93, 204, 20, 20);
        gateNudgeRight.setBounds(119, 204, 20, 20);
        grid.setBounds(210, 92, 554, 136);
        targets.setBounds(774, 92, getWidth() - 792, 136);
    }

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    void setPolarityMode(int polarityMode)
    {
        polarityMode = juce::jlimit(0, 2, polarityMode);
        displayedPolarityMode = polarityMode;
        grid.setPolarityMode(polarityMode);

        const bool isBipolar = polarityMode == 1;
        const bool isNegative = polarityMode == 2;
        bipolarButton.setButtonText(isBipolar ? "BIPOLAR"
            : (isNegative ? "UNIPOLAR -" : "UNIPOLAR +"));
        bipolarButton.getProperties().set(
            "activeColour",
            (isBipolar ? bipolarLedColour
                       : (isNegative ? unipolarNegativeLedColour
                                     : controlAccentColour)).toString());
        bipolarButton.setToggleState(true, juce::dontSendNotification);
        bipolarButton.repaint();
    }

    void syncPolarityFromParameters()
    {
        const auto* bipolar = processor.parameters.getRawParameterValue(
            "sequencerBipolar");
        const auto* negative = processor.parameters.getRawParameterValue(
            "sequencerUnipolarNegative");
        const bool isBipolar = bipolar != nullptr && bipolar->load() >= 0.5f;
        const bool isNegative = ! isBipolar && negative != nullptr
            && negative->load() >= 0.5f;
        const int polarityMode = isBipolar ? 1 : (isNegative ? 2 : 0);
        if (displayedPolarityMode == polarityMode)
            return;
        setPolarityMode(polarityMode);
    }

    void cyclePolarityMode()
    {
        if (bipolarModeAttachment == nullptr
            || negativeModeAttachment == nullptr)
            return;

        const int nextMode = (juce::jlimit(0, 2, displayedPolarityMode) + 1) % 3;

        // Advance the label, LED and grid as one state. The two persisted
        // booleans are then written as a canonical pair without exposing the
        // intermediate pair through either attachment callback.
        setPolarityMode(nextMode);
        const juce::ScopedValueSetter<bool> guard(
            updatingPolarityParameters, true);
        negativeModeAttachment->setValueAsCompleteGesture(
            nextMode == 2 ? 1.0f : 0.0f);
        bipolarModeAttachment->setValueAsCompleteGesture(
            nextMode == 1 ? 1.0f : 0.0f);
    }

    void setGateMode(int gateMode)
    {
        gateMode = juce::jlimit(0, 3, gateMode);
        displayedGateMode = gateMode;
        const auto text = gateMode == 0 ? "INPUT"
                        : gateMode == 1 ? "WET"
                        : gateMode == 2 ? "FEEDBACK"
                                        : "OFF";
        const auto colour = gateMode == 0 ? linkLedColour
                          : gateMode == 1 ? controlAccentColour
                          : gateMode == 2 ? bipolarLedColour
                                          : juce::Colour(0xff596069);
        gateModeButton.setButtonText(text);
        gateModeButton.getProperties().set(
            "activeColour", colour.toString());
        gateModeButton.setToggleState(
            gateMode != 3, juce::dontSendNotification);
        gateModeButton.repaint();
    }

    void syncGateModeFromParameter()
    {
        const auto* gateMode = processor.parameters.getRawParameterValue(
            "sequencerGateMode");
        const int mode = gateMode != nullptr
            ? juce::jlimit(0, 3, juce::roundToInt(gateMode->load())) : 0;
        if (displayedGateMode != mode)
            setGateMode(mode);
    }

    void cycleGateMode()
    {
        if (gateModeAttachment == nullptr)
            return;

        const int nextMode = (juce::jlimit(0, 3, displayedGateMode) + 1) % 4;
        setGateMode(nextMode);
        const juce::ScopedValueSetter<bool> guard(
            updatingGateModeParameter, true);
        gateModeAttachment->setValueAsCompleteGesture(
            static_cast<float>(nextMode));
    }

    void mouseEnter(const juce::MouseEvent& event) override
    {
        if (showHelp == nullptr)
            return;

        const auto isWithin = [](juce::Component* component,
                                 const juce::Component& owner)
        {
            for (auto* current = component; current != nullptr;
                 current = current->getParentComponent())
                if (current == &owner)
                    return true;
            return false;
        };
        juce::String controlTitle;
        const std::array<juce::Slider*, 12> sequencerSliders
        {{ &rateSlider, &startSlider, &endSlider,
           &attackSlider, &releaseSlider, &peakSlider,
           &moveSlider, &colourSlider, &saturationSlider,
           &luminanceSlider, &shortGateSlider, &longGateSlider }};
        for (auto* slider : sequencerSliders)
            if (isWithin(event.eventComponent, *slider))
            {
                controlTitle = slider->getName().trim();
                break;
            }

        for (auto* component = event.eventComponent;
             component != nullptr && component != this;
             component = component->getParentComponent())
        {
            if (auto* tooltipClient = dynamic_cast<juce::TooltipClient*>(component))
            {
                const auto tooltip = tooltipClient->getTooltip();
                if (tooltip.isNotEmpty())
                {
                    auto title = controlTitle.isNotEmpty()
                        ? controlTitle : component->getName().trim();
                    if (title.isEmpty())
                        if (auto* label = dynamic_cast<juce::Label*>(component))
                            title = label->getText().trim();
                    if (title.isEmpty())
                        title = "Sequencer";
                    showHelp(title.toUpperCase() + "\n" + tooltip);
                    return;
                }
            }
        }
    }

    void configureKnob(juce::Slider& slider, double minimum,
                       double maximum, double interval)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 18);
        slider.setRange(minimum, maximum, interval);
        slider.setMouseDragSensitivity(80);
        slider.setColour(juce::Slider::rotarySliderFillColourId,
                         knobIndicatorColour);
        slider.setColour(juce::Slider::textBoxTextColourId, knobValueColour);
        slider.setColour(juce::Slider::textBoxBackgroundColourId,
                         juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::textBoxOutlineColourId,
                         juce::Colours::transparentBlack);
    }

    void updateSequencerControlEnablement()
    {
        const bool shouldEnable = enabledButton.getToggleState();
        if (sequencerControlsEnabled == shouldEnable)
            return;

        sequencerControlsEnabled = shouldEnable;
        const std::array<juce::Component*, 34> controls
        {{ &hostSync, &bipolarButton, &animateButton, &gateModeButton,
           &waveformBox, &userSequenceBox, &directionBox,
           &rateSlider, &startSlider, &endSlider, &attackSlider,
           &releaseSlider, &peakSlider, &moveSlider, &colourSlider,
           &saturationSlider, &luminanceSlider,
           &startLabel, &endLabel, &attackLabel, &releaseLabel,
           &peakLabel, &moveLabel, &shortGateLabel, &longGateLabel,
           &nudgeLeft, &nudgeRight, &gateNudgeLeft, &gateNudgeRight,
           &rangeLinkButton, &shortGateSlider, &longGateSlider,
           &grid, &targets }};
        for (auto* control : controls)
        {
            control->setEnabled(shouldEnable);
            control->setAlpha(shouldEnable ? 1.0f : 0.48f);
        }
        repaint();
    }

    void configureVerticalSlider(juce::Slider& slider, double minimum,
                                 double maximum, double interval)
    {
        slider.setSliderStyle(juce::Slider::LinearVertical);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setRange(minimum, maximum, interval);
        slider.setMouseDragSensitivity(80);
        slider.setColour(juce::Slider::rotarySliderFillColourId,
                         knobIndicatorColour);
    }

    std::unique_ptr<SliderAttachment> makeSliderAttachment(
        const juce::String& id, juce::Slider& slider)
    {
        return std::make_unique<SliderAttachment>(processor.parameters,
                                                   id, slider);
    }

    void configureSavedReset(DetentedSlider& slider, juce::String id)
    {
        slider.onSavedStateReset = [this, &slider, id = std::move(id)]
        {
            const auto saved = processor.getSavedParameterValue(id);
            if (! saved.has_value())
                return false;
            slider.setValue(*saved, juce::sendNotificationSync);
            return true;
        };
    }

    void updateAccent()
    {
        // Target labels and the three colour controls must remain readable.
        // LUM deliberately affects only the step display.
        accent = juce::Colour::fromHSV(
            static_cast<float>(colourSlider.getValue()),
            static_cast<float>(saturationSlider.getValue()),
            0.72f, 1.0f);
        const auto stepAccent = juce::Colour::fromHSV(
            static_cast<float>(colourSlider.getValue()),
            static_cast<float>(saturationSlider.getValue()),
            static_cast<float>(luminanceSlider.getValue()), 1.0f);
        colourSlider.setColour(juce::Slider::rotarySliderFillColourId, accent);
        saturationSlider.setColour(juce::Slider::rotarySliderFillColourId,
                                   accent);
        luminanceSlider.setColour(juce::Slider::rotarySliderFillColourId,
                                  accent);
        grid.setAccent(stepAccent);
        updateTargetHighlights();
        repaint();
    }

    void updateAnimatedControls()
    {
        const auto update = [this](juce::Slider& slider,
                                   const juce::String& parameterId)
        {
            const auto value = animateButton.getToggleState()
                ? processor.getSequencerAnimatedNormalised(parameterId)
                : std::nullopt;
            auto& sliderProperties = slider.getProperties();
            if (value.has_value())
            {
                const bool changed = ! sliderProperties.contains("animatedNormalised")
                    || std::abs(static_cast<float>(
                           sliderProperties["animatedNormalised"]) - *value)
                           > 0.000001f;
                if (changed)
                {
                    sliderProperties.set("animatedNormalised", *value);
                    slider.repaint();
                }
            }
            else if (sliderProperties.contains("animatedNormalised"))
            {
                sliderProperties.remove("animatedNormalised");
                slider.repaint();
            }
        };
        update(attackSlider, "sequencerAttack");
        update(releaseSlider, "sequencerRelease");
        update(peakSlider, "sequencerPeak");
        update(moveSlider, "sequencerMove");
        update(startSlider, "sequencerStart");
        update(endSlider, "sequencerEnd");
        update(shortGateSlider, "sequencerGateShortLength");
        update(longGateSlider, "sequencerGateLongLength");
    }

    void refreshUserSequences()
    {
        const auto selectedText = userSequenceBox.getText();
        userSequenceFiles.clear();
        const auto directory = AfterglowAudioProcessor::getSequencesDirectory();
        directory.createDirectory();
        directory.findChildFiles(userSequenceFiles, juce::File::findFiles,
                                 false, "*.agseq");
        std::sort(userSequenceFiles.begin(), userSequenceFiles.end(),
            [](const juce::File& left, const juce::File& right)
            {
                return left.getFileNameWithoutExtension().compareNatural(
                           right.getFileNameWithoutExtension(), true) < 0;
            });
        userSequenceBox.clear(juce::dontSendNotification);
        for (int index = 0; index < userSequenceFiles.size(); ++index)
            userSequenceBox.addItem(
                userSequenceFiles[index].getFileNameWithoutExtension(),
                index + 1);
        userSequenceBox.setTextWhenNothingSelected(
            selectedText.isNotEmpty() ? selectedText : "USER");
    }

    void loadSelectedUserSequence()
    {
        const int index = userSequenceBox.getSelectedId() - 1;
        if (! juce::isPositiveAndBelow(index, userSequenceFiles.size()))
            return;
        const auto xml = juce::XmlDocument::parse(userSequenceFiles[index]);
        if (xml == nullptr || ! xml->hasTagName("AfterglowSequence"))
            return;
        if (xml->hasAttribute("gateShort"))
            shortGateSlider.setValue(
                xml->getDoubleAttribute("gateShort", 0.50),
                juce::sendNotificationSync);
        if (xml->hasAttribute("gateLong"))
            longGateSlider.setValue(
                xml->getDoubleAttribute("gateLong", 0.90),
                juce::sendNotificationSync);
        for (auto* stepElement : xml->getChildWithTagNameIterator("Step"))
        {
            const int step = stepElement->getIntAttribute("index", -1);
            if (! juce::isPositiveAndBelow(step,
                                           afterglow::sequencer::stepCount))
                continue;
            setParameterActualValue(processor.parameters.getParameter(
                afterglow::sequencer::stepParameterId(step)),
                static_cast<float>(stepElement->getDoubleAttribute("value", 1.0)));
            if (stepElement->hasAttribute("gate"))
                setParameterActualValue(processor.parameters.getParameter(
                    afterglow::sequencer::gateStepParameterId(step)),
                    static_cast<float>(stepElement->getDoubleAttribute(
                        "gate", static_cast<double>(
                            static_cast<int>(
                                afterglow::sequencer::GateStepMode::rest)))));
        }
        grid.repaint();
    }

    void saveUserSequence()
    {
        sequenceFileChooser = std::make_unique<juce::FileChooser>(
            "Save Afterglow User Sequence",
            AfterglowAudioProcessor::getSequencesDirectory()
                .getChildFile("New Sequence.agseq"), "*.agseq");
        const juce::Component::SafePointer<SequencerPanel> safeThis(this);
        sequenceFileChooser->launchAsync(
            juce::FileBrowserComponent::saveMode
                | juce::FileBrowserComponent::canSelectFiles,
            [safeThis](const juce::FileChooser& chooser)
            {
                if (safeThis == nullptr)
                    return;
                auto file = chooser.getResult();
                if (file == juce::File())
                    return;
                if (! file.hasFileExtension(".agseq"))
                    file = file.withFileExtension(".agseq");
                auto xml = std::make_unique<juce::XmlElement>(
                    "AfterglowSequence");
                xml->setAttribute("gateShort",
                    safeThis->shortGateSlider.getValue());
                xml->setAttribute("gateLong",
                    safeThis->longGateSlider.getValue());
                for (int step = 0; step < afterglow::sequencer::stepCount;
                     ++step)
                {
                    auto* element = xml->createNewChildElement("Step");
                    element->setAttribute("index", step);
                    const auto read = [&safeThis](const juce::String& id)
                    {
                        auto* parameter = safeThis->processor.parameters
                            .getParameter(id);
                        return parameter != nullptr
                            ? parameter->convertFrom0to1(parameter->getValue())
                            : 0.0f;
                    };
                    element->setAttribute("value", read(
                        afterglow::sequencer::stepParameterId(step)));
                    element->setAttribute("gate", read(
                        afterglow::sequencer::gateStepParameterId(step)));
                }
                if (xml->writeTo(file))
                    safeThis->refreshUserSequences();
                safeThis->sequenceFileChooser.reset();
            });
    }

    AfterglowAudioProcessor& processor;
    std::function<juce::Colour()> surroundColour;
    std::function<void(const juce::String&)> showHelp;
    LookAndFeel patternMenuLookAndFeel { true };
    StepGrid grid;
    TargetList targets;
    juce::ToggleButton hostSync;
    juce::ToggleButton enabledButton;
    juce::ToggleButton bipolarButton;
    juce::ToggleButton animateButton;
    juce::ToggleButton gateModeButton;
    juce::ToggleButton rangeLinkButton;
    juce::ComboBox waveformBox;
    juce::ComboBox userSequenceBox;
    juce::ComboBox directionBox;
    juce::ToggleButton nudgeLeft;
    juce::ToggleButton nudgeRight;
    juce::ToggleButton gateNudgeLeft;
    juce::ToggleButton gateNudgeRight;
    DetentedSlider rateSlider;
    DetentedSlider startSlider;
    DetentedSlider endSlider;
    DetentedSlider attackSlider;
    DetentedSlider releaseSlider;
    DetentedSlider peakSlider;
    DetentedSlider moveSlider;
    DetentedSlider shortGateSlider;
    DetentedSlider longGateSlider;
    ParameterDragLabel startLabel;
    ParameterDragLabel endLabel;
    ParameterDragLabel attackLabel;
    ParameterDragLabel releaseLabel;
    ParameterDragLabel peakLabel;
    ParameterDragLabel moveLabel;
    ParameterDragLabel shortGateLabel;
    ParameterDragLabel longGateLabel;
    juce::Slider colourSlider;
    juce::Slider saturationSlider;
    juce::Slider luminanceSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
        hostSyncAttachment, enabledAttachment,
        rangeLinkAttachment;
    std::unique_ptr<juce::ParameterAttachment> bipolarModeAttachment,
        negativeModeAttachment, gateModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        directionAttachment;
    std::unique_ptr<SliderAttachment> rateAttachment, startAttachment,
        endAttachment, attackAttachment, releaseAttachment, peakAttachment,
        moveAttachment, colourAttachment, saturationAttachment,
        luminanceAttachment, shortGateAttachment, longGateAttachment;
    juce::Array<juce::File> userSequenceFiles;
    std::unique_ptr<juce::FileChooser> sequenceFileChooser;
    juce::Colour accent { 0xff34d6c6 };
    int linkedRangeLength = afterglow::sequencer::stepCount - 1;
    int displayedPolarityMode = -1;
    int displayedGateMode = -1;
    bool updatingPolarityParameters = false;
    bool updatingGateModeParameter = false;
    bool updatingLinkedRange = false;
    bool sequencerControlsEnabled = true;
};

void AfterglowAudioProcessorEditor::PresetDisplayLabel::paint(
    juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat().reduced(0.5f);
    g.setColour(findColour(juce::Label::backgroundColourId));
    g.fillRoundedRectangle(bounds, 3.0f);
    g.setColour(findColour(juce::Label::outlineColourId));
    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
    g.setColour(isEnabled()
        ? findColour(juce::Label::textColourId)
        : findColour(juce::Label::textColourId).darker(0.45f));
    g.setFont(getFont());
    g.drawFittedText(getText(), getLocalBounds().reduced(6, 1),
                     getJustificationType(), 1, 0.72f);
}

void AfterglowAudioProcessorEditor::PresetDisplayLabel::mouseDown(
    const juce::MouseEvent& event)
{
    if (isEnabled() && event.mods.isLeftButtonDown() && onShowMenu != nullptr)
        onShowMenu();
}

void AfterglowAudioProcessorEditor::PresetDisplayLabel::mouseWheelMove(
    const juce::MouseEvent&, const juce::MouseWheelDetails& wheel)
{
    if (! isEnabled() || onStep == nullptr
        || std::abs(wheel.deltaY) < 0.0001f)
        return;

    const int direction = wheel.deltaY > 0.0f ? -1 : 1;
    const auto now = juce::Time::getMillisecondCounter();
    if (direction == lastWheelDirection && now - lastWheelTimeMs < 100u)
        return;

    lastWheelDirection = direction;
    lastWheelTimeMs = now;
    onStep(direction);
}

void AfterglowAudioProcessorEditor::ParameterDragLabel::setTargetAppearance(
    bool assigned, juce::Colour accent)
{
    if (targetAssigned == assigned && targetAccent == accent)
        return;

    targetAssigned = assigned;
    targetAccent = accent;
    setColour(juce::Label::textColourId,
              targetAssigned ? targetAccent : knobLabelColour);
    repaint();
}

void AfterglowAudioProcessorEditor::ParameterDragLabel::paint(
    juce::Graphics& graphics)
{
    const auto parameterId = parameterIdProvider != nullptr
        ? parameterIdProvider() : juce::String();
    const bool draggable = afterglow::sequencer::targetChoiceForParameter(
        parameterId) > 0;
    if (! draggable)
    {
        if (! dragHandleOnly)
            juce::Label::paint(graphics);
        return;
    }

    auto colour = targetAssigned ? targetAccent : knobLabelColour;
    if (! isEnabled())
        colour = colour.darker(0.42f);

    const auto caption = dragHandleOnly ? juce::String()
                                        : getText().trim();
    graphics.setColour(colour);
    const auto labelFont = getFont();
    graphics.setFont(labelFont);
    if (caption.isEmpty())
    {
        graphics.drawFittedText("::", getLocalBounds(),
                                juce::Justification::centred, 1, 0.72f);
        return;
    }

    // Keep the caption itself centred over its knob.  The drag marker belongs
    // to this same label component, but sits one measured space outside the
    // centred caption so it cannot pull the visible control name sideways.
    graphics.drawFittedText(caption, getLocalBounds(),
                            getJustificationType(), 1, 0.72f);
    const float captionWidth = juce::GlyphArrangement::getStringWidth(
        labelFont, caption);
    const float markerAdvance = juce::GlyphArrangement::getStringWidth(
        labelFont, "::");
    const float gap = juce::GlyphArrangement::getStringWidth(labelFont, " ");
    const float captionLeft = static_cast<float>(getWidth()) * 0.5f
                            - captionWidth * 0.5f;
    const float markerX = dragHandleRight
        ? captionLeft + captionWidth + gap
        : captionLeft - gap - markerAdvance;
    // Give the glyphs a little drawing room beyond their typographic advance;
    // otherwise JUCE can clip the outer colon at small GUI scales.
    constexpr float markerPadding = 2.0f;
    const int markerLeft = juce::roundToInt(markerX - markerPadding);
    const int markerWidth = juce::jmax(
        1, juce::roundToInt(markerAdvance + markerPadding * 2.0f));
    graphics.drawText("::", markerLeft, 0, markerWidth, getHeight(),
                      getJustificationType(), false);
}

void AfterglowAudioProcessorEditor::ParameterDragLabel::mouseDrag(
    const juce::MouseEvent& event)
{
    if (event.getDistanceFromDragStart() < 4)
        return;
    const auto parameterId = parameterIdProvider != nullptr
        ? parameterIdProvider() : juce::String();
    if (afterglow::sequencer::targetChoiceForParameter(parameterId) <= 0)
        return;
    if (auto* container = juce::DragAndDropContainer::findParentDragContainerFor(this))
        if (! container->isDragAndDropActive())
            container->startDragging(
                juce::var(juce::String(sequencerTargetDragPrefix) + parameterId),
                this);
}

AfterglowAudioProcessorEditor::LookAndFeel::LookAndFeel(
    bool compactPopupFont)
    : useCompactPopupFont(compactPopupFont)
{
    setColour(juce::Slider::textBoxTextColourId, knobValueColour);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff14181d));
    setColour(juce::PopupMenu::textColourId, knobValueColour);
    setColour(juce::PopupMenu::highlightedBackgroundColourId,
              juce::Colour(0xff2a3037));
    setColour(juce::PopupMenu::highlightedTextColourId,
              knobValueColour);
}

juce::Font AfterglowAudioProcessorEditor::LookAndFeel::getPopupMenuFont()
{
    auto font = juce::LookAndFeel_V4::getPopupMenuFont();
    if (useCompactPopupFont)
        font.setHeight(juce::jmax(9.0f, font.getHeight() - 1.5f));
    return font;
}

juce::Font AfterglowAudioProcessorEditor::LookAndFeel::getComboBoxFont(
    juce::ComboBox& box)
{
    auto font = juce::LookAndFeel_V4::getComboBoxFont(box);
    if (static_cast<bool>(box.getProperties().getWithDefault(
            "compactClosedCombo", false)))
        font.setHeight(font.getHeight() * 0.85f);
    return font;
}

juce::Label* AfterglowAudioProcessorEditor::LookAndFeel::createSliderTextBox(juce::Slider& slider)
{
    auto* label = juce::LookAndFeel_V4::createSliderTextBox(slider);
    label->setFont(juce::Font(juce::FontOptions(12.5f)));
    return label;
}

void AfterglowAudioProcessorEditor::LookAndFeel::drawRotarySlider(
    juce::Graphics& g, int x, int y, int width, int height,
    float position, float startAngle, float endAngle, juce::Slider& slider)
{
    if (slider.getProperties().contains("animatedNormalised"))
        position = juce::jlimit(0.0f, 1.0f, static_cast<float>(
            slider.getProperties()["animatedNormalised"]));
    const auto radius = static_cast<float>(juce::jmin(width, height)) * 0.40f + 2.0f;
    const auto centre = juce::Point<float>(static_cast<float>(x + width) * 0.5f,
                                           static_cast<float>(y + height) * 0.5f);
    const auto angle = startAngle + position * (endAngle - startAngle);
    const bool enabled = slider.isEnabled();
    const auto trackColour = enabled ? knobTrackColour : juce::Colour(0xff25292e);
    const auto bodyColour = enabled ? knobBodyColour : juce::Colour(0xff202328);
    auto indicatorColour = enabled ? slider.findColour(juce::Slider::rotarySliderFillColourId)
                                   : juce::Colour(0xff555b63);

    if (enabled && slider.isMouseOverOrDragging())
        indicatorColour = indicatorColour.brighter(0.10f);

    float fillStartAngle = startAngle;
    float fillEndAngle = angle;
    if (static_cast<bool>(slider.getProperties().getWithDefault("hasCentreDetent", false)))
    {
        const double centreDetent = static_cast<double>(slider.getProperties()
            .getWithDefault("centreDetentValue", 0.0));
        const auto normalisedCentre = static_cast<float>(slider.getNormalisableRange()
            .convertTo0to1(centreDetent));
        const float centreAngle = startAngle + normalisedCentre * (endAngle - startAngle);
        fillStartAngle = juce::jmin(angle, centreAngle);
        fillEndAngle = juce::jmax(angle, centreAngle);
    }

    constexpr float ringThickness = 2.0f;
    juce::Path track;
    track.addCentredArc(centre.x, centre.y, radius, radius,
                        0.0f, startAngle, endAngle, true);
    g.setColour(trackColour);
    g.strokePath(track, juce::PathStrokeType(ringThickness,
                                              juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));

    if (std::abs(fillEndAngle - fillStartAngle) > 0.0001f)
    {
        juce::Path fill;
        fill.addCentredArc(centre.x, centre.y, radius, radius,
                           0.0f, fillStartAngle, fillEndAngle, true);
        g.setColour(indicatorColour);
        g.strokePath(fill, juce::PathStrokeType(ringThickness,
                                                 juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
    }

    g.setColour(bodyColour);
    g.fillEllipse(centre.x - radius * 0.72f,
                  centre.y - radius * 0.72f,
                  radius * 1.44f,
                  radius * 1.44f);
    g.setColour(indicatorColour);
    g.drawLine(centre.x, centre.y,
               centre.x + std::sin(angle) * radius * 0.58f,
               centre.y - std::cos(angle) * radius * 0.58f,
               2.0f);
}

void AfterglowAudioProcessorEditor::LookAndFeel::drawLinearSlider(
    juce::Graphics& graphics, int x, int y, int width, int height,
    float sliderPosition, float minSliderPosition, float maxSliderPosition,
    juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (slider.getProperties().contains("animatedNormalised"))
    {
        const float animated = juce::jlimit(
            0.0f, 1.0f, static_cast<float>(
                slider.getProperties()["animatedNormalised"]));
        sliderPosition = style == juce::Slider::LinearVertical
            ? static_cast<float>(y + height - 1)
                - animated * static_cast<float>(height - 2)
            : static_cast<float>(x + 1)
                + animated * static_cast<float>(width - 2);
    }
    if (style != juce::Slider::LinearVertical
        && style != juce::Slider::LinearHorizontal)
    {
        juce::LookAndFeel_V4::drawLinearSlider(
            graphics, x, y, width, height, sliderPosition,
            minSliderPosition, maxSliderPosition, style, slider);
        return;
    }

    const bool enabled = slider.isEnabled();
    const auto accent = enabled
        ? slider.findColour(juce::Slider::rotarySliderFillColourId)
        : juce::Colour(0xff555b63);
    if (style == juce::Slider::LinearHorizontal)
    {
        const float left = static_cast<float>(x + 1);
        const float right = static_cast<float>(x + width - 1);
        const float centreX = 0.5f * (left + right);
        const float centreY = static_cast<float>(y + height / 2);
        graphics.setColour(enabled
            ? juce::Colour(0xffb5bac0).withAlpha(0.86f)
            : juce::Colour(0xff555b63).withAlpha(0.58f));
        graphics.drawLine(left, centreY, right, centreY, 1.25f);
        graphics.drawLine(centreX, centreY - 4.0f,
                          centreX, centreY + 4.0f, 1.0f);
        graphics.setColour(accent);
        graphics.fillRoundedRectangle(sliderPosition - 2.0f,
                                      centreY - 6.0f,
                                      4.0f, 12.0f, 1.0f);
        return;
    }

    const float centreX = static_cast<float>(x + width / 2);
    const float top = static_cast<float>(y + 1);
    const float bottom = static_cast<float>(y + height - 1);
    const float centreY = 0.5f * (top + bottom);
    graphics.setColour(enabled
        ? knobTrackColour.withAlpha(0.90f)
        : juce::Colour(0xff555b63).withAlpha(0.58f));
    graphics.drawLine(centreX, top, centreX, bottom, 1.25f);
    graphics.setColour(accent.withAlpha(enabled ? 0.92f : 0.42f));
    graphics.drawLine(centreX, centreY, centreX, sliderPosition, 2.6f);
    graphics.setColour(enabled
        ? knobTrackColour.withAlpha(0.90f)
        : juce::Colour(0xff555b63).withAlpha(0.58f));
    graphics.drawLine(centreX - 4.0f, centreY,
                      centreX + 4.0f, centreY, 1.0f);
    graphics.setColour(accent);
    graphics.fillRoundedRectangle(centreX - 6.0f,
                                  sliderPosition - 2.0f,
                                  12.0f, 4.0f, 1.0f);
}

void AfterglowAudioProcessorEditor::LookAndFeel::drawToggleButton(
    juce::Graphics& g, juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    const auto bounds = button.getLocalBounds().toFloat();
    const bool ledOnly = static_cast<bool>(button.getProperties().getWithDefault("ledOnly", false));
    const bool statusLedOnly = static_cast<bool>(
        button.getProperties().getWithDefault("statusLedOnly", false));
    const bool snowflakeIcon = static_cast<bool>(
        button.getProperties().getWithDefault("snowflakeIcon", false));
    const bool listIcon = static_cast<bool>(
        button.getProperties().getWithDefault("listIcon", false));
    const int presetArrowDirection = static_cast<int>(
        button.getProperties().getWithDefault("presetArrowDirection", 0));
    const bool saveIcon = static_cast<bool>(
        button.getProperties().getWithDefault("saveIcon", false));
    const bool saveAsIcon = static_cast<bool>(
        button.getProperties().getWithDefault("saveAsIcon", false));
    const bool folderIcon = static_cast<bool>(
        button.getProperties().getWithDefault("folderIcon", false));
    const bool rangeLinkIcon = static_cast<bool>(
        button.getProperties().getWithDefault("rangeLinkIcon", false));
    const bool randomIcon = static_cast<bool>(
        button.getProperties().getWithDefault("randomIcon", false));
    const bool mixLockIcon = static_cast<bool>(
        button.getProperties().getWithDefault("mixLockIcon", false));
    const bool sequencerLabelStyle = static_cast<bool>(
        button.getProperties().getWithDefault("sequencerLabelStyle", false));
    const bool standardTextButton = static_cast<bool>(
        button.getProperties().getWithDefault("standardTextButton", false));
    const bool invisibleButton = static_cast<bool>(
        button.getProperties().getWithDefault("invisibleButton", false));
    if (invisibleButton)
        return;
    const bool active = button.getToggleState();
    const bool enabled = button.isEnabled();
    auto activeColour = ledOnly ? juce::Colour(0xffff9363) : controlAccentColour;
    const auto configuredColour = button.getProperties()
        .getWithDefault("activeColour", juce::var()).toString();
    if (configuredColour.isNotEmpty())
        activeColour = juce::Colour::fromString(configuredColour);

    if (shouldDrawButtonAsHighlighted)
        activeColour = activeColour.brighter(0.10f);
    if (shouldDrawButtonAsDown)
        activeColour = activeColour.darker(0.15f);

    if (snowflakeIcon || listIcon || presetArrowDirection != 0
        || saveIcon || saveAsIcon || folderIcon || rangeLinkIcon
        || randomIcon || mixLockIcon || standardTextButton)
    {
        const auto buttonBounds = bounds.reduced(0.5f);
        if (! mixLockIcon)
        {
            g.setColour(active && enabled ? activeColour.darker(0.32f)
                                         : juce::Colour(0xff242a30));
            g.fillRoundedRectangle(buttonBounds, 3.0f);
            g.setColour(active && enabled ? activeColour.brighter(0.12f)
                                         : (shouldDrawButtonAsHighlighted
                                                ? knobLabelColour
                                                : juce::Colour(0xff59616a)));
            g.drawRoundedRectangle(buttonBounds, 3.0f, 1.0f);
        }

        const auto iconColour = mixLockIcon
            ? (active && enabled ? activeColour
                                 : juce::Colour(0xff69717b))
            : presetArrowDirection != 0
            ? knobIndicatorColour.withAlpha(enabled ? 0.9f : 0.2f)
            : (enabled
                   ? (active ? juce::Colours::white : knobValueColour)
                   : knobLabelColour.darker(0.45f));
        g.setColour(iconColour);
        if (mixLockIcon)
        {
            const auto centre = bounds.getCentre();
            const auto body = juce::Rectangle<float>(10.0f, 8.0f)
                .withCentre(centre.translated(0.0f, 2.5f));
            const auto shackle = juce::Rectangle<float>(7.0f, 8.0f)
                .withCentre(centre.translated(0.0f, -2.0f));
            g.drawRoundedRectangle(shackle, 3.5f, 1.5f);
            g.fillRoundedRectangle(body, 1.5f);
            g.setColour(juce::Colour(0xff242a30));
            g.fillEllipse(centre.x - 1.0f, centre.y + 1.0f,
                          2.0f, 2.0f);
        }
        else if (randomIcon)
        {
            g.setFont(juce::Font(juce::FontOptions(
                15.0f, juce::Font::bold)));
            g.drawText("!!!", button.getLocalBounds(),
                       juce::Justification::centred, false);
        }
        else if (standardTextButton)
        {
            g.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
            g.drawFittedText(button.getButtonText(),
                             button.getLocalBounds().reduced(5, 2),
                             juce::Justification::centred, 1);
        }
        else if (rangeLinkIcon)
        {
            const auto centre = bounds.getCentre();
            const float iconScale = static_cast<bool>(
                button.getProperties().getWithDefault(
                    "compactRangeLink", false)) ? 0.75f : 1.0f;
            const auto upper = juce::Rectangle<float>(
                10.0f * iconScale, 7.0f * iconScale)
                .withCentre(centre.translated(-3.5f * iconScale,
                                              -2.5f * iconScale));
            const auto lower = juce::Rectangle<float>(
                10.0f * iconScale, 7.0f * iconScale)
                .withCentre(centre.translated(3.5f * iconScale,
                                              2.5f * iconScale));
            g.drawRoundedRectangle(upper, 3.0f * iconScale,
                                   1.4f * iconScale);
            g.drawRoundedRectangle(lower, 3.0f * iconScale,
                                   1.4f * iconScale);
            g.drawLine(centre.x - 2.0f * iconScale,
                       centre.y - 1.5f * iconScale,
                       centre.x + 2.0f * iconScale,
                       centre.y + 1.5f * iconScale,
                       1.4f * iconScale);
        }
        else if (snowflakeIcon)
        {
            const auto centre = bounds.getCentre();
            const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight())
                               * 0.275f;
            for (int axis = 0; axis < 3; ++axis)
            {
                const float angle = static_cast<float>(axis)
                                  * juce::MathConstants<float>::pi / 3.0f;
                const juce::Point<float> direction(std::cos(angle),
                                                    std::sin(angle));
                g.drawLine(centre.x - direction.x * radius,
                           centre.y - direction.y * radius,
                           centre.x + direction.x * radius,
                           centre.y + direction.y * radius, 1.5f);
            }
            g.fillEllipse(centre.x - 1.25f, centre.y - 1.25f, 2.5f, 2.5f);
        }
        else if (listIcon)
        {
            const float left = bounds.getX() + bounds.getWidth() * 0.26f;
            const float lineLeft = bounds.getX() + bounds.getWidth() * 0.43f;
            const float lineRight = bounds.getRight() - bounds.getWidth() * 0.22f;
            for (int row = 0; row < 3; ++row)
            {
                const float y = bounds.getY() + bounds.getHeight()
                              * (0.31f + static_cast<float>(row) * 0.19f);
                g.fillEllipse(left - 1.25f, y - 1.25f, 2.5f, 2.5f);
                g.drawLine(lineLeft, y, lineRight, y, 1.5f);
            }
        }
        else if (presetArrowDirection != 0)
        {
            const auto centre = bounds.getCentre();
            const float arrowScale = static_cast<bool>(
                button.getProperties().getWithDefault(
                    "compactPresetArrow", false)) ? 0.75f : 1.0f;
            const float innerOffset = 2.0f * arrowScale;
            const float pointOffset = 3.0f * arrowScale;
            const float halfHeight = 7.0f * arrowScale;
            juce::Path arrow;
            if (presetArrowDirection < 0)
            {
                arrow.startNewSubPath(centre.x + innerOffset,
                                      centre.y - halfHeight);
                arrow.lineTo(centre.x - pointOffset, centre.y);
                arrow.lineTo(centre.x + innerOffset,
                             centre.y + halfHeight);
            }
            else
            {
                arrow.startNewSubPath(centre.x - innerOffset,
                                      centre.y - halfHeight);
                arrow.lineTo(centre.x + pointOffset, centre.y);
                arrow.lineTo(centre.x - innerOffset,
                             centre.y + halfHeight);
            }
            g.strokePath(arrow, juce::PathStrokeType(2.0f * arrowScale));
        }
        else if (folderIcon)
        {
            const auto icon = juce::Rectangle<float>(14.0f, 10.0f)
                .withCentre(bounds.getCentre().translated(0.0f, 1.0f));
            g.fillRoundedRectangle(icon, 1.5f);
            g.fillRect(icon.getX() + 1.5f, icon.getY() - 2.0f,
                       5.0f, 3.0f);
        }
        else
        {
            const auto icon = bounds.withSizeKeepingCentre(14.0f, 14.0f);

            g.drawRoundedRectangle(icon, 1.5f, 1.4f);
            g.fillRect(icon.getX() + 3.0f, icon.getY(),
                       icon.getWidth() - 6.0f, 5.0f);
            g.setColour(juce::Colour(0xff242a30));
            g.fillRect(icon.getX() + 5.0f, icon.getY() + 1.0f,
                       icon.getWidth() - 9.0f, 3.0f);
            g.setColour(iconColour);
            g.drawRect(icon.getX() + 3.5f, icon.getBottom() - 7.0f,
                       icon.getWidth() - 7.0f, 5.0f, 1.2f);

            if (saveAsIcon)
            {
                const auto pencilStart = juce::Point<float>(
                    icon.getRight() - 4.0f, icon.getBottom() + 2.5f);
                const auto pencilEnd = juce::Point<float>(
                    icon.getRight() + 3.0f, icon.getBottom() - 4.5f);
                g.drawLine({ pencilStart, pencilEnd }, 2.8f);
                g.fillEllipse(pencilEnd.x - 1.6f, pencilEnd.y - 1.6f,
                              3.2f, 3.2f);
                g.drawLine(pencilStart.x - 1.3f, pencilStart.y + 1.3f,
                           pencilStart.x + 1.3f, pencilStart.y - 1.3f,
                           1.3f);
            }
        }
        return;
    }

    const bool centredLed = ledOnly || statusLedOnly;
    const float configuredLedDiameter = static_cast<float>(
        button.getProperties().getWithDefault("ledDiameter", -1.0f));
    const float diameter = configuredLedDiameter > 0.0f
                         ? configuredLedDiameter
                         : centredLed
                             ? juce::jmin(bounds.getWidth(),
                                          bounds.getHeight()) - 6.0f
                             : 10.0f;
    const juce::Font toggleFont(sequencerLabelStyle
        ? juce::FontOptions(10.5f, juce::Font::bold)
        : juce::FontOptions(11.0f));
    const float textWidth = statusLedOnly ? 0.0f
        : juce::GlyphArrangement::getStringWidth(toggleFont, button.getButtonText());
    const float contentWidth = diameter + 5.0f + textWidth;
    const float contentX = bounds.getCentreX() - contentWidth * 0.5f;
    const float fixedLedCentreX = static_cast<float>(button.getProperties()
        .getWithDefault("fixedLedCentreX", -1.0f));
    const juce::Point<float> centre = centredLed
        ? bounds.getCentre()
        : juce::Point<float>(fixedLedCentreX >= 0.0f
                                 ? fixedLedCentreX
                                 : contentX + diameter * 0.5f,
                             bounds.getCentreY());
    const auto ledBounds = juce::Rectangle<float>(diameter, diameter).withCentre(centre);
    if (active && enabled)
    {
        g.setColour(activeColour.withAlpha(0.20f));
        g.fillEllipse(ledBounds.expanded(3.0f));
    }

    if (statusLedOnly)
    {
        const auto disabledActiveColour = activeColour.darker(0.72f);
        g.setColour(enabled
            ? (active ? activeColour : juce::Colour(0xff343940))
            : (active ? disabledActiveColour : juce::Colour(0xff282c31)));
        g.fillEllipse(ledBounds);
        g.setColour(enabled
            ? (active ? activeColour.brighter(0.30f) : juce::Colour(0xff596069))
            : (active ? disabledActiveColour.brighter(0.12f)
                      : juce::Colour(0xff41474e)));
        g.drawEllipse(ledBounds, 1.0f);
        if (button.getButtonText().isNotEmpty())
        {
            g.setColour(enabled
                ? (active ? juce::Colour(0xff17201b) : knobValueColour)
                : knobValueColour.darker(0.45f));
            g.setFont(juce::Font(juce::FontOptions(8.0f, juce::Font::bold)));
            g.drawText(button.getButtonText(), ledBounds.toNearestInt(),
                       juce::Justification::centred, false);
        }
        return;
    }

    if (ledOnly)
    {
        g.setColour(juce::Colour(0xff24282e));
        g.fillEllipse(ledBounds);
        g.setColour(active ? activeColour : juce::Colour(0xff69717b));
        g.drawEllipse(ledBounds, 1.0f);

        const float iconRadius = diameter * 0.285f;
        const float gapAngle = juce::MathConstants<float>::pi * 0.23f;
        juce::Path powerRing;
        powerRing.addCentredArc(centre.x, centre.y, iconRadius, iconRadius, 0.0f,
                               gapAngle,
                               juce::MathConstants<float>::twoPi - gapAngle,
                               true);
        g.strokePath(powerRing, juce::PathStrokeType(1.8f,
                                                     juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
        g.drawLine(centre.x, centre.y - iconRadius * 0.82f,
                   centre.x, centre.y + iconRadius * 0.10f, 1.8f);
        return;
    }

    g.setColour(active ? activeColour : juce::Colour(0xff343940));
    g.fillEllipse(ledBounds);
    g.setColour(active ? activeColour.brighter(0.30f) : juce::Colour(0xff596069));
    g.drawEllipse(ledBounds, 1.0f);

    g.setColour(button.isEnabled()
        ? button.findColour(juce::ToggleButton::textColourId)
        : knobLabelColour.darker(0.35f));
    g.setFont(toggleFont);
    const int textX = juce::roundToInt(fixedLedCentreX >= 0.0f
        ? centre.x + diameter * 0.5f + 5.0f
        : contentX + diameter + 5.0f);
    g.drawFittedText(button.getButtonText(), textX, 0,
                     juce::jmax(0, button.getWidth() - textX), button.getHeight(),
                     juce::Justification::centredLeft, 1);
}

void AfterglowAudioProcessorEditor::LookAndFeel::drawPopupMenuItem(
    juce::Graphics& g, const juce::Rectangle<int>& area,
    bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
    bool hasSubMenu, const juce::String& text,
    const juce::String& shortcutKeyText, const juce::Drawable* icon,
    const juce::Colour* textColour)
{
    const auto insetArea = area.withTrimmedLeft(3).withTrimmedRight(3);
    juce::LookAndFeel_V4::drawPopupMenuItem(
        g, insetArea, isSeparator, isActive, isHighlighted, isTicked,
        hasSubMenu, text, shortcutKeyText, icon, textColour);
}

void AfterglowAudioProcessorEditor::LookAndFeel::drawPopupMenuBackground(
    juce::Graphics& g, int width, int height)
{
    g.fillAll(juce::Colour(0xff14181d));
    g.setColour(popupBorderColour);
    g.drawRect(0, 0, width, height, 2);
}

juce::PopupMenu::Options
AfterglowAudioProcessorEditor::LookAndFeel::getOptionsForComboBoxPopupMenu(
    juce::ComboBox& box, juce::Label& label)
{
    return juce::PopupMenu::Options()
        .withTargetComponent(&box)
        .withTargetScreenArea(box.getScreenBounds())
        .withPreferredPopupDirection(
            juce::PopupMenu::Options::PopupDirection::downwards)
        .withMinimumWidth(box.getWidth())
        .withMaximumNumColumns(1)
        .withStandardItemHeight(label.getHeight());
}

AfterglowAudioProcessorEditor::AfterglowAudioProcessorEditor(AfterglowAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    const auto storedSettings = loadInterfaceSettings();
    const auto presetsDirectory = AfterglowAudioProcessor::getPresetsDirectory();
    loadFavouritePresets();
    const auto storedPresetFile = processor.getCurrentPresetFile();
    const auto storedPresetFolder = storedPresetFile.getParentDirectory();
    if (storedPresetFile != juce::File()
        && (storedPresetFolder == presetsDirectory
            || storedPresetFolder.isAChildOf(presetsDirectory)))
    {
        currentPresetFile = storedPresetFile;
        selectedPresetFolder = storedPresetFolder;
    }
    logoImage = juce::ImageFileFormat::loadFrom(
        BinaryData::logo_png, static_cast<size_t>(BinaryData::logo_pngSize));
    setLookAndFeel(&lookAndFeel);
    addAndMakeVisible(content);
    content.addMouseListener(this, true);
    content.setInterceptsMouseClicks(false, true);
    logoMenuButton.setName("GUI Zoom");
    logoMenuButton.getProperties().set("invisibleButton", true);
    logoMenuButton.setClickingTogglesState(false);
    logoMenuButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    logoMenuButton.setTooltip(
        "Click the Afterglow logo to choose a GUI zoom factor from 75% to 300%");
    logoMenuButton.onClick = [this] { showLogoZoomMenu(); };
    content.addAndMakeVisible(logoMenuButton);
    versionLabel.setText("v1.89.1", juce::dontSendNotification);
    versionLabel.setJustificationType(juce::Justification::centred);
    versionLabel.setColour(juce::Label::textColourId, knobIndicatorColour);
    versionLabel.setFont(juce::Font(juce::FontOptions(8.5f, juce::Font::bold)));
    content.addAndMakeVisible(versionLabel);
    leftTimeDial = addDial("timeL", "LEFT TIME", "Left delay time. Tempo Sync switches this same knob between milliseconds and musical note values.");
    rightTimeDial = addDial("timeR", "RIGHT TIME", "Right delay time. Tempo Sync switches this same knob between milliseconds and musical note values.");
    leftTimeDial->label.setText("L", juce::dontSendNotification);
    rightTimeDial->label.setText("R", juce::dontSendNotification);
    rightTimeDial->label.dragHandleRight = true;
    feedbackDial = addDial("feedback", "FDBK", "Controls repeat regeneration. Higher values lengthen the tail; 100% sustains and values above it regenerate.");
    addDial("cross", "CROSS", "0% repeats each channel into itself; 100% swaps Left and Right. Ping-Pong forces full cross-routing.");
    auto* spreadDial = addDial("spread", "SPREAD", "Sets delayed stereo width: 0% is centred, +100% is full width and negative values reverse Left and Right.");
    spreadDial->slider.setCentreDetent(0.0, 2.0);
    spreadDial->slider.setDoubleClickReturnValue(true, 0.0);
    addDial("duck", "DUCK", "Reduces audible delay while dry input is present. The feedback tail continues internally without attenuation.");
    addDial("duckAttack", "ATTACK", "How quickly the ducking envelope turns down the delayed signal after new input arrives.");
    addDial("duckRelease", "RELEASE", "How quickly the delayed signal returns after the dry input level falls.");
    addDial("diffusionAmount", "DIFF AMT", "Sets diffusion density and decay. At 0% it is off; high values form a reverb-like tail.");
    addDial("diffusionSize", "DIFF SIZE", "Sets the diffusion-cloud span without moving the selected delay taps.");
    modulationAmountDial = addDial("modulationAmount", "MOD AMT", "Depth of stereo delay-time movement in the selected mode.");
    addDial("modulationRate", "MOD RATE", "Sets modulation speed, including LoFi drift, Crystal triangle motion and the Phaser notch sweep.");
    mixDial = addDial("mix", "MIX", "0% is fully dry, 50% is an equal mix and 100% is fully wet. The centre has a tactile detent.");
    mixDial->slider.setCentreDetent(50.0, 2.0);
    mixDial->slider.setDoubleClickReturnValue(true, 50.0);
    lowCutDial = addDial("lowCut", "LO CUT", "Remove low frequencies from every echo, including the first. At minimum this filter is bypassed.");
    highCutDial = addDial("highCut", "HI CUT", "Remove high frequencies from every echo, including the first. At maximum this filter is bypassed.");
    auto* outputDial = addDial("output", "OUT", "Adjust the final output level. The 0 dB centre position has a tactile detent.");
    outputDial->slider.setCentreDetent(0.0, 0.6);

    modeBox.addItem("Analogue", 13);
    modeBox.addItem("Brigade", 3);
    modeBox.addItem("Chaotic", 14);
    modeBox.addItem("Chaotic Rev", 16);
    modeBox.addItem("Chromatic", 12);
    modeBox.addItem("Colour", 19);
    modeBox.addItem("Crystal", 9);
    modeBox.addItem("Digital", 1);
    modeBox.addItem("Forward Rev", 17);
    modeBox.addItem("Ghost", 5);
    modeBox.addItem("Grain", 21);
    modeBox.addItem("HiFi", 4);
    modeBox.addItem("LoFi", 8);
    modeBox.addItem("Phaser", 10);
    modeBox.addItem("Pitch", 6);
    modeBox.addItem("Pitch Lush", 11);
    modeBox.addItem("Pitch Rev", 7);
    modeBox.addItem("Pure Reverse", 15);
    modeBox.addItem("Tape I", 2);
    modeBox.addItem("Tape II", 18);
    modeBox.addItem("Twin Tape", 20);
    modeBox.setJustificationType(juce::Justification::centred);
    modeBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff19151f));
    modeBox.setColour(juce::ComboBox::textColourId, knobValueColour);
    modeBox.setColour(juce::ComboBox::outlineColourId, knobTrackColour);
    modeBox.setColour(juce::ComboBox::arrowColourId, knobIndicatorColour);
    modeBox.setScrollWheelEnabled(true);
    modeBox.setName("Mode");
    modeBox.setTooltip("Choose the delay mode and its feedback-loop character");
    content.addAndMakeVisible(modeBox);
    modeLabel.setText("MODE", juce::dontSendNotification);
    modeLabel.setJustificationType(juce::Justification::centred);
    modeLabel.setColour(juce::Label::textColourId, knobLabelColour);
    modeLabel.setFont(juce::Font(juce::FontOptions(10.5f, juce::Font::bold)));
    content.addAndMakeVisible(modeLabel);

    distortionTypeBox.addItem("Amp", 7);
    distortionTypeBox.addItem("Asym", 9);
    distortionTypeBox.addItem("Bumpy", 8);
    distortionTypeBox.addItem("Comb", 12);
    distortionTypeBox.addItem("Crackle", 5);
    distortionTypeBox.addItem("Fresh", 15);
    distortionTypeBox.addItem("Fuzz", 17);
    distortionTypeBox.addItem("Hard Clip", 16);
    distortionTypeBox.addItem("Lightning", 6);
    distortionTypeBox.addItem("Pixelated", 11);
    distortionTypeBox.addItem("Razor", 10);
    distortionTypeBox.addItem("Rectify", 4);
    distortionTypeBox.addItem("Samurai", 14);
    distortionTypeBox.addItem("Saturation", 2);
    distortionTypeBox.addItem("Soft Clip", 1);
    distortionTypeBox.addItem("Stressed", 13);
    distortionTypeBox.addItem("Tube", 3);
    distortionTypeBox.setJustificationType(juce::Justification::centred);
    distortionTypeBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff19151f));
    distortionTypeBox.setColour(juce::ComboBox::textColourId, knobValueColour);
    distortionTypeBox.setColour(juce::ComboBox::outlineColourId, knobTrackColour);
    distortionTypeBox.setColour(juce::ComboBox::arrowColourId, knobIndicatorColour);
    distortionTypeBox.setScrollWheelEnabled(true);
    distortionTypeBox.setName("Distortion Type");
    distortionTypeBox.setTooltip("Choose the input distortion used by the DIST amount control");
    content.addAndMakeVisible(distortionTypeBox);
    distortionTypeLabel.setText("DIST TYPE", juce::dontSendNotification);
    distortionTypeLabel.setJustificationType(juce::Justification::centred);
    distortionTypeLabel.setColour(juce::Label::textColourId, knobLabelColour);
    distortionTypeLabel.setFont(juce::Font(juce::FontOptions(10.5f, juce::Font::bold)));
    content.addAndMakeVisible(distortionTypeLabel);

    freezeButton.setName("Freeze");
    freezeButton.getProperties().set("snowflakeIcon", true);
    freezeButton.getProperties().set(
        "activeColour", juce::Colour(0xff73c7cb).toString());
    freezeButton.setClickingTogglesState(false);
    freezeButton.setTooltip("Hold or toggle Freeze to sustain the captured delay without decay");
    content.addAndMakeVisible(freezeButton);

    freezeModeButton.getProperties().set("statusLedOnly", true);
    freezeModeButton.getProperties().set("ledDiameter", 8.0f);
    freezeModeButton.getProperties().set(
        "activeColour", controlAccentColour.toString());
    freezeModeButton.setClickingTogglesState(true);
    freezeModeButton.setName("Freeze Mode");
    freezeModeButton.setTooltip("Choose whether Freeze is momentary or toggled");
    content.addAndMakeVisible(freezeModeButton);

    freezeModeLabel.setJustificationType(juce::Justification::centredLeft);
    freezeModeLabel.setColour(juce::Label::textColourId, knobLabelColour);
    freezeModeLabel.setFont(juce::Font(juce::FontOptions(8.6f, juce::Font::bold)));
    content.addAndMakeVisible(freezeModeLabel);

    if (auto* parameter = processor.parameters.getParameter("freeze"))
        freezeAttachment = std::make_unique<juce::ParameterAttachment>(
            *parameter,
            [this](float value)
            {
                const juce::ScopedValueSetter<bool> guard(
                    updatingFreezeControls, true);
                freezeButton.setToggleState(value >= 0.5f,
                                            juce::dontSendNotification);
            });

    if (auto* parameter = processor.parameters.getParameter("freezeToggle"))
        freezeModeAttachment = std::make_unique<juce::ParameterAttachment>(
            *parameter,
            [this](float value)
            {
                const juce::ScopedValueSetter<bool> guard(
                    updatingFreezeControls, true);
                freezeModeButton.setToggleState(value >= 0.5f,
                                                juce::dontSendNotification);
                updateFreezeModeDisplay();
            });

    freezeButton.onStateChange = [this]
    {
        if (updatingFreezeControls || freezeModeButton.getToggleState())
            return;

        setFreezeActiveFromUi(freezeButton.isDown());
    };
    freezeButton.onClick = [this]
    {
        if (freezeModeButton.getToggleState())
        {
            const bool active = ! freezeButton.getToggleState();
            setFreezeActiveFromUi(active);
            information.setText(active
                ? "FREEZE: ON | New input is excluded while the captured delay cycle repeats without decay.\nClick FREEZE again to release it and return smoothly to the live delay tail."
                : "FREEZE: OFF | New input has returned and FDBK again controls the repeat decay.",
                juce::dontSendNotification);
        }
        else
        {
            information.setText(
                "FREEZE: RELEASED | New input and the stored FDBK value have returned smoothly.\nHold FREEZE whenever you want to sustain the current delay without admitting new audio.",
                juce::dontSendNotification);
        }
    };
    freezeModeButton.onClick = [this]
    {
        if (freezeModeAttachment != nullptr)
            freezeModeAttachment->setValueAsCompleteGesture(
                freezeModeButton.getToggleState() ? 1.0f : 0.0f);

        setFreezeActiveFromUi(false);
        updateFreezeModeDisplay();
        information.setText(freezeModeButton.getToggleState()
            ? "FREEZE MODE: TOGGLE | Each FREEZE click alternates between captured and released states."
            : "FREEZE MODE: MOMENTARY | The delay remains frozen only while FREEZE is held.",
            juce::dontSendNotification);
    };

    modeDials.push_back(addDial("drive", "DIST", "Sets input distortion before the delay. Recirculating repeats are not distorted again."));
    modeDials.push_back(addDial("age", "AGE", "Changes the selected mode's age or character. The description follows the active mode."));
    modeDials.push_back(addDial("wow", "WOW", "Slow delay-time movement and pitch drift. Available in every delay mode."));
    modeDials.push_back(addDial("flutter", "FLUTTER", "Faster, finer delay-time movement with slight stereo variation. Available in every delay mode."));

    twinTapeCharacterPageButton.getProperties().set("statusLedOnly", true);
    twinTapeCharacterPageButton.getProperties().set("ledDiameter", 8.0f);
    twinTapeCharacterPageButton.getProperties().set(
        "activeColour", linkLedColour.toString());
    twinTapeCharacterPageButton.setClickingTogglesState(true);
    twinTapeCharacterPageButton.setToggleState(true,
                                                juce::dontSendNotification);
    twinTapeCharacterPageButton.setName("Twin Tape Character Page");
    twinTapeCharacterPageButton.setTooltip(
        "Showing the independent Tape I/II Age and Shine controls; click for DIST, shared AGE, WOW and FLUTTER");
    twinTapeCharacterPageButton.onClick = [this]
    {
        updateModeControlState();
        const bool headPage = twinTapeCharacterPageButton.getToggleState();
        information.setText(headPage
            ? "TWIN TAPE HEAD PAGE | AGE and SHINE are independent for Tape I and Tape II.\nClick the amber page LED for shared DIST, AGE, WOW and FLUTTER."
            : "TWIN TAPE CHARACTER PAGE | DIST, shared AGE, WOW and FLUTTER colour both tape heads.\nClick the page LED again for the independent AGE I/SHINE I and AGE II/SHINE II controls.",
            juce::dontSendNotification);
    };
    content.addAndMakeVisible(twinTapeCharacterPageButton);

    pitchShiftDial = addDial("pitchShift", "SHIFT", "Pitch, Pitch Rev and Pitch Lush transposition from -24 to +24 semitones. Zero preserves the original pitch.");
    pitchShiftDial->slider.setCentreDetent(0.0, 0.15);
    pitchShiftDial->slider.setDoubleClickReturnValue(true, 0.0);
    pitchDetuneDial = addDial("pitchDetune", "DETUNE", "Stereo pitch offset: positive raises Right and lowers Left by half; negative reverses them.");
    pitchDetuneDial->slider.setCentreDetent(0.0, 0.03);
    pitchDetuneDial->slider.setDoubleClickReturnValue(true, 0.0);
    for (size_t tap = 0; tap < multiTapCount; ++tap)
    {
        const auto tapNumber = juce::String(static_cast<int>(tap) + 1);
        const auto timeHelp = "Tap " + tapNumber
            + " delay position. Global Sync changes all taps without overwriting stored millisecond or note values.";
        tapTimeDials[tap] = addDial(
            tapTimeParameterIds[tap],
            ("TAP " + tapNumber + " TIME").toRawUTF8(),
            timeHelp.toRawUTF8());
        tapTimeDials[tap]->label.dragHandleOnly = true;
        tapTimeDials[tap]->slider.setTextBoxStyle(
            juce::Slider::NoTextBox, false, 0, 0);
        tapTimeDials[tap]->slider.onValueChange = [this, tap]
        {
            auto* dial = tapTimeDials[tap];
            information.setText(
                "TAP " + juce::String(static_cast<int>(tap) + 1)
                    + " TIME: "
                    + dial->slider.getTextFromValue(dial->slider.getValue())
                    + "\n" + dial->help,
                juce::dontSendNotification);
        };

        const auto panHelp = "Tap " + tapNumber
            + " stereo position from fully Left through centre to fully Right.";
        tapPanDials[tap] = addDial(
            tapPanParameterIds[tap],
            ("TAP " + tapNumber + " PAN").toRawUTF8(),
            panHelp.toRawUTF8());
        tapPanDials[tap]->label.dragHandleOnly = true;
        tapPanDials[tap]->slider.setTextBoxStyle(
            juce::Slider::NoTextBox, false, 0, 0);
        tapPanDials[tap]->slider.setCentreDetent(0.0, 2.0);
        tapPanDials[tap]->slider.setDoubleClickReturnValue(true, 0.0);
        tapPanDials[tap]->slider.onValueChange = [this, tap]
        {
            auto* dial = tapPanDials[tap];
            information.setText(
                "TAP " + juce::String(static_cast<int>(tap) + 1)
                    + " PAN: "
                    + dial->slider.getTextFromValue(dial->slider.getValue())
                    + "\n" + dial->help,
                juce::dontSendNotification);
        };

        const auto levelHelp = "Tap " + tapNumber
            + " output level. Fixed RMS normalisation controls level when several taps are enabled.";
        tapLevelDials[tap] = addDial(
            tapLevelParameterIds[tap],
            ("TAP " + tapNumber + " VOL").toRawUTF8(),
            levelHelp.toRawUTF8());
        tapLevelDials[tap]->label.dragHandleOnly = true;
        tapLevelDials[tap]->slider.setTextBoxStyle(
            juce::Slider::NoTextBox, false, 0, 0);
        tapLevelDials[tap]->slider.onValueChange = [this, tap]
        {
            auto* dial = tapLevelDials[tap];
            information.setText(
                "TAP " + juce::String(static_cast<int>(tap) + 1)
                    + " VOL: "
                    + dial->slider.getTextFromValue(dial->slider.getValue())
                    + "\n" + dial->help,
                juce::dontSendNotification);
        };
    }
    themeDial = addDial("", "THEME", "Turn clockwise for full mode colour or left for charcoal. The colour strip blends in at maximum.", false);
    themeDial->label.setJustificationType(juce::Justification::centred);
    themeDial->slider.setRange(0.0, 100.0, 0.1);
    themeDial->slider.setValue(storedSettings.themePercent, juce::dontSendNotification);
    themeDial->slider.setDoubleClickReturnValue(true, 100.0);
    themeDial->slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    themeDial->slider.onValueChange = [this]
    {
        information.setText("THEME\nTurn clockwise for full mode colour; at maximum the colour strip blends into the surround.",
            juce::dontSendNotification);
        repaint();
    };

    browserMixDial = addDial(
        "mix", "MIX",
        "Browser copy of the main Mix control. It changes the same Mix level without becoming a sequencer target.");
    browserMixDial->parameterId.clear();
    browserMixDial->label.parameterIdProvider = {};
    browserMixDial->label.setInterceptsMouseClicks(false, false);
    browserMixDial->label.setTooltip(
        "Browser copy of the main Mix control");
    browserMixDial->slider.setTextBoxStyle(
        juce::Slider::NoTextBox, false, 0, 0);
    browserMixDial->slider.setCentreDetent(50.0, 2.0);
    browserMixDial->slider.setDoubleClickReturnValue(true, 50.0);
    browserMixDial->slider.setTooltip(
        "Adjust Mix while the preset browser is open");
    browserMixDial->slider.setVisible(false);
    browserMixDial->label.setVisible(false);

    const auto trackLockedMix = [this](Dial& dial)
    {
        const auto existingCallback = dial.slider.onValueChange;
        dial.slider.onValueChange = [this, &dial, existingCallback]
        {
            if (existingCallback != nullptr)
                existingCallback();
            if (mixLockButton.getToggleState() && ! restoringLockedMix)
                lockedMixValue = dial.slider.getValue();
        };
    };
    trackLockedMix(*mixDial);
    trackLockedMix(*browserMixDial);

    mixLockButton.setClickingTogglesState(true);
    mixLockButton.setName("Mix Lock");
    mixLockButton.getProperties().set("mixLockIcon", true);
    mixLockButton.getProperties().set(
        "activeColour", knobIndicatorColour.toString());
    mixLockButton.setTooltip(
        "Lock the current Mix level so loading presets cannot change it");
    mixLockButton.onClick = [this]
    {
        if (mixLockButton.getToggleState() && mixDial != nullptr)
            lockedMixValue = mixDial->slider.getValue();
        information.setText(
            mixLockButton.getToggleState()
                ? "MIX LOCK: ON | Presets retain the current Mix level.\nAdjust either visible Mix knob to choose a new locked level."
                : "MIX LOCK: OFF | Presets can load their saved Mix level again.\nThe current Mix value is left unchanged.",
            juce::dontSendNotification);
    };
    content.addAndMakeVisible(mixLockButton);

    if (auto* modeParameter = processor.parameters.getParameter("mode"))
        modeAttachment = std::make_unique<juce::ParameterAttachment>(
            *modeParameter,
            [this](float value)
            {
                const int modeIndex = juce::jlimit(
                    0, delayModeCount - 1,
                    static_cast<int>(std::lround(value)));
                modeBox.setSelectedId(modeIndex + 1, juce::dontSendNotification);
                updateModeControlState();
            });

    modeBox.onChange = [this]
    {
        if (modeAttachment != nullptr)
            modeAttachment->setValueAsCompleteGesture(
                static_cast<float>(juce::jlimit(
                    1, delayModeCount, modeBox.getSelectedId()) - 1));
        updateModeControlState();
    };

    if (auto* distortionTypeParameter = processor.parameters.getParameter("distortionType"))
        distortionTypeAttachment = std::make_unique<juce::ParameterAttachment>(
            *distortionTypeParameter,
            [this](float value)
            {
                const int lastIndex = juce::jmax(0, distortionTypeBox.getNumItems() - 1);
                const int typeIndex = juce::jlimit(
                    0, lastIndex, static_cast<int>(std::lround(value)));
                distortionTypeBox.setSelectedId(typeIndex + 1,
                                                juce::dontSendNotification);
            });

    distortionTypeBox.onChange = [this]
    {
        if (distortionTypeAttachment != nullptr)
            distortionTypeAttachment->setValueAsCompleteGesture(static_cast<float>(
                juce::jlimit(1, juce::jmax(1, distortionTypeBox.getNumItems()),
                             distortionTypeBox.getSelectedId()) - 1));
        const int typeIndex = juce::jlimit(
            0, static_cast<int>(distortionSummaries.size()) - 1,
            distortionTypeBox.getSelectedId() - 1);
        information.setText(
            "DIST TYPE: " + distortionTypeBox.getText()
                + " | " + distortionSummaries[static_cast<size_t>(typeIndex)]
                + "\n"
                + distortionDetails[static_cast<size_t>(typeIndex)],
            juce::dontSendNotification);
    };

    const auto configureSyncButton = [this](juce::ToggleButton& button)
    {
        button.getProperties().set("statusLedOnly", true);
        button.getProperties().set("ledDiameter", 8.0f);
        button.getProperties().set("activeColour", tempoSyncLedColour.toString());
        button.setClickingTogglesState(true);
        content.addAndMakeVisible(button);
    };
    configureSyncButton(syncLeftButton);
    configureSyncButton(syncRightButton);
    configureSyncButton(multiTapSyncButton);
    syncLeftButton.setName("Left Tempo Sync");
    syncRightButton.setName("Right Tempo Sync");
    multiTapSyncButton.setName("Multi-Tap Tempo Sync");
    syncLeftButton.setTooltip("Switch the Left Time knob between milliseconds and note divisions");
    syncRightButton.setTooltip("Switch the Right Time knob between milliseconds and note divisions");
    multiTapSyncButton.setTooltip("Switch all four Multi-Tap Time knobs between milliseconds and note divisions");

    if (auto* parameter = processor.parameters.getParameter("delayStyle"))
        delayStyleAttachment = std::make_unique<juce::ParameterAttachment>(
            *parameter, [](float) {});

    multiTapButton.getProperties().set("statusLedOnly", true);
    multiTapButton.getProperties().set("ledDiameter", 8.0f);
    multiTapButton.getProperties().set(
        "activeColour", knobIndicatorColour.toString());
    multiTapButton.setClickingTogglesState(true);
    multiTapButton.setName("Multi-Tap");
    multiTapButton.setTooltip("Switch the Timing panel between Stereo and four-tap operation");
    content.addAndMakeVisible(multiTapButton);
    if (auto* parameter = processor.parameters.getParameter("multiTap"))
        multiTapAttachment = std::make_unique<juce::ParameterAttachment>(
            *parameter,
            [this](float value)
            {
                multiTapButton.setToggleState(value >= 0.5f,
                                              juce::dontSendNotification);
                updateTimeControlState();
            });
    if (auto* parameter = processor.parameters.getParameter("multiTapSync"))
        multiTapSyncAttachment = std::make_unique<juce::ParameterAttachment>(
            *parameter,
            [this](float value)
            {
                multiTapSyncButton.setToggleState(value >= 0.5f,
                                                  juce::dontSendNotification);
                updateTimeControlState();
            });

    multiTapButton.onClick = [this]
    {
        if (multiTapAttachment != nullptr)
            multiTapAttachment->setValueAsCompleteGesture(
                multiTapButton.getToggleState() ? 1.0f : 0.0f);
        synchroniseDelayStyleFromTimingControls();
        updateTimeControlState();
        information.setText(multiTapButton.getToggleState()
            ? "TIMING: MULTI-TAP | Four independent Time, Pan, Vol and On rows are active.\nGlobal Sync changes all four Time knobs between milliseconds and note divisions."
            : "TIMING: STEREO | Independent Left and Right timing, Link and Ping-Pong are active.\nThe complete Multi-Tap setup remains stored for the next time it is opened.",
            juce::dontSendNotification);
    };
    multiTapSyncButton.onClick = [this]
    {
        if (multiTapSyncAttachment != nullptr)
            multiTapSyncAttachment->setValueAsCompleteGesture(
                multiTapSyncButton.getToggleState() ? 1.0f : 0.0f);
        updateTimeControlState();
        information.setText(multiTapSyncButton.getToggleState()
            ? "MULTI-TAP SYNC: ON | All four Time knobs now select musical note divisions."
            : "MULTI-TAP SYNC: OFF | All four Time knobs now use their stored millisecond values.",
            juce::dontSendNotification);
    };

    for (size_t tap = 0; tap < multiTapCount; ++tap)
    {
        auto& button = tapEnableButtons[tap];
        button.setButtonText(juce::String(static_cast<int>(tap) + 1));
        button.getProperties().set("statusLedOnly", true);
        button.getProperties().set("ledDiameter", 12.0f);
        button.getProperties().set(
            "activeColour", controlAccentColour.toString());
        button.setClickingTogglesState(true);
        button.setName("Tap " + juce::String(static_cast<int>(tap) + 1));
        button.setTooltip("Enable or mute Multi-Tap delay "
            + juce::String(static_cast<int>(tap) + 1));
        content.addAndMakeVisible(button);
        if (auto* parameter = processor.parameters.getParameter(
                tapEnabledParameterIds[tap]))
        {
            tapEnableAttachments[tap] =
                std::make_unique<juce::ParameterAttachment>(
                    *parameter,
                    [this, tap](float value)
                    {
                        tapEnableButtons[tap].setToggleState(
                            value >= 0.5f, juce::dontSendNotification);
                    });
        }
        button.onClick = [this, tap]
        {
            if (tapEnableAttachments[tap] != nullptr)
                tapEnableAttachments[tap]->setValueAsCompleteGesture(
                    tapEnableButtons[tap].getToggleState() ? 1.0f : 0.0f);
            information.setText(
                "TAP " + juce::String(static_cast<int>(tap) + 1)
                    + (tapEnableButtons[tap].getToggleState()
                        ? ": ON | This tap now contributes to the Multi-Tap pattern."
                        : ": OFF | This tap is muted; its Time, Pan and Vol settings remain stored."),
                juce::dontSendNotification);
        };
    }

    if (auto* parameter = processor.parameters.getParameter("syncL"))
        syncLeftAttachment = std::make_unique<juce::ParameterAttachment>(
            *parameter,
            [this](float value)
            {
                syncLeftButton.setToggleState(value >= 0.5f,
                                              juce::dontSendNotification);
                updateTimeControlState();
            });
    if (auto* parameter = processor.parameters.getParameter("syncR"))
        syncRightAttachment = std::make_unique<juce::ParameterAttachment>(
            *parameter,
            [this](float value)
            {
                syncRightButton.setToggleState(value >= 0.5f,
                                               juce::dontSendNotification);
                updateTimeControlState();
            });

    syncLeftButton.onClick = [this]
    {
        if (syncLeftAttachment != nullptr)
            syncLeftAttachment->setValueAsCompleteGesture(
                syncLeftButton.getToggleState() ? 1.0f : 0.0f);
        updateTimeControlState();
        information.setText(syncLeftButton.getToggleState()
            ? "LEFT TEMPO SYNC: ON | Left Time now selects musical note divisions."
            : "LEFT TEMPO SYNC: OFF | Left Time now uses milliseconds.",
            juce::dontSendNotification);
    };
    syncRightButton.onClick = [this]
    {
        if (syncRightAttachment != nullptr)
            syncRightAttachment->setValueAsCompleteGesture(
                syncRightButton.getToggleState() ? 1.0f : 0.0f);
        updateTimeControlState();
        information.setText(syncRightButton.getToggleState()
            ? "RIGHT TEMPO SYNC: ON | Right Time now selects musical note divisions."
            : "RIGHT TEMPO SYNC: OFF | Right Time now uses milliseconds.",
            juce::dontSendNotification);
    };

    pingPongButton.setColour(juce::ToggleButton::textColourId,
                            knobLabelColour);
    pingPongButton.setName("Ping-Pong");
    pingPongButton.setTooltip("Alternate successive repeats between the Left and Right channels");
    content.addAndMakeVisible(pingPongButton);
    if (auto* parameter = processor.parameters.getParameter("pingPong"))
        pingPongAttachment = std::make_unique<juce::ParameterAttachment>(
            *parameter,
            [this](float value)
            {
                pingPongButton.setToggleState(value >= 0.5f,
                                              juce::dontSendNotification);
            });
    pingPongButton.onClick = [this]
    {
        if (pingPongAttachment != nullptr)
            pingPongAttachment->setValueAsCompleteGesture(
                pingPongButton.getToggleState() ? 1.0f : 0.0f);
        synchroniseDelayStyleFromTimingControls();
        information.setText(pingPongButton.getToggleState()
            ? "PING-PONG: ON | The summed input starts on Left and repeats alternate between channels."
            : "PING-PONG: OFF | Input and Cross return to normal stereo routing.",
            juce::dontSendNotification);
    };

    linkButton.setColour(juce::ToggleButton::textColourId, knobLabelColour);
    linkButton.getProperties().set("activeColour", linkLedColour.toString());
    linkButton.setName("Link L/R");
    linkButton.setTooltip("Link Right Time to Left while preserving the stored Right setting");
    content.addAndMakeVisible(linkButton);
    if (auto* linkParameter = processor.parameters.getParameter("link"))
        linkAttachment = std::make_unique<juce::ParameterAttachment>(
            *linkParameter,
            [this](float value)
            {
                linkButton.setToggleState(value >= 0.5f, juce::dontSendNotification);
                updateTimeControlState();
            });
    linkButton.onClick = [this]
    {
        if (linkAttachment != nullptr)
            linkAttachment->setValueAsCompleteGesture(linkButton.getToggleState() ? 1.0f : 0.0f);
        synchroniseDelayStyleFromTimingControls();
        updateTimeControlState();
        information.setText(linkButton.getToggleState()
            ? "TIME LINK: ON | Right follows Left without overwriting its stored settings."
            : "TIME LINK: OFF | The previous Right Time and Right Note have been restored.",
            juce::dontSendNotification);
    };

    bypassButton.setName("Power");
    bypassButton.setTooltip("Switch Afterglow processing on or off");
    bypassButton.getProperties().set("ledOnly", true);
    bypassButton.getProperties().set("ledDiameter", 24.0f);
    bypassButton.getProperties().set(
        "activeColour", powerLedColour.toString());
    content.addAndMakeVisible(bypassButton);
    if (auto* bypassParameter = processor.parameters.getParameter("bypass"))
        bypassAttachment = std::make_unique<juce::ParameterAttachment>(
            *bypassParameter,
            [this](float value)
            {
                bypassButton.setToggleState(value < 0.5f, juce::dontSendNotification);
            });
    bypassButton.onClick = [this]
    {
        if (bypassAttachment != nullptr)
            bypassAttachment->setValueAsCompleteGesture(
                bypassButton.getToggleState() ? 0.0f : 1.0f);
        information.setText(bypassButton.getToggleState()
            ? "POWER: ON | Afterglow processing is active."
            : "POWER: OFF | Dry input passes through while the delay engine keeps running.",
            juce::dontSendNotification);
    };

    presetBrowserButton.setName("Preset Browser");
    presetBrowserButton.setTooltip(
        "Open or close the preset browser");
    presetBrowserButton.getProperties().set("listIcon", true);
    presetBrowserButton.getProperties().set(
        "activeColour", presetAccentColour.toString());
    presetBrowserButton.setClickingTogglesState(true);
    content.addAndMakeVisible(presetBrowserButton);
    presetBrowserButton.onClick = [this]
    {
        setPresetBrowserOpen(presetBrowserButton.getToggleState());
    };

    const auto configurePresetStepButton = [this](
        juce::ToggleButton& button, int direction,
        const juce::String& description)
    {
        button.setName(description);
        button.setTooltip(description);
        button.getProperties().set("presetArrowDirection", direction);
        button.setClickingTogglesState(false);
        content.addAndMakeVisible(button);
    };
    configurePresetStepButton(previousPresetButton, -1, "Previous preset");
    configurePresetStepButton(nextPresetButton, 1, "Next preset");
    previousPresetButton.onClick = [this] { stepPreset(-1); };
    nextPresetButton.onClick = [this] { stepPreset(1); };

    currentPresetLabel.setJustificationType(juce::Justification::centred);
    currentPresetLabel.setColour(juce::Label::backgroundColourId,
                                 juce::Colour(0xff14181d));
    currentPresetLabel.setColour(juce::Label::outlineColourId,
                                 knobTrackColour);
    currentPresetLabel.setColour(juce::Label::textColourId,
                                 knobValueColour);
    currentPresetLabel.setFont(juce::Font(
        juce::FontOptions(10.0f, juce::Font::bold)));
    currentPresetLabel.setMinimumHorizontalScale(0.68f);
    currentPresetLabel.setInterceptsMouseClicks(true, false);
    currentPresetLabel.setName("Current Preset");
    currentPresetLabel.onStep = [this](int direction)
    {
        stepPreset(direction);
    };
    currentPresetLabel.onShowMenu = [this]
    {
        showPresetQuickMenu();
    };
    content.addAndMakeVisible(currentPresetLabel);

    const auto configureBrowserHeading = [this](juce::Label& label,
                                                 const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centredLeft);
        label.setColour(juce::Label::textColourId, knobLabelColour);
        label.setFont(juce::Font(juce::FontOptions(9.5f, juce::Font::bold)));
        content.addChildComponent(label);
    };
    configureBrowserHeading(presetFolderLabel, "FOLDERS");
    configureBrowserHeading(presetListLabel, "PRESETS");

    presetFolderTree.setRootItemVisible(false);
    presetFolderTree.setDefaultOpenness(false);
    presetFolderTree.setIndentSize(14);
    presetFolderTree.setColour(juce::TreeView::backgroundColourId,
                               juce::Colour(0xff11151a));
    presetFolderTree.setColour(juce::TreeView::linesColourId,
                               knobTrackColour);
    presetFolderTree.setColour(
        juce::TreeView::selectedItemBackgroundColourId,
        juce::Colours::transparentBlack);
    content.addChildComponent(presetFolderTree);

    presetList.setRowHeight(22);
    presetList.setColour(juce::ListBox::backgroundColourId,
                         juce::Colour(0xff11151a));
    presetList.setColour(juce::ListBox::outlineColourId,
                         knobTrackColour);
    presetList.setOutlineThickness(1);
    presetList.onHorizontalStep = [this](int direction)
    {
        return handlePresetCursorKey(direction);
    };
    content.addChildComponent(presetList);

    const auto configureBrowserActionButton = [this](
        juce::ToggleButton& button, const juce::String& name,
        const juce::Identifier& iconProperty)
    {
        button.setName(name);
        button.setTooltip(name);
        button.getProperties().set(iconProperty, true);
        button.getProperties().set(
            "activeColour", presetAccentColour.toString());
        button.setClickingTogglesState(false);
        content.addChildComponent(button);
    };
    configureBrowserActionButton(
        presetSaveButton, "Save preset", juce::Identifier("saveIcon"));
    configureBrowserActionButton(
        presetSaveAsButton, "Save preset as", juce::Identifier("saveAsIcon"));
    configureBrowserActionButton(
        presetOpenFolderButton, "Open current preset folder",
        juce::Identifier("folderIcon"));
    presetSaveButton.onClick = [this] { saveCurrentPreset(); };
    presetSaveAsButton.onClick = [this] { showPresetSaveAs(); };
    presetOpenFolderButton.onClick = [this]
    {
        const auto root = AfterglowAudioProcessor::getPresetsDirectory();
        auto folder = selectedPresetFolder;
        if (! folder.isDirectory()
            || (folder != root && ! folder.isAChildOf(root)))
            folder = root;

        if (! folder.startAsProcess())
            information.setText(
                "PRESET FOLDER | The current preset folder could not be opened.",
                juce::dontSendNotification);
    };

    information.setText("AFTERGLOW 1.89.1 | Drag a parameter label onto the sequencer target list\nSEQ opens or closes the modulation panel without disturbing the delay layout.", juce::dontSendNotification);
    information.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    information.setJustificationType(juce::Justification::centredLeft);
    information.setFont(juce::FontOptions(11.5f));
    information.setMinimumHorizontalScale(0.82f);
    content.addAndMakeVisible(information);

    sequencerPanel = std::make_unique<SequencerPanel>(
        processor, lookAndFeel,
        [this]
        {
            const float amount = themeDial != nullptr
                ? juce::jlimit(0.0f, 1.0f,
                    static_cast<float>(themeDial->slider.getValue() * 0.01))
                : 1.0f;
            return juce::Colour(0xff24292f).interpolatedWith(
                modeColourForId(modeBox.getSelectedId()), amount);
        },
        [this](const juce::String& helpText)
        {
            information.setText(helpText, juce::dontSendNotification);
        });
    content.addAndMakeVisible(*sequencerPanel);
    sequencerPanel->setAnimate(storedSettings.sequencerAnimate);
    sequencerPanel->onAnimateChanged = [this]
    {
        updateAnimatedParameterDisplays();
        saveInterfaceSettings();
    };
    updateSequencerTargetHighlights();
    sequencerOpen = storedSettings.sequencerOpen;
    sequencerPanel->setVisible(sequencerOpen);

    echoOffsetLabel.setText("ECHO OFFSET", juce::dontSendNotification);
    echoOffsetLabel.setJustificationType(juce::Justification::centred);
    echoOffsetLabel.setColour(juce::Label::textColourId, knobLabelColour);
    echoOffsetLabel.setFont(juce::Font(
        juce::FontOptions(8.5f, juce::Font::bold)));
    content.addAndMakeVisible(echoOffsetLabel);
    echoOffsetSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    echoOffsetSlider.setTextBoxStyle(
        juce::Slider::NoTextBox, false, 0, 0);
    echoOffsetSlider.setRange(0.0, 100.0, 0.1);
    echoOffsetSlider.setName("Echo Offset");
    echoOffsetSlider.setDoubleClickReturnValue(true, 0.0);
    echoOffsetSlider.setColour(juce::Slider::rotarySliderFillColourId,
                               knobIndicatorColour);
    echoOffsetSlider.setTooltip(
        "Offset reverse and chaos echoes relative to the main delay time");
    echoOffsetSlider.textFromValueFunction = [](double value)
    {
        return juce::String(value, 1) + "%";
    };
    echoOffsetAttachment = std::make_unique<SliderAttachment>(
        processor.parameters, "echoOffset", echoOffsetSlider);
    echoOffsetSlider.onSavedStateReset = [this]
    {
        const auto saved = processor.getSavedParameterValue("echoOffset");
        if (! saved.has_value())
            return false;
        echoOffsetSlider.setValue(*saved, juce::sendNotificationSync);
        return true;
    };
    content.addAndMakeVisible(echoOffsetSlider);

    randomButton.setClickingTogglesState(false);
    randomButton.setName("Randomise");
    randomButton.getProperties().set("randomIcon", true);
    randomButton.getProperties().set("activeColour",
                                     knobIndicatorColour.toString());
    randomButton.setTooltip(
        "Randomise the delay controls, Mode, Dist Type and Echo Offset; "
        "leave the complete sequencer unchanged");
    randomButton.onClick = [this] { randomiseDelayParameters(); };
    content.addAndMakeVisible(randomButton);

    sequencerButton.setClickingTogglesState(true);
    sequencerButton.setName("Sequencer");
    sequencerButton.setToggleState(sequencerOpen, juce::dontSendNotification);
    sequencerButton.getProperties().set("standardTextButton", true);
    sequencerButton.getProperties().set("activeColour",
                                        controlAccentColour.toString());
    sequencerButton.setTooltip("Show or hide the sequencer");
    sequencerButton.onClick = [this]
    {
        setSequencerOpen(sequencerButton.getToggleState());
    };
    content.addAndMakeVisible(sequencerButton);

    if (modeAttachment != nullptr)
        modeAttachment->sendInitialUpdate();
    else
        modeBox.setSelectedId(1, juce::dontSendNotification);
    if (distortionTypeAttachment != nullptr)
        distortionTypeAttachment->sendInitialUpdate();
    else
        distortionTypeBox.setSelectedId(1, juce::dontSendNotification);
    if (freezeModeAttachment != nullptr)
        freezeModeAttachment->sendInitialUpdate();
    if (freezeAttachment != nullptr)
        freezeAttachment->sendInitialUpdate();
    if (linkAttachment != nullptr)
        linkAttachment->sendInitialUpdate();
    if (syncLeftAttachment != nullptr)
        syncLeftAttachment->sendInitialUpdate();
    if (syncRightAttachment != nullptr)
        syncRightAttachment->sendInitialUpdate();
    if (pingPongAttachment != nullptr)
        pingPongAttachment->sendInitialUpdate();
    if (multiTapAttachment != nullptr)
        multiTapAttachment->sendInitialUpdate();
    if (multiTapSyncAttachment != nullptr)
        multiTapSyncAttachment->sendInitialUpdate();
    for (auto& attachment : tapEnableAttachments)
        if (attachment != nullptr)
            attachment->sendInitialUpdate();
    if (bypassAttachment != nullptr)
        bypassAttachment->sendInitialUpdate();
    updateTimeControlState();
    updateModeControlState();
    updatePresetNavigationDisplay();
    setResizable(true, true);
    setResizeLimits(695, sequencerOpen ? 414 : 207,
                    2778, sequencerOpen ? 1656 : 828);
    if (auto* editorConstrainer = getConstrainer())
        editorConstrainer->setFixedAspectRatio(
            static_cast<double>(designWidth)
                / static_cast<double>(activeDesignHeight()));
    setSize(juce::roundToInt(static_cast<float>(designWidth)
                             * static_cast<float>(storedSettings.scalePercent) * 0.01f),
            juce::roundToInt(static_cast<float>(activeDesignHeight())
                             * static_cast<float>(storedSettings.scalePercent) * 0.01f));
    observedEditorWidth = getWidth();
    observedThemeValue = themeDial != nullptr ? themeDial->slider.getValue() : 100.0;
    saveInterfaceSettings();
    startTimerHz(30);
}

AfterglowAudioProcessorEditor::~AfterglowAudioProcessorEditor()
{
    stopTimer();
    saveInterfaceSettings();
    if (presetFolderNameWindow != nullptr)
    {
        presetFolderNameWindow->exitModalState(0);
        presetFolderNameWindow->setLookAndFeel(nullptr);
        presetFolderNameWindow.reset();
    }
    content.removeMouseListener(this);
    presetFolderTree.setRootItem(nullptr);
    presetFolderRoot.reset();
    setLookAndFeel(nullptr);
}

bool AfterglowAudioProcessorEditor::keyPressed(const juce::KeyPress& key)
{
    if (key.getKeyCode() == juce::KeyPress::leftKey)
        return handlePresetCursorKey(-1);
    if (key.getKeyCode() == juce::KeyPress::rightKey)
        return handlePresetCursorKey(1);

    return juce::AudioProcessorEditor::keyPressed(key);
}

void AfterglowAudioProcessorEditor::mouseDown(
    const juce::MouseEvent& event)
{
    if (! presetBrowserOpen || ! event.mods.isPopupMenu())
        return;

    bool isInsidePresetList = false;
    for (auto* component = event.eventComponent; component != nullptr;
         component = component->getParentComponent())
    {
        if (component == &presetList)
        {
            isInsidePresetList = true;
            break;
        }
    }
    if (! isInsidePresetList)
        return;

    const auto relativeEvent = event.getEventRelativeTo(&presetList);
    const int row = presetList.getRowContainingPosition(
        juce::roundToInt(relativeEvent.position.x),
        juce::roundToInt(relativeEvent.position.y));
    if (row < 0)
        showPresetContextMenu({}, event.getScreenPosition());
}

void AfterglowAudioProcessorEditor::mouseEnter(
    const juce::MouseEvent& event)
{
    const auto isWithin = [](juce::Component* component,
                             const juce::Component& owner)
    {
        for (auto* current = component; current != nullptr;
             current = current->getParentComponent())
            if (current == &owner)
                return true;
        return false;
    };

    for (const auto& dial : dials)
    {
        if (! isWithin(event.eventComponent, dial->slider)
            && ! isWithin(event.eventComponent, dial->label))
            continue;

        auto title = dial->label.getText();
        if (dial.get() == leftTimeDial)
            title = "LEFT TIME";
        else if (dial.get() == rightTimeDial)
            title = "RIGHT TIME";
        information.setText(title + "\n" + dial->help,
                            juce::dontSendNotification);
        return;
    }

    if (sequencerPanel != nullptr
        && isWithin(event.eventComponent, *sequencerPanel))
        return;

    for (auto* component = event.eventComponent;
         component != nullptr && component != &content;
         component = component->getParentComponent())
    {
        if (auto* tooltipClient = dynamic_cast<juce::TooltipClient*>(component))
        {
            const auto tooltip = tooltipClient->getTooltip();
            if (tooltip.isNotEmpty())
            {
                auto title = component->getName().trim();
                if (title.isEmpty())
                    if (auto* label = dynamic_cast<juce::Label*>(component))
                        title = label->getText().trim();
                if (title.isEmpty())
                    title = "Afterglow";
                information.setText(title.toUpperCase() + "\n" + tooltip,
                                    juce::dontSendNotification);
                return;
            }
        }
    }
}

AfterglowAudioProcessorEditor::Dial* AfterglowAudioProcessorEditor::addDial(
    const char* id, const char* caption, const char* description,
    bool attachToParameter)
{
    auto dial = std::make_unique<Dial>();
    dial->help = description;
    dial->parameterId = attachToParameter ? juce::String(id)
                                          : juce::String();
    dial->slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    dial->slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 18);
    dial->slider.setMouseDragSensitivity(80);
    dial->slider.setColour(juce::Slider::rotarySliderFillColourId, knobIndicatorColour);
    dial->slider.setColour(juce::Slider::textBoxTextColourId, knobValueColour);
    dial->slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    dial->slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    dial->label.setText(caption, juce::dontSendNotification);
    auto* dialPointer = dial.get();
    dial->label.parameterIdProvider = [dialPointer]
    {
        if (dialPointer->parameterId == "divisionL")
            return juce::String("timeL");
        if (dialPointer->parameterId == "divisionR")
            return juce::String("timeR");
        for (size_t tap = 0; tap < tapDivisionParameterIds.size(); ++tap)
            if (dialPointer->parameterId == tapDivisionParameterIds[tap])
                return juce::String(tapTimeParameterIds[tap]);
        return dialPointer->parameterId;
    };
    dial->label.setTooltip("Drag " + juce::String(caption)
                           + " onto a sequencer target slot");
    dial->label.setJustificationType(juce::Justification::centredTop);
    dial->label.setInterceptsMouseClicks(true, false);
    dial->label.setColour(juce::Label::textColourId, knobLabelColour);
    dial->label.setFont(juce::Font(juce::FontOptions(10.5f, juce::Font::bold)));
    content.addAndMakeVisible(dial->slider);
    content.addAndMakeVisible(dial->label);
    if (attachToParameter)
        dial->attachment = std::make_unique<SliderAttachment>(
            processor.parameters, id, dial->slider);
    auto* control = dial.get();
    if (attachToParameter)
        control->slider.onSavedStateReset = [this, control]
        {
            return restoreDialFromSavedState(*control);
        };
    control->slider.onValueChange = [this, control] {
        if (control == leftTimeDial && linkButton.getToggleState())
            synchroniseLinkedTimeControls();
        information.setText(control->label.getText() + "\n" + control->help, juce::dontSendNotification);
    };
    dials.push_back(std::move(dial));
    return control;
}

bool AfterglowAudioProcessorEditor::restoreDialFromSavedState(Dial& dial)
{
    if (dial.parameterId.isEmpty())
        return false;

    const auto savedValue = processor.getSavedParameterValue(dial.parameterId);
    if (! savedValue.has_value())
        return false;

    dial.slider.setValue(static_cast<double>(*savedValue),
                         juce::sendNotificationSync);
    information.setText(
        "RESTORED: " + dial.label.getText() + " = "
            + dial.slider.getTextFromValue(dial.slider.getValue())
            + "\nReturned to the value stored in the current saved state.",
        juce::dontSendNotification);
    return true;
}

void AfterglowAudioProcessorEditor::setFreezeActiveFromUi(bool active)
{
    if (updatingFreezeControls)
        return;

    {
        const juce::ScopedValueSetter<bool> guard(updatingFreezeControls, true);
        freezeButton.setToggleState(active, juce::dontSendNotification);
    }

    if (freezeAttachment != nullptr)
        freezeAttachment->setValueAsCompleteGesture(active ? 1.0f : 0.0f);

    if (active && ! freezeModeButton.getToggleState())
        information.setText(
            "FREEZE: HELD | New input is excluded while the captured delay cycle repeats without decay.\nRelease FREEZE to restore new input and the live delay tail smoothly.",
            juce::dontSendNotification);
}

void AfterglowAudioProcessorEditor::updateFreezeModeDisplay()
{
    freezeModeLabel.setText(freezeModeButton.getToggleState()
        ? "TOGGLE" : "MOMENTARY", juce::dontSendNotification);
}

void AfterglowAudioProcessorEditor::setPresetBrowserOpen(bool shouldOpen)
{
    presetBrowserOpen = shouldOpen;
    presetBrowserButton.setToggleState(shouldOpen,
                                       juce::dontSendNotification);
    if (shouldOpen)
    {
        const auto root = AfterglowAudioProcessor::getPresetsDirectory();
        root.createDirectory();
        if (! selectedPresetFolder.isDirectory()
            || (selectedPresetFolder != root
                && ! selectedPresetFolder.isAChildOf(root)))
            selectedPresetFolder = root;
        rebuildPresetFolderTree();
        information.setText(
            "PRESET BROWSER | Choose a folder, then click a preset to load it.\nSAVE overwrites the current preset; SAVE AS creates a new preset or copy.",
            juce::dontSendNotification);
    }
    updatePresetBrowserVisibility();
}

void AfterglowAudioProcessorEditor::updatePresetBrowserVisibility()
{
    for (const auto& dial : dials)
    {
        const bool visible = dial.get() == themeDial
            ? ! presetBrowserOpen
            : (dial.get() == browserMixDial
                ? presetBrowserOpen : ! presetBrowserOpen);
        dial->slider.setVisible(visible);
        dial->label.setVisible(visible);
    }

    presetFolderLabel.setVisible(presetBrowserOpen);
    presetListLabel.setVisible(presetBrowserOpen);
    presetFolderTree.setVisible(presetBrowserOpen);
    presetList.setVisible(presetBrowserOpen);
    presetSaveButton.setVisible(presetBrowserOpen);
    presetSaveAsButton.setVisible(presetBrowserOpen);
    presetOpenFolderButton.setVisible(presetBrowserOpen);
    information.setVisible(! presetBrowserOpen);
    echoOffsetLabel.setVisible(! presetBrowserOpen);
    echoOffsetSlider.setVisible(! presetBrowserOpen);
    randomButton.setVisible(! presetBrowserOpen);
    mixLockButton.setVisible(! presetBrowserOpen);
    twinTapeCharacterPageButton.setVisible(
        ! presetBrowserOpen && modeBox.getSelectedId() == 20);

    linkButton.setVisible(! presetBrowserOpen);
    pingPongButton.setVisible(! presetBrowserOpen);
    syncLeftButton.setVisible(! presetBrowserOpen);
    syncRightButton.setVisible(! presetBrowserOpen);
    multiTapButton.setVisible(! presetBrowserOpen);
    multiTapSyncButton.setVisible(false);
    for (auto& button : tapEnableButtons)
        button.setVisible(false);

    if (! presetBrowserOpen)
        updateTimeControlState();
    resized();
    repaint();
}

void AfterglowAudioProcessorEditor::setSequencerOpen(bool shouldOpen)
{
    if (sequencerOpen == shouldOpen)
        return;

    const float scale = juce::jlimit(
        0.75f, 3.0f,
        static_cast<float>(getWidth()) / static_cast<float>(designWidth));
    sequencerOpen = shouldOpen;
    sequencerButton.setToggleState(shouldOpen, juce::dontSendNotification);
    if (sequencerPanel != nullptr)
        sequencerPanel->setVisible(shouldOpen);
    if (auto* editorConstrainer = getConstrainer())
        editorConstrainer->setFixedAspectRatio(
            static_cast<double>(designWidth)
                / static_cast<double>(activeDesignHeight()));
    setResizeLimits(695, shouldOpen ? 414 : 207,
                    2778, shouldOpen ? 1656 : 828);
    setSize(juce::roundToInt(static_cast<float>(designWidth) * scale),
            juce::roundToInt(static_cast<float>(activeDesignHeight()) * scale));
    saveInterfaceSettings();
    resized();
    repaint();
}

void AfterglowAudioProcessorEditor::showLogoZoomMenu()
{
    const int currentPercent = juce::jlimit(
        75, 300, juce::roundToInt(
            100.0f * static_cast<float>(getWidth())
                / static_cast<float>(designWidth)));
    constexpr std::array<int, 10> zoomChoices
        { 75, 100, 125, 150, 175, 200, 225, 250, 275, 300 };
    juce::PopupMenu menu;
    menu.addSectionHeader("GUI Zoom");
    for (const int percent : zoomChoices)
        menu.addItem(percent, juce::String(percent) + "%", true,
                     currentPercent == percent);

    juce::Component::SafePointer<AfterglowAudioProcessorEditor> safeThis(this);
    menu.showMenuAsync(
        juce::PopupMenu::Options()
            .withTargetComponent(&logoMenuButton)
            .withPreferredPopupDirection(
                juce::PopupMenu::Options::PopupDirection::downwards),
        [safeThis](int result)
        {
            if (safeThis != nullptr && result >= 75 && result <= 300)
                safeThis->setZoomPercent(result);
        });
}

void AfterglowAudioProcessorEditor::setZoomPercent(int requestedPercent)
{
    const int percent = juce::jlimit(75, 300, requestedPercent);
    const float scale = static_cast<float>(percent) * 0.01f;
    setSize(juce::roundToInt(static_cast<float>(designWidth) * scale),
            juce::roundToInt(static_cast<float>(activeDesignHeight()) * scale));
    observedEditorWidth = getWidth();
    stableSettingsTicks = 0;
    settingsDirty = false;
    saveInterfaceSettings();
    resized();
    information.setText(
        "GUI ZOOM | " + juce::String(percent) + "%\n"
            + juce::String(getWidth()) + " x "
            + juce::String(getHeight()) + " px",
        juce::dontSendNotification);
}

void AfterglowAudioProcessorEditor::updateSequencerTargetHighlights()
{
    if (sequencerPanel == nullptr)
        return;

    const auto accent = sequencerPanel->getAccent();
    for (const auto& dial : dials)
    {
        const auto parameterId = dial->label.parameterIdProvider != nullptr
            ? dial->label.parameterIdProvider() : juce::String();
        dial->label.setTargetAppearance(
            sequencerPanel->isTargetAssigned(parameterId), accent);
    }
    sequencerPanel->updateTargetHighlights();
}

void AfterglowAudioProcessorEditor::updateAnimatedParameterDisplays()
{
    const bool animate = sequencerPanel != nullptr
                      && sequencerPanel->getAnimate();
    for (const auto& dial : dials)
    {
        const auto parameterId = dial->label.parameterIdProvider != nullptr
            ? dial->label.parameterIdProvider() : juce::String();
        const auto value = animate && parameterId.isNotEmpty()
            ? processor.getSequencerAnimatedNormalised(parameterId)
            : std::nullopt;
        auto& sliderProperties = dial->slider.getProperties();
        if (value.has_value())
        {
            const bool changed = ! sliderProperties.contains("animatedNormalised")
                || std::abs(static_cast<float>(
                       sliderProperties["animatedNormalised"]) - *value)
                       > 0.000001f;
            if (changed)
            {
                sliderProperties.set("animatedNormalised", *value);
                dial->slider.repaint();
            }
        }
        else if (sliderProperties.contains("animatedNormalised"))
        {
            sliderProperties.remove("animatedNormalised");
            dial->slider.repaint();
        }
    }
}

void AfterglowAudioProcessorEditor::randomiseDelayParameters()
{
    juce::Random random;
    for (auto* parameter : processor.getParameters())
    {
        auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(parameter);
        if (ranged == nullptr)
            continue;

        const auto id = ranged->getParameterID();
        if (id.startsWith("sequencer")
            || id.startsWith("seqStep")
            || id.startsWith("seqTarget")
            || id == "bypass" || id == "freeze"
            || id == "freezeToggle" || id == "delayStyle")
            continue;

        if (id == "feedback")
        {
            // Keep one-click randomisation below the regenerative range. The
            // full 0-200% control remains available for deliberate settings.
            setParameterActualValue(ranged, random.nextFloat() * 98.0f);
            continue;
        }
        if (id == "output")
        {
            // Random combinations of drive, feedback and filters can be hot;
            // a non-positive output trim keeps auditioning safe.
            setParameterActualValue(ranged,
                -9.0f + random.nextFloat() * 9.0f);
            continue;
        }

        ranged->beginChangeGesture();
        ranged->setValueNotifyingHost(random.nextFloat());
        ranged->endChangeGesture();
    }

    synchroniseDelayStyleFromTimingControls();
    updateTimeControlState();
    updateModeControlState();
    information.setText(
        "RANDOMISE | Delay controls, Mode, Dist Type and Echo Offset were randomised.\nThe complete sequencer, power and Freeze were left unchanged.",
        juce::dontSendNotification);
}

void AfterglowAudioProcessorEditor::loadFavouritePresets()
{
    favouritePresetPaths.clear();
    const auto file = AfterglowAudioProcessor::getSettingsDirectory()
        .getChildFile("Favourites.ini");
    if (! file.existsAsFile())
        return;

    juce::StringArray lines;
    lines.addLines(file.loadFileAsString());
    bool inFavourites = false;
    for (auto line : lines)
    {
        line = line.trim();
        if (line.startsWithChar('['))
        {
            inFavourites = line.equalsIgnoreCase("[Favourites]");
            continue;
        }
        if (! inFavourites || ! line.startsWithIgnoreCase("Preset="))
            continue;

        auto relativePath = line.substring(7).trim()
            .replaceCharacter('\\', '/');
        if (relativePath.isNotEmpty()
            && ! favouritePresetPaths.contains(relativePath, true))
            favouritePresetPaths.add(relativePath);
    }
}

void AfterglowAudioProcessorEditor::saveFavouritePresets() const
{
    const auto settingsDirectory =
        AfterglowAudioProcessor::getSettingsDirectory();
    settingsDirectory.createDirectory();
    juce::String contents("[Favourites]\r\n");
    for (const auto& relativePath : favouritePresetPaths)
        contents += "Preset=" + relativePath + "\r\n";
    settingsDirectory.getChildFile("Favourites.ini")
        .replaceWithText(contents, false, false, "\r\n");
}

bool AfterglowAudioProcessorEditor::isPresetFavourite(
    const juce::File& file) const
{
    const auto root = AfterglowAudioProcessor::getPresetsDirectory();
    if (! file.existsAsFile()
        || (file.getParentDirectory() != root
            && ! file.isAChildOf(root)))
        return false;

    const auto relativePath = file.getRelativePathFrom(root)
        .replaceCharacter('\\', '/');
    return favouritePresetPaths.contains(relativePath, true);
}

juce::Array<juce::File>
AfterglowAudioProcessorEditor::favouritePresetFiles() const
{
    const auto root = AfterglowAudioProcessor::getPresetsDirectory();
    juce::Array<juce::File> files;
    for (const auto& relativePath : favouritePresetPaths)
    {
        const auto file = root.getChildFile(relativePath);
        if (file.existsAsFile() && file.hasFileExtension(presetFileExtension)
            && (file.getParentDirectory() == root || file.isAChildOf(root)))
            files.addIfNotAlreadyThere(file);
    }
    std::sort(files.begin(), files.end(),
              [](const juce::File& first, const juce::File& second)
              {
                  const int nameOrder = first.getFileNameWithoutExtension()
                      .compareNatural(second.getFileNameWithoutExtension(),
                                      true);
                  return nameOrder != 0
                      ? nameOrder < 0
                      : first.getFullPathName().compareNatural(
                            second.getFullPathName(), true) < 0;
              });
    return files;
}

juce::Array<juce::File>
AfterglowAudioProcessorEditor::navigationPresetFiles() const
{
    if (favouritesViewSelected)
        return favouritePresetFiles();

    const auto root = AfterglowAudioProcessor::getPresetsDirectory();
    auto folder = currentPresetFile.existsAsFile()
        ? currentPresetFile.getParentDirectory()
        : selectedPresetFolder;
    if (! folder.isDirectory()
        || (folder != root && ! folder.isAChildOf(root)))
        folder = root;
    return presetFilesInFolder(folder);
}

void AfterglowAudioProcessorEditor::togglePresetFavourite(
    const juce::File& file)
{
    const auto root = AfterglowAudioProcessor::getPresetsDirectory();
    if (! file.existsAsFile()
        || (file.getParentDirectory() != root && ! file.isAChildOf(root)))
        return;

    const auto relativePath = file.getRelativePathFrom(root)
        .replaceCharacter('\\', '/');
    const int existing = favouritePresetPaths.indexOf(relativePath, true);
    if (existing >= 0)
        favouritePresetPaths.remove(existing);
    else
        favouritePresetPaths.add(relativePath);
    saveFavouritePresets();
    refreshPresetList();
}

void AfterglowAudioProcessorEditor::copyPreset(const juce::File& file)
{
    if (! file.existsAsFile() || ! file.hasFileExtension(presetFileExtension))
        return;
    copiedPresetFile = file;
    information.setText(
        "PRESET COPIED: " + file.getFileNameWithoutExtension()
            + "\nRight-click a real preset folder or preset list and choose Paste preset here.",
        juce::dontSendNotification);
}

void AfterglowAudioProcessorEditor::pastePresetIntoFolder(
    const juce::File& folder)
{
    const auto root = AfterglowAudioProcessor::getPresetsDirectory();
    if (! copiedPresetFile.existsAsFile() || ! folder.isDirectory()
        || (folder != root && ! folder.isAChildOf(root)))
        return;

    const auto baseName = copiedPresetFile.getFileNameWithoutExtension();
    auto destination = folder.getChildFile(
        baseName + presetFileExtension);
    if (destination.exists())
    {
        destination = folder.getChildFile(
            baseName + " Copy" + presetFileExtension);
        int copyNumber = 2;
        while (destination.exists())
            destination = folder.getChildFile(
                baseName + " Copy " + juce::String(copyNumber++)
                    + presetFileExtension);
    }

    if (! copiedPresetFile.copyFileTo(destination))
    {
        information.setText(
            "PRESET PASTE FAILED | The preset file could not be copied.",
            juce::dontSendNotification);
        return;
    }

    selectedPresetFolder = folder;
    favouritesViewSelected = false;
    rebuildPresetFolderTree();
    information.setText(
        "PRESET PASTED: " + destination.getFileNameWithoutExtension()
            + "\nThe copy is independent of the original and can be edited or saved normally.",
        juce::dontSendNotification);
}

void AfterglowAudioProcessorEditor::showPresetContextMenu(
    const juce::File& file, juce::Point<int> position)
{
    juce::PopupMenu menu;
    menu.setLookAndFeel(&lookAndFeel);
    if (file.existsAsFile())
    {
        menu.addItem(1, "Copy preset");
        menu.addSeparator();
    }
    menu.addItem(2, "Paste preset here",
                 copiedPresetFile.existsAsFile()
                    && ! favouritesViewSelected
                    && selectedPresetFolder.isDirectory());
    lookAndFeel.setPopupBorderColour(modeColourForId(modeBox.getSelectedId()));
    const auto safeThis = juce::Component::SafePointer<
        AfterglowAudioProcessorEditor>(this);
    const auto destinationFolder = selectedPresetFolder;
    menu.showMenuAsync(
        juce::PopupMenu::Options()
            .withTargetScreenArea({ position.x, position.y, 1, 1 })
            .withMinimumWidth(150),
        [safeThis, file, destinationFolder](int result)
        {
            if (auto* editor = safeThis.getComponent())
            {
                if (result == 1)
                    editor->copyPreset(file);
                else if (result == 2)
                    editor->pastePresetIntoFolder(destinationFolder);
            }
        });
}

void AfterglowAudioProcessorEditor::showCreatePresetFolderDialog(
    const juce::File& parentFolder)
{
    const auto root = AfterglowAudioProcessor::getPresetsDirectory();
    if (! parentFolder.isDirectory()
        || (parentFolder != root && ! parentFolder.isAChildOf(root)))
        return;

    presetFolderNameWindow = std::make_unique<juce::AlertWindow>(
        "New Preset Folder",
        "Enter a name for the new folder inside "
            + (parentFolder == root ? juce::String("Presets")
                                    : parentFolder.getFileName()) + ".",
        juce::AlertWindow::NoIcon);
    presetFolderNameWindow->setLookAndFeel(&lookAndFeel);
    presetFolderNameWindow->addTextEditor(
        "folderName", {}, "Folder name:");
    presetFolderNameWindow->addButton(
        "Create", 1,
        juce::KeyPress(juce::KeyPress::returnKey, 0, 0));
    presetFolderNameWindow->addButton(
        "Cancel", 0,
        juce::KeyPress(juce::KeyPress::escapeKey, 0, 0));

    const auto safeThis = juce::Component::SafePointer<
        AfterglowAudioProcessorEditor>(this);
    presetFolderNameWindow->enterModalState(
        true,
        juce::ModalCallbackFunction::create(
            [safeThis, parentFolder](int result)
            {
                auto* editor = safeThis.getComponent();
                if (editor == nullptr || editor->presetFolderNameWindow == nullptr)
                    return;

                const auto name = editor->presetFolderNameWindow
                    ->getTextEditorContents("folderName").trim();
                editor->presetFolderNameWindow->setLookAndFeel(nullptr);
                editor->presetFolderNameWindow.reset();
                if (result != 1)
                    return;

                if (name.isEmpty() || name == "." || name == ".."
                    || name.containsAnyOf("\\/:*?\"<>|")
                    || name.endsWithChar('.') || name.endsWithChar(' '))
                {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::WarningIcon,
                        "Invalid Folder Name",
                        "Please use a normal folder name without \\ / : * ? \" < > | or a trailing dot.",
                        "OK", editor);
                    return;
                }

                const auto folder = parentFolder.getChildFile(name);
                if (folder.exists())
                {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::WarningIcon,
                        "Folder Already Exists",
                        "A file or folder with that name already exists.",
                        "OK", editor);
                    return;
                }

                const auto createResult = folder.createDirectory();
                if (createResult.failed())
                {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::WarningIcon,
                        "Folder Creation Failed",
                        createResult.getErrorMessage(), "OK", editor);
                    return;
                }

                editor->selectedPresetFolder = folder;
                editor->favouritesViewSelected = false;
                editor->rebuildPresetFolderTree();
            }),
        false);
}

void AfterglowAudioProcessorEditor::confirmDeletePresetFolder(
    const juce::File& folder)
{
    const auto root = AfterglowAudioProcessor::getPresetsDirectory();
    if (! folder.isDirectory() || folder == root
        || ! folder.isAChildOf(root))
        return;

    const bool hasContents = folder.getNumberOfChildFiles(
        juce::File::findFilesAndDirectories) > 0;
    const auto message = hasContents
        ? "Delete the folder '" + folder.getFileName()
            + "' and everything inside it?\n\nThis moves the folder, presets and subfolders to the system Recycle Bin or Trash."
        : "Delete the empty folder '" + folder.getFileName()
            + "'?\n\nThis moves it to the system Recycle Bin or Trash.";
    const auto safeThis = juce::Component::SafePointer<
        AfterglowAudioProcessorEditor>(this);
    juce::AlertWindow::showOkCancelBox(
        juce::AlertWindow::WarningIcon,
        "Delete Preset Folder", message,
        "Delete", "Cancel", this,
        juce::ModalCallbackFunction::create(
            [safeThis, folder](int result)
            {
                auto* editor = safeThis.getComponent();
                if (editor == nullptr || result != 1)
                    return;

                const bool containedCurrentPreset =
                    editor->currentPresetFile.existsAsFile()
                    && editor->currentPresetFile.isAChildOf(folder);
                const bool containedCopiedPreset =
                    editor->copiedPresetFile.existsAsFile()
                    && editor->copiedPresetFile.isAChildOf(folder);
                if (! folder.moveToTrash())
                {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::WarningIcon,
                        "Folder Deletion Failed",
                        "The preset folder could not be moved to the system Recycle Bin or Trash.",
                        "OK", editor);
                    return;
                }

                const auto rootDirectory =
                    AfterglowAudioProcessor::getPresetsDirectory();
                bool favouritesChanged = false;
                for (int index = editor->favouritePresetPaths.size();
                     --index >= 0;)
                {
                    const auto favouriteFile = rootDirectory.getChildFile(
                        editor->favouritePresetPaths[index]);
                    if (favouriteFile.isAChildOf(folder))
                    {
                        editor->favouritePresetPaths.remove(index);
                        favouritesChanged = true;
                    }
                }
                if (favouritesChanged)
                    editor->saveFavouritePresets();
                if (containedCopiedPreset)
                    editor->copiedPresetFile = juce::File();
                if (containedCurrentPreset)
                {
                    editor->currentPresetFile = juce::File();
                    editor->processor.parameters.state.removeProperty(
                        "currentPresetPath", nullptr);
                }

                editor->selectedPresetFolder = rootDirectory;
                editor->favouritesViewSelected = false;
                editor->rebuildPresetFolderTree();
                editor->updatePresetNavigationDisplay();
            }));
}

void AfterglowAudioProcessorEditor::showPresetFolderContextMenu(
    const juce::File& folder, bool favourites, juce::Point<int> position)
{
    juce::PopupMenu menu;
    menu.setLookAndFeel(&lookAndFeel);
    menu.addItem(1, "New folder...");
    menu.addItem(2, "Delete folder...",
                 ! favourites
                    && folder != AfterglowAudioProcessor::getPresetsDirectory()
                    && folder.isDirectory());
    menu.addSeparator();
    menu.addItem(3, "Paste preset here",
                 ! favourites && copiedPresetFile.existsAsFile()
                    && folder.isDirectory());
    lookAndFeel.setPopupBorderColour(modeColourForId(modeBox.getSelectedId()));
    const auto safeThis = juce::Component::SafePointer<
        AfterglowAudioProcessorEditor>(this);
    menu.showMenuAsync(
        juce::PopupMenu::Options()
            .withTargetScreenArea({ position.x, position.y, 1, 1 })
            .withMinimumWidth(150),
        [safeThis, folder, favourites](int result)
        {
            if (auto* editor = safeThis.getComponent())
            {
                if (result == 1)
                    editor->showCreatePresetFolderDialog(
                        favourites
                            ? AfterglowAudioProcessor::getPresetsDirectory()
                            : folder);
                else if (result == 2)
                    editor->confirmDeletePresetFolder(folder);
                else if (result == 3)
                    editor->pastePresetIntoFolder(folder);
            }
        });
}

void AfterglowAudioProcessorEditor::rebuildPresetFolderTree()
{
    const auto root = AfterglowAudioProcessor::getPresetsDirectory();
    root.createDirectory();
    presetFolderTree.setRootItem(nullptr);
    presetFolderRoot.reset();
    presetFolderRoot = std::make_unique<FolderTreeItem>(
        root,
        [this](const juce::File& folder, bool favourites)
        {
            if (favourites)
                selectFavouritePresets();
            else
                selectPresetFolder(folder);
        },
        [this](const juce::File& folder, bool favourites,
               juce::Point<int> position)
        {
            showPresetFolderContextMenu(folder, favourites, position);
        });
    presetFolderTree.setRootItem(presetFolderRoot.get());
    presetFolderRoot->setOpen(true);

    auto target = selectedPresetFolder;
    if (! target.isDirectory()
        || (target != root && ! target.isAChildOf(root)))
        target = root;

    if (auto* item = presetFolderRoot->findItemForDirectory(
            target, favouritesViewSelected))
    {
        for (auto* parent = item->getParentItem(); parent != nullptr;
             parent = parent->getParentItem())
            parent->setOpen(true);
        item->setSelected(true, true, juce::sendNotification);
        presetFolderTree.scrollToKeepItemVisible(item);
    }
    else
    {
        selectedPresetFolder = root;
        presetFolderRoot->setSelected(true, true, juce::sendNotification);
    }
}

void AfterglowAudioProcessorEditor::selectPresetFolder(
    const juce::File& folder)
{
    const auto root = AfterglowAudioProcessor::getPresetsDirectory();
    if (! folder.isDirectory()
        || (folder != root && ! folder.isAChildOf(root)))
        return;

    favouritesViewSelected = false;
    selectedPresetFolder = folder;
    refreshPresetList();
}

void AfterglowAudioProcessorEditor::selectFavouritePresets()
{
    favouritesViewSelected = true;
    refreshPresetList();
}

void AfterglowAudioProcessorEditor::refreshPresetList()
{
    visiblePresetFiles = favouritesViewSelected
        ? favouritePresetFiles()
        : presetFilesInFolder(selectedPresetFolder);

    presetList.updateContent();
    presetList.deselectAllRows();
    if ((favouritesViewSelected && isPresetFavourite(currentPresetFile))
        || (! favouritesViewSelected
            && currentPresetFile.getParentDirectory() == selectedPresetFolder))
        for (int index = 0; index < visiblePresetFiles.size(); ++index)
            if (visiblePresetFiles[index] == currentPresetFile)
            {
                selectedPresetBrowserColumn = index % 2;
                presetList.selectRow(index / 2);
                break;
            }
    presetList.repaint();
    updatePresetNavigationDisplay();
}

void AfterglowAudioProcessorEditor::updatePresetNavigationDisplay()
{
    const auto files = navigationPresetFiles();
    const bool hasPresets = ! files.isEmpty();
    previousPresetButton.setEnabled(hasPresets);
    nextPresetButton.setEnabled(hasPresets);

    if (currentPresetFile != juce::File())
    {
        currentPresetLabel.setText(
            currentPresetFile.getFileNameWithoutExtension(),
            juce::dontSendNotification);
        currentPresetLabel.setTooltip(currentPresetFile.getFullPathName());
    }
    else
    {
        currentPresetLabel.setText("NO PRESET", juce::dontSendNotification);
        currentPresetLabel.setTooltip({});
    }
}

bool AfterglowAudioProcessorEditor::handlePresetCursorKey(int direction)
{
    auto& keyDown = direction < 0
        ? previousPresetKeyDown : nextPresetKeyDown;
    if (! keyDown)
        stepPreset(direction);
    keyDown = true;
    return true;
}

void AfterglowAudioProcessorEditor::stepPreset(int direction)
{
    const auto files = navigationPresetFiles();
    if (files.isEmpty())
    {
        information.setText(
            "PRESET NAVIGATION | No presets are available in the selected folder.",
            juce::dontSendNotification);
        updatePresetNavigationDisplay();
        return;
    }

    int currentIndex = files.indexOf(currentPresetFile);
    if (currentIndex < 0)
        currentIndex = direction >= 0 ? -1 : 0;
    const int nextIndex = (currentIndex + (direction >= 0 ? 1 : -1)
                           + files.size()) % files.size();
    loadPresetFile(files[nextIndex]);
}

void AfterglowAudioProcessorEditor::showPresetQuickMenu()
{
    const auto files = navigationPresetFiles();
    if (files.isEmpty())
    {
        information.setText(
            "PRESET MENU | No presets are available in the current folder.",
            juce::dontSendNotification);
        return;
    }

    const auto safeThis = juce::Component::SafePointer<
        AfterglowAudioProcessorEditor>(this);
    auto quickList = std::make_unique<PresetPopupList>(
        files, currentPresetFile,
        [safeThis](const juce::File& file)
        {
            if (auto* editor = safeThis.getComponent())
                editor->loadPresetFile(file);
        });

    juce::PopupMenu menu;
    menu.setLookAndFeel(&lookAndFeel);
    menu.addCustomItem(1, std::move(quickList), nullptr, "Preset list");
    lookAndFeel.setPopupBorderColour(modeColourForId(modeBox.getSelectedId()));
    menu.showMenuAsync(
        juce::PopupMenu::Options()
            .withTargetComponent(&currentPresetLabel)
            .withTargetScreenArea(currentPresetLabel.getScreenBounds())
            .withPreferredPopupDirection(
                juce::PopupMenu::Options::PopupDirection::downwards)
            .withMinimumWidth(220)
            .withMaximumNumColumns(1));
}

int AfterglowAudioProcessorEditor::getNumRows()
{
    return (visiblePresetFiles.size() + 1) / 2;
}

void AfterglowAudioProcessorEditor::paintListBoxItem(
    int row, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    const int firstColumnWidth = width / 2;
    for (int column = 0; column < 2; ++column)
    {
        const int index = row * 2 + column;
        if (! juce::isPositiveAndBelow(index, visiblePresetFiles.size()))
            continue;

        const int cellX = column == 0 ? 0 : firstColumnWidth;
        const int cellWidth = column == 0
            ? firstColumnWidth : width - firstColumnWidth;
        const auto& file = visiblePresetFiles.getReference(index);
        const bool isSelected = rowIsSelected
                             && column == selectedPresetBrowserColumn;
        if (isSelected)
        {
            g.setColour(presetAccentColour.withAlpha(0.30f));
            g.fillRoundedRectangle(static_cast<float>(cellX + 2), 1.0f,
                                   static_cast<float>(cellWidth - 4),
                                   static_cast<float>(height - 2), 3.0f);
        }

        const bool isCurrent = file == currentPresetFile;
        const bool isFavourite = isPresetFavourite(file);
        g.setColour(isFavourite ? knobIndicatorColour
                                : knobLabelColour.darker(0.25f));
        g.fillPath(makeStarPath({ static_cast<float>(cellX + 7),
                                 static_cast<float>(height) * 0.5f - 5.0f,
                                 10.0f, 10.0f }));
        g.setColour(isCurrent ? knobIndicatorColour : knobValueColour);
        g.setFont(juce::Font(juce::FontOptions(
            11.0f, isCurrent ? juce::Font::bold : juce::Font::plain)));
        g.drawFittedText(file.getFileNameWithoutExtension(), cellX + 24, 0,
                         juce::jmax(0, cellWidth - 48), height,
                         juce::Justification::centredLeft, 1);
        if (isCurrent)
        {
            g.setColour(knobIndicatorColour);
            g.fillEllipse(static_cast<float>(cellX + cellWidth - 18),
                          static_cast<float>(height) * 0.5f - 3.0f,
                          6.0f, 6.0f);
        }
    }
}

void AfterglowAudioProcessorEditor::listBoxItemClicked(
    int row, const juce::MouseEvent& event)
{
    const int eventWidth = event.eventComponent != nullptr
        ? event.eventComponent->getWidth() : presetList.getWidth();
    const int clickedColumn = event.position.x
        >= static_cast<float>(eventWidth) * 0.5f ? 1 : 0;
    const int clickedIndex = row * 2 + clickedColumn;
    if (! juce::isPositiveAndBelow(clickedIndex, visiblePresetFiles.size()))
    {
        if (event.mods.isPopupMenu())
            showPresetContextMenu({}, event.getScreenPosition());
        return;
    }
    selectedPresetBrowserColumn = clickedColumn;
    presetList.repaintRow(row);
    const int index = row * 2 + selectedPresetBrowserColumn;
    if (! juce::isPositiveAndBelow(index, visiblePresetFiles.size()))
        return;

    const int firstColumnWidth = eventWidth / 2;
    const int cellX = selectedPresetBrowserColumn == 0
        ? 0 : firstColumnWidth;
    const float positionInCell = event.position.x
        - static_cast<float>(cellX);
    const auto file = visiblePresetFiles[index];
    if (event.mods.isPopupMenu())
    {
        showPresetContextMenu(file, event.getScreenPosition());
        return;
    }
    if (positionInCell >= 3.0f && positionInCell <= 22.0f)
    {
        togglePresetFavourite(file);
        return;
    }
    loadPresetFile(file);
}

void AfterglowAudioProcessorEditor::returnKeyPressed(int row)
{
    loadPresetAtBrowserCell(row, selectedPresetBrowserColumn);
}

void AfterglowAudioProcessorEditor::selectedRowsChanged(int row)
{
    if (row < 0)
        return;

    if (! juce::isPositiveAndBelow(
            row * 2 + selectedPresetBrowserColumn,
            visiblePresetFiles.size()))
        selectedPresetBrowserColumn = 0;

    presetList.repaintRow(row);
}

void AfterglowAudioProcessorEditor::loadPresetAtBrowserCell(
    int row, int column)
{
    const int index = row * 2 + juce::jlimit(0, 1, column);
    if (! juce::isPositiveAndBelow(index, visiblePresetFiles.size()))
        return;

    loadPresetFile(visiblePresetFiles[index]);
}

void AfterglowAudioProcessorEditor::loadPresetFile(const juce::File& file)
{
    const bool preserveMix = mixLockButton.getToggleState();
    const double mixToPreserve = lockedMixValue;
    const juce::ScopedValueSetter<bool> mixRestoreGuard(
        restoringLockedMix, preserveMix);
    const auto result = processor.loadPresetFromFile(file);
    if (result.failed())
    {
        information.setText("PRESET LOAD FAILED | " + result.getErrorMessage(),
                            juce::dontSendNotification);
        return;
    }

    if (preserveMix)
    {
        setParameterActualValue(
            processor.parameters.getParameter("mix"),
            static_cast<float>(mixToPreserve));
        lockedMixValue = mixToPreserve;
    }

    currentPresetFile = file;
    selectedPresetFolder = file.getParentDirectory();
    if (presetBrowserOpen)
        refreshPresetList();
    else
        updatePresetNavigationDisplay();
    updateTimeControlState();
    updateModeControlState();
    information.setText(
        "PRESET LOADED: " + file.getFileNameWithoutExtension()
            + "\nClick another preset to load it, or use the Save As... button to save a copy.",
        juce::dontSendNotification);
}

void AfterglowAudioProcessorEditor::saveCurrentPreset()
{
    if (currentPresetFile == juce::File())
    {
        showPresetSaveAs();
        return;
    }

    const auto result = processor.savePresetToFile(currentPresetFile);
    if (result.failed())
    {
        information.setText("PRESET SAVE FAILED | " + result.getErrorMessage(),
                            juce::dontSendNotification);
        return;
    }

    refreshPresetList();
    updatePresetNavigationDisplay();
    information.setText(
        "PRESET SAVED: " + currentPresetFile.getFileNameWithoutExtension()
            + "\nThe current preset file has been overwritten with the latest settings.",
        juce::dontSendNotification);
}

void AfterglowAudioProcessorEditor::showPresetSaveAs()
{
    const auto root = AfterglowAudioProcessor::getPresetsDirectory();
    root.createDirectory();
    auto destinationFolder = selectedPresetFolder;
    if (! destinationFolder.isDirectory()
        || (destinationFolder != root
            && ! destinationFolder.isAChildOf(root)))
        destinationFolder = root;

    const auto suggestedName = currentPresetFile == juce::File()
        ? juce::String("New Preset")
        : currentPresetFile.getFileNameWithoutExtension();
    const auto suggestedFile = destinationFolder.getChildFile(
        suggestedName + presetFileExtension);
    presetFileChooser = std::make_unique<juce::FileChooser>(
        "Save Afterglow Preset As", suggestedFile,
        "*" + juce::String(presetFileExtension), true, false, this);
    const auto safeThis = juce::Component::SafePointer<
        AfterglowAudioProcessorEditor>(this);
    presetFileChooser->launchAsync(
        juce::FileBrowserComponent::saveMode
            | juce::FileBrowserComponent::canSelectFiles
            | juce::FileBrowserComponent::warnAboutOverwriting,
        [safeThis](const juce::FileChooser& chooser)
        {
            auto* editor = safeThis.getComponent();
            if (editor == nullptr)
                return;

            auto file = chooser.getResult();
            if (file == juce::File())
                return;
            if (! file.hasFileExtension(presetFileExtension))
                file = file.withFileExtension(presetFileExtension);

            const auto rootDirectory =
                AfterglowAudioProcessor::getPresetsDirectory();
            const auto parentDirectory = file.getParentDirectory();
            if (parentDirectory != rootDirectory
                && ! parentDirectory.isAChildOf(rootDirectory))
            {
                editor->information.setText(
                    "PRESET SAVE FAILED | Presets must be saved inside the Afterglow Presets folder.",
                    juce::dontSendNotification);
                return;
            }

            const auto result = editor->processor.savePresetToFile(file);
            if (result.failed())
            {
                editor->information.setText(
                    "PRESET SAVE FAILED | " + result.getErrorMessage(),
                    juce::dontSendNotification);
                return;
            }

            editor->currentPresetFile = file;
            editor->selectedPresetFolder = parentDirectory;
            editor->rebuildPresetFolderTree();
            editor->updatePresetNavigationDisplay();
            editor->information.setText(
                "PRESET SAVED AS: " + file.getFileNameWithoutExtension()
                    + "\nThe Save button now overwrites this file; use Save As... to save another copy.",
                juce::dontSendNotification);
        });
}

void AfterglowAudioProcessorEditor::synchroniseLinkedTimeControls()
{
    if (! linkButton.getToggleState() || leftTimeDial == nullptr || rightTimeDial == nullptr)
        return;

    if (std::abs(rightTimeDial->slider.getValue()
                 - leftTimeDial->slider.getValue()) > 1.0e-9)
        rightTimeDial->slider.setValue(leftTimeDial->slider.getValue(), juce::dontSendNotification);
}

void AfterglowAudioProcessorEditor::synchroniseDelayStyleFromTimingControls()
{
    if (delayStyleAttachment == nullptr)
        return;

    int style = 1; // Dual
    if (multiTapButton.getToggleState())
    {
        style = 4; // Quad
    }
    else if (pingPongButton.getToggleState())
    {
        style = 3; // Ping Pong
    }
    else if (linkButton.getToggleState())
    {
        style = 0; // Single
    }
    else if (const auto* storedStyle =
                 processor.parameters.getRawParameterValue("delayStyle"))
    {
        // Ratio has no dedicated front-panel switch.  Preserve it until the
        // user deliberately changes one of the timing-style controls.
        if (static_cast<int>(std::lround(storedStyle->load())) == 2)
            style = 2;
    }

    delayStyleAttachment->setValueAsCompleteGesture(
        static_cast<float>(style));
}

void AfterglowAudioProcessorEditor::updateTimeControlState()
{
    const bool controlsVisible = ! presetBrowserOpen;
    const bool twinTape = modeBox.getSelectedId() == 20;
    const bool multiTap = multiTapButton.getToggleState() && ! twinTape;
    const bool linked = linkButton.getToggleState();
    const bool leftSynced = syncLeftButton.getToggleState();
    const auto* storedRightSync =
        processor.parameters.getRawParameterValue("syncR");
    const bool storedRightSynced = storedRightSync != nullptr
        && storedRightSync->load() >= 0.5f;
    const bool rightSynced = linked ? leftSynced : storedRightSynced;
    if (leftTimeDial == nullptr || rightTimeDial == nullptr)
        return;

    syncRightButton.setToggleState(rightSynced, juce::dontSendNotification);

    leftTimeDial->slider.setVisible(controlsVisible && ! multiTap);
    rightTimeDial->slider.setVisible(controlsVisible && ! multiTap);
    leftTimeDial->label.setVisible(controlsVisible && ! multiTap);
    rightTimeDial->label.setVisible(controlsVisible && ! multiTap);
    linkButton.setVisible(controlsVisible && ! multiTap);
    pingPongButton.setVisible(controlsVisible && ! multiTap);
    syncLeftButton.setVisible(controlsVisible && ! multiTap);
    syncRightButton.setVisible(controlsVisible && ! multiTap);
    multiTapButton.setVisible(controlsVisible && ! twinTape);
    multiTapButton.setEnabled(! twinTape);
    multiTapSyncButton.setVisible(controlsVisible && multiTap);

    leftTimeDial->attachment.reset();
    rightTimeDial->attachment.reset();

    const char* leftParameter = leftSynced ? "divisionL" : "timeL";
    const char* rightParameter = linked
                               ? leftParameter
                               : (rightSynced ? "divisionR" : "timeR");
    leftTimeDial->parameterId = leftParameter;
    rightTimeDial->parameterId = rightParameter;
    leftTimeDial->attachment = std::make_unique<SliderAttachment>(
        processor.parameters, leftParameter, leftTimeDial->slider);
    rightTimeDial->attachment = std::make_unique<SliderAttachment>(
        processor.parameters, rightParameter, rightTimeDial->slider);

    leftTimeDial->slider.setEnabled(true);
    rightTimeDial->slider.setEnabled(! linked);
    rightTimeDial->label.setEnabled(! linked);
    syncRightButton.setEnabled(! linked);
    leftTimeDial->help = leftSynced
        ? "Left tempo-synchronised note division. With Link L/R enabled, this controls both channels."
        : "Left delay time in milliseconds. With Link L/R enabled, this controls both channels.";
    rightTimeDial->help = rightSynced
        ? "Right tempo-synchronised note division. Link L/R makes this follow the left knob."
        : "Right delay time in milliseconds. Link L/R makes this follow the left knob.";

    const bool tapSync = multiTapSyncButton.getToggleState();
    for (size_t tap = 0; tap < multiTapCount; ++tap)
    {
        if (tapTimeDials[tap] == nullptr || tapPanDials[tap] == nullptr
            || tapLevelDials[tap] == nullptr)
            continue;

        tapTimeDials[tap]->attachment.reset();
        tapTimeDials[tap]->parameterId = tapSync
            ? tapDivisionParameterIds[tap]
            : tapTimeParameterIds[tap];
        tapTimeDials[tap]->attachment = std::make_unique<SliderAttachment>(
            processor.parameters,
            tapSync ? tapDivisionParameterIds[tap]
                    : tapTimeParameterIds[tap],
            tapTimeDials[tap]->slider);
        tapTimeDials[tap]->help = tapSync
            ? "Tap " + juce::String(static_cast<int>(tap) + 1)
                + " tempo-synchronised note division. Global Sync preserves its stored millisecond value."
            : "Tap " + juce::String(static_cast<int>(tap) + 1)
                + " delay time in milliseconds. Global Sync restores its separately stored note division.";
        tapTimeDials[tap]->slider.setVisible(controlsVisible && multiTap);
        tapPanDials[tap]->slider.setVisible(controlsVisible && multiTap);
        tapLevelDials[tap]->slider.setVisible(controlsVisible && multiTap);
        tapTimeDials[tap]->label.setVisible(controlsVisible && multiTap);
        tapPanDials[tap]->label.setVisible(controlsVisible && multiTap);
        tapLevelDials[tap]->label.setVisible(controlsVisible && multiTap);
        tapEnableButtons[tap].setVisible(controlsVisible && multiTap);
    }
    repaint();
}

void AfterglowAudioProcessorEditor::updateContextSensitiveHelp(
    int selectedMode)
{
    if (feedbackDial != nullptr)
    {
        switch (selectedMode)
        {
            case 2:
            case 12:
            case 18:
                feedbackDial->help = "Regeneration passes through the selected tape colour, so wear and saturation accumulate on every repeat.";
                break;
            case 3:
            case 13:
                feedbackDial->help = "Bucket-brigade regeneration can emphasise resonant tones and noise at short times and high values.";
                break;
            case 5:
                feedbackDial->help = "Regeneration repeats the frequency shift, so the Ghost spectrum keeps moving on every echo.";
                break;
            case 6:
            case 7:
            case 11:
                feedbackDial->help = "Regeneration repeats the pitch process, so the selected interval accumulates across successive echoes.";
                break;
            case 8:
                feedbackDial->help = "Regeneration recirculates LoFi wear, noise and instability. High values create progressively rougher tails.";
                break;
            case 10:
                feedbackDial->help = "Regeneration recirculates the Phaser. High FDBK and RES settings can reinforce narrow resonant peaks.";
                break;
            case 14:
            case 16:
                feedbackDial->help = "Regeneration layers the mode's changing delay and pitch pattern into an evolving, irregular tail.";
                break;
            case 15:
                feedbackDial->help = "Regeneration layers additional reverse grains while preserving their original pitch.";
                break;
            case 17:
                feedbackDial->help = "Regeneration builds the separately diffused reverse path behind the audible forward repeats.";
                break;
            case 9:
                feedbackDial->help = "Clean bright regeneration with restrained soft-knee control at dense settings.";
                break;
            case 20:
                feedbackDial->help = "Regeneration follows both variable-speed tape heads, so pitch, direction, Age and Shine accumulate through the echo tail.";
                break;
            case 21:
                feedbackDial->help = "Regeneration recirculates the overlapping forward grains, allowing pitch, scatter and texture to build across the tail.";
                break;
            default:
                feedbackDial->help = "Sets repeat regeneration. Higher values lengthen the tail; 100% sustains and values above it self-regenerate.";
                break;
        }
    }

    if (modulationAmountDial != nullptr)
    {
        switch (selectedMode)
        {
            case 8:
                modulationAmountDial->help = "In LoFi, increases irregular stereo drift and unstable worn-tape movement.";
                break;
            case 9:
                modulationAmountDial->help = "In Crystal, controls clean stereo triangle-wave delay movement.";
                break;
            case 10:
                modulationAmountDial->help = "In Phaser, controls the depth of the moving stereo notch sweep.";
                break;
            case 14:
            case 16:
                modulationAmountDial->help = "In Chaotic modes, increases the range of the wandering delay and pitch motion.";
                break;
            case 19:
                modulationAmountDial->help = "In Colour, moves the virtual-clock heads continuously through the same bounded path as the Time controls.";
                break;
            case 20:
                modulationAmountDial->help = "In Twin Tape, adds smooth movement around both independent tape-cycle times before each head's Age motion.";
                break;
            case 21:
                modulationAmountDial->help = "In Grain, increases the stereo scatter between overlapping grains; MOD RATE controls how quickly that scatter moves.";
                break;
            default:
                modulationAmountDial->help = "Controls the depth of smooth stereo delay-time movement in this mode.";
                break;
        }
    }

    if (modeDials.size() > 1)
    {
        auto* characterDial = modeDials[1];
        switch (selectedMode)
        {
            case 1:  characterDial->help = "Reduces Digital's floating-converter bit depth, moving from clean to coarse resolution."; break;
            case 2:  characterDial->help = "In Tape I, increases head loss, saturation, splice wear and compression."; break;
            case 3:  characterDial->help = "In Brigade, narrows bandwidth and increases clock grain, noise and saturation."; break;
            case 4:  characterDial->help = "In HiFi, adds restrained bandwidth wear and signal-tracking asperity."; break;
            case 5:  characterDial->help = "In Ghost, progressively darkens and saturates the frequency-shifted repeats."; break;
            case 6:
            case 7:  characterDial->help = "In this Pitch mode, reduces floating-converter bit depth from clean to coarse."; break;
            case 8:  characterDial->help = "In LoFi, increases scrape motion, bandwidth loss, noise and roughness."; break;
            case 9:  characterDial->help = "In Crystal, lowers a steep output filter without darkening each feedback circulation."; break;
            case 10: characterDial->help = "AGE is not used in Phaser mode."; break;
            case 11: characterDial->help = "Blends the stereo-detuned voice into the shifted voice; 50% combines them equally."; break;
            case 12: characterDial->help = "In Chromatic, moves from bright clean heads to dark compressed tape wear."; break;
            case 13: characterDial->help = "In Analogue, narrows bandwidth, raises clock texture and thickens saturation."; break;
            case 18: characterDial->help = "In Tape II, increases head wear, magnetic compression and darker tape colour."; break;
            case 19: characterDial->help = "In Colour, blends the clean cubic delay into denser companding, softer bandwidth and asymmetric saturation."; break;
            case 20:
                characterDial->help = twinTapeCharacterPageButton
                        .getToggleState()
                    ? "Tape I Shine controls its independent high-frequency openness before both heads enter Feedback."
                    : "Shared Twin Tape Age darkens and saturates both heads while preserving their independent Age and Shine settings.";
                break;
            case 21: characterDial->help = "In Grain, shortens the overlapping grain windows, increases scatter and reduces floating-converter resolution."; break;
            default: characterDial->help = "AGE is not used in this mode."; break;
        }
    }
}

void AfterglowAudioProcessorEditor::updateModeControlState()
{
    const int selectedMode = modeBox.getSelectedId();
    const bool twinTapeEnabled = selectedMode == 20;
    const bool twinTapeHeadPage = twinTapeEnabled
        && twinTapeCharacterPageButton.getToggleState();
    const bool ageEnabled = (selectedMode >= 1 && selectedMode <= 9)
                         || selectedMode == 12 || selectedMode == 13
                         || selectedMode == 18 || selectedMode == 19
                         || selectedMode == 21;
    const bool phaserEnabled = selectedMode == 10;
    const bool ghostEnabled = selectedMode == 5;
    const bool lushPitchEnabled = selectedMode == 11;
    const bool pitchEnabled = selectedMode == 6 || selectedMode == 7
                           || lushPitchEnabled || selectedMode == 21;
    const bool echoOffsetEnabled = selectedMode >= 14
                                && selectedMode <= 17;
    distortionTypeBox.setEnabled(true);
    distortionTypeLabel.setEnabled(true);
    twinTapeCharacterPageButton.setVisible(
        twinTapeEnabled && ! presetBrowserOpen);
    twinTapeCharacterPageButton.setTooltip(twinTapeHeadPage
        ? "Showing the independent Tape I/II Age and Shine controls; click for DIST, shared AGE, WOW and FLUTTER"
        : "Showing DIST, shared AGE, WOW and FLUTTER; click for the independent Tape I/II Age and Shine controls");
    echoOffsetSlider.setEnabled(echoOffsetEnabled);
    echoOffsetLabel.setEnabled(echoOffsetEnabled);
    echoOffsetLabel.setColour(
        juce::Label::textColourId,
        echoOffsetEnabled ? knobLabelColour
                          : knobLabelColour.darker(0.45f));
    const auto attachDial = [this](Dial* dial, const char* parameterId,
                                   const char* label,
                                   const juce::String& help,
                                   double defaultValue)
    {
        if (dial == nullptr)
            return;
        dial->attachment.reset();
        dial->parameterId = parameterId;
        dial->label.setText(label, juce::dontSendNotification);
        dial->help = help;
        dial->slider.clearCentreDetent();
        dial->slider.setDoubleClickReturnValue(true, defaultValue);
        dial->attachment = std::make_unique<SliderAttachment>(
            processor.parameters, parameterId, dial->slider);
        dial->slider.setEnabled(true);
        dial->label.setEnabled(true);
    };
    if (twinTapeHeadPage && modeDials.size() >= 4)
    {
        attachDial(modeDials[0], "twinTapeAge1", "AGE I",
                   "Tape I wear. Adds independent pitch warble, chorusing, dropouts and tape artefacts.", 20.0);
        attachDial(modeDials[1], "twinTapeShine1", "SHINE I",
                   "Tape I brightness. 100% is open; lower values progressively soften its high frequencies.", 100.0);
        attachDial(modeDials[2], "twinTapeAge2", "AGE II",
                   "Tape II wear. Its separate warble and dropouts keep both tape heads moving independently.", 20.0);
        attachDial(modeDials[3], "twinTapeShine2", "SHINE II",
                   "Tape II brightness. Use a different setting from Tape I to separate the two pitch layers.", 100.0);
    }
    else if (modeDials.size() >= 4)
    {
        attachDial(modeDials[0], "drive", "DIST",
                   "Sets input distortion before the delay. Recirculating repeats are not distorted again.", 0.0);
        if (lushPitchEnabled)
        {
            attachDial(modeDials[1], "pitchMix", "P MIX",
                       "Blends the stereo-detuned voice into the shifted voice; 50% combines them equally.", 50.0);
        }
        else
        {
            attachDial(modeDials[1], "age", "AGE",
                       "Changes the selected mode's age or character. The description follows the active mode.", 30.0);
        }
        attachDial(modeDials[2], "wow", "WOW",
                   "Slow delay-time movement and pitch drift. Available in every ordinary delay mode.", 0.0);
        attachDial(modeDials[3], "flutter", "FLUTTER",
                   "Faster, finer delay-time movement with slight stereo variation.", 0.0);
        auto* characterDial = modeDials[1];
        const bool characterEnabled = ageEnabled || lushPitchEnabled
                                   || twinTapeEnabled;
        characterDial->slider.setEnabled(characterEnabled);
        characterDial->label.setEnabled(characterEnabled);
    }

    if (highCutDial != nullptr && lowCutDial != nullptr)
    {
        highCutDial->attachment.reset();
        lowCutDial->attachment.reset();
        if (twinTapeEnabled)
        {
            attachDial(highCutDial, "twinTapeGain1", "GAIN I",
                       "Tape I level before the two heads are blended and sent into Feedback.", 0.0);
            attachDial(lowCutDial, "twinTapeGain2", "GAIN II",
                       "Tape II level before the two heads are blended and sent into Feedback.", 0.0);
            highCutDial->slider.setCentreDetent(0.0, 0.6);
            lowCutDial->slider.setCentreDetent(0.0, 0.6);
        }
        else
        {
            attachDial(highCutDial, "highCut", "HI CUT",
                       "Remove high frequencies from every echo, including the first. At maximum this filter is bypassed.", 20000.0);
            attachDial(lowCutDial, "lowCut", "LO CUT",
                       "Remove low frequencies from every echo, including the first. At minimum this filter is bypassed.", 10.0);
        }
    }
    if (pitchShiftDial != nullptr && pitchDetuneDial != nullptr)
    {
        pitchShiftDial->attachment.reset();
        pitchDetuneDial->attachment.reset();
        if (twinTapeEnabled)
        {
            pitchShiftDial->label.setText("TAPE I", juce::dontSendNotification);
            pitchShiftDial->parameterId = "twinTapeSpeed1";
            pitchShiftDial->help = "Tape I direction and speed: Reverse Fast/Normal/Slow or forward Slow/Normal/Fast.";
            pitchShiftDial->slider.clearCentreDetent();
            pitchShiftDial->slider.setDoubleClickReturnValue(true, 4.0);
            pitchShiftDial->attachment = std::make_unique<SliderAttachment>(
                processor.parameters, "twinTapeSpeed1", pitchShiftDial->slider);

            pitchDetuneDial->label.setText("TAPE II", juce::dontSendNotification);
            pitchDetuneDial->parameterId = "twinTapeSpeed2";
            pitchDetuneDial->help = "Tape II direction and speed. Different head settings create octave layers and reverse textures.";
            pitchDetuneDial->slider.clearCentreDetent();
            pitchDetuneDial->slider.setDoubleClickReturnValue(true, 4.0);
            pitchDetuneDial->attachment = std::make_unique<SliderAttachment>(
                processor.parameters, "twinTapeSpeed2", pitchDetuneDial->slider);
        }
        else if (phaserEnabled)
        {
            pitchShiftDial->label.setText("RES", juce::dontSendNotification);
            pitchShiftDial->parameterId = "phaserResonance";
            pitchShiftDial->help = "Phaser resonance. Higher settings reinforce the moving notches inside the delay feedback path.";
            pitchShiftDial->slider.clearCentreDetent();
            pitchShiftDial->slider.setDoubleClickReturnValue(true, 30.0);
            pitchShiftDial->attachment = std::make_unique<SliderAttachment>(
                processor.parameters, "phaserResonance", pitchShiftDial->slider);

            pitchDetuneDial->label.setText("STAGE", juce::dontSendNotification);
            pitchDetuneDial->parameterId = "phaserStages";
            pitchDetuneDial->help = "Number of all-pass Phaser stages: 2, 4, 6, 8, 10 or 12. More stages create more moving notches.";
            pitchDetuneDial->slider.clearCentreDetent();
            pitchDetuneDial->slider.setDoubleClickReturnValue(true, 12.0);
            pitchDetuneDial->attachment = std::make_unique<SliderAttachment>(
                processor.parameters, "phaserStages", pitchDetuneDial->slider);
        }
        else if (ghostEnabled)
        {
            pitchShiftDial->label.setText("SHIFT", juce::dontSendNotification);
            pitchShiftDial->parameterId = "ghostShift";
            pitchShiftDial->help = "Ghost shift from -1000 to +1000 Hz. Centre is neutral; small values create slow spectral movement.";
            pitchShiftDial->slider.setCentreDetent(0.0, 0.75);
            pitchShiftDial->slider.setDoubleClickReturnValue(true, 0.0);
            pitchShiftDial->attachment = std::make_unique<SliderAttachment>(
                processor.parameters, "ghostShift", pitchShiftDial->slider);

            pitchDetuneDial->label.setText("DETUNE", juce::dontSendNotification);
            pitchDetuneDial->parameterId = "ghostDetune";
            pitchDetuneDial->help = "Ghost Left/Right shift separation from 0 to 100 Hz. Higher values pan and beat more rapidly.";
            pitchDetuneDial->slider.clearCentreDetent();
            pitchDetuneDial->slider.setDoubleClickReturnValue(true, 14.0);
            pitchDetuneDial->attachment = std::make_unique<SliderAttachment>(
                processor.parameters, "ghostDetune", pitchDetuneDial->slider);
        }
        else
        {
            pitchShiftDial->label.setText("SHIFT", juce::dontSendNotification);
            pitchShiftDial->parameterId = "pitchShift";
            pitchShiftDial->help = "Pitch, Pitch Rev and Pitch Lush transposition from -24 to +24 semitones. Zero preserves the original pitch.";
            pitchShiftDial->slider.setCentreDetent(0.0, 0.15);
            pitchShiftDial->slider.setDoubleClickReturnValue(true, 0.0);
            pitchShiftDial->attachment = std::make_unique<SliderAttachment>(
                processor.parameters, "pitchShift", pitchShiftDial->slider);

            pitchDetuneDial->label.setText("DETUNE", juce::dontSendNotification);
            pitchDetuneDial->parameterId = "pitchDetune";
            pitchDetuneDial->help = "Stereo pitch offset. Positive raises Right and lowers Left; negative reverses them.";
            pitchDetuneDial->slider.setCentreDetent(0.0, 0.03);
            pitchDetuneDial->slider.setDoubleClickReturnValue(true, 0.0);
            pitchDetuneDial->attachment = std::make_unique<SliderAttachment>(
                processor.parameters, "pitchDetune", pitchDetuneDial->slider);
        }

        const bool pitchPanelEnabled = pitchEnabled || phaserEnabled
                                    || ghostEnabled || twinTapeEnabled;
        pitchShiftDial->slider.setEnabled(pitchPanelEnabled);
        pitchShiftDial->label.setEnabled(pitchPanelEnabled);
        pitchDetuneDial->slider.setEnabled(pitchPanelEnabled);
        pitchDetuneDial->label.setEnabled(pitchPanelEnabled);
    }

    updateTimeControlState();

    updateContextSensitiveHelp(selectedMode);

    if (selectedMode == 2)
        information.setText("MODE: TAPE I | Warm two-stage head loss, saturation and natural drift\nLonger delay times darken and move more; Age increases wear and compression.",
                            juce::dontSendNotification);
    else if (selectedMode == 3)
        information.setText("MODE: BRIGADE | Dark two-stage bandwidth loss, companding and clock texture\nAge increases grain, noise and saturation; long delays lose more high end.",
                            juce::dontSendNotification);
    else if (selectedMode == 4)
        information.setText("MODE: HIFI | Bright reel-to-reel tone with gentle saturation and drift\nAge adds restrained bandwidth wear and signal-tracking asperity.",
                            juce::dontSendNotification);
    else if (selectedMode == 5)
        information.setText("MODE: GHOST | Analogue-style delay with dedicated Hertz frequency shifting\nSHIFT moves both channels; DETUNE separates Left and Right, accumulating through feedback.",
                            juce::dontSendNotification);
    else if (selectedMode == 6)
        information.setText("MODE: PITCH | Dual overlapping read heads transpose every echo\nSHIFT covers -24 to +24 semitones; AGE reduces floating-converter bit depth.",
                            juce::dontSendNotification);
    else if (selectedMode == 7)
        information.setText("MODE: PITCH REV | Reversed overlapping grains are transposed inside feedback\nSHIFT/DETUNE set pitch; AGE reduces floating-converter bit depth.",
                            juce::dontSendNotification);
    else if (selectedMode == 8)
        information.setText("MODE: LOFI | Clearly worn tape with dark bandwidth, uneven level, hiss and asymmetric grit\nBuilt-in chaotic pitch motion remains active; AGE and MOD intensify its instability.",
                            juce::dontSendNotification);
    else if (selectedMode == 9)
        information.setText("MODE: CRYSTAL | Pristine bright digital repeats with a nearly colourless limiter\nMOD uses smoothed triangle motion; AGE and the EQ stay outside the feedback loop.",
                            juce::dontSendNotification);
    else if (selectedMode == 10)
        information.setText("MODE: PHASER | Stereo phasing lives inside the delay feedback loop\nRES controls resonance; STAGE selects 2/4/6/8/10/12 stages; AGE is unused.",
                            juce::dontSendNotification);
    else if (selectedMode == 11)
        information.setText("MODE: PITCH LUSH | Shifted and lush stereo-detuned echoes share one delay time\nSHIFT/DETUNE shape both voices; P MIX blends detuned into pitch-shifted.",
                            juce::dontSendNotification);
    else if (selectedMode == 12)
        information.setText("MODE: CHROMATIC | Clean modern tape motion with brighter heads and controlled feedback\nAGE now moves clearly from open and clean to dark, compressed head wear while transport drift stays restrained.",
                            juce::dontSendNotification);
    else if (selectedMode == 13)
        information.setText("MODE: ANALOGUE | High-bandwidth, low-noise BBD-style echoes with thicker asymmetric saturation\nAGE narrows the bandwidth, raises clock texture and drives the analogue stage harder.",
                            juce::dontSendNotification);
    else if (selectedMode == 14)
        information.setText("MODE: CHAOTIC | Smoothly wandering delay intervals and pitch ratios create unstable, forward-moving echoes\nMOD RATE changes the motion speed; feedback turns the irregular grains into evolving patterns.",
                            juce::dontSendNotification);
    else if (selectedMode == 15)
        information.setText("MODE: PURE REVERSE | Overlapping reverse grains preserve the original pitch while playing every repeat backwards\nDelay Time controls the grain length; feedback builds smooth reverse layers without transposition.",
                            juce::dontSendNotification);
    else if (selectedMode == 16)
        information.setText("MODE: CHAOTIC REV | Reverse grains wander through independent delay lengths and pitch ratios\nMOD RATE changes the random motion speed; feedback creates tumbling, unstable reverse textures.",
                            juce::dontSendNotification);
    else if (selectedMode == 17)
        information.setText("MODE: FORWARD REV | Forward repeats are heard while a separately diffused reverse path regenerates\nDIFF AMT and DIFF SIZE shape the reverse feedback cloud without blurring the direct forward echo.",
                            juce::dontSendNotification);
    else if (selectedMode == 18)
        information.setText("MODE: TAPE II | Inertial transport glides through Time changes with continuous acceleration and deceleration\nWarm two-stage heads, magnetic asymmetry and bounded varispeed create a smoother, more fluid tape echo.",
                            juce::dontSendNotification);
    else if (selectedMode == 19)
        information.setText("MODE: COLOUR | Independent virtual-clock heads follow Left and Right Time with a continuous Doppler glide\nAGE morphs the exact clean delay into smoother companding, softer bandwidth and restrained asymmetric colour.",
                            juce::dontSendNotification);
    else if (selectedMode == 20)
        information.setText("MODE: TWIN TAPE | Two independent variable-speed tape heads can run forward or reverse at half, normal or double speed\nAGE, SHINE and GAIN are separate per head; Freeze keeps all head controls live over the captured loops.",
                            juce::dontSendNotification);
    else if (selectedMode == 21)
        information.setText("MODE: GRAIN | Four overlapping forward grains turn each delay into a smooth pitch-moving texture\nSHIFT/DETUNE set pitch; AGE shortens and scatters the grains while MOD controls their moving spread.",
                            juce::dontSendNotification);
    else
        information.setText("MODE: DIGITAL | Clean, stable repeats with click-resistant time changes\nAGE reduces floating-converter bit depth without adding analogue drift or wear.",
                            juce::dontSendNotification);

    lookAndFeel.setPopupBorderColour(modeColourForId(selectedMode));
    repaint();
}

void AfterglowAudioProcessorEditor::paint(juce::Graphics& g)
{
    const int designHeight = activeDesignHeight();
    g.fillAll(juce::Colour(0xff1b2026));
    const float scale = juce::jmin(
        static_cast<float>(getWidth()) / static_cast<float>(designWidth),
        static_cast<float>(getHeight()) / static_cast<float>(designHeight));
    const float offsetX = 0.5f * (static_cast<float>(getWidth())
        - static_cast<float>(designWidth) * scale);
    const float offsetY = 0.5f * (static_cast<float>(getHeight())
        - static_cast<float>(designHeight) * scale);
    g.saveState();
    g.addTransform(juce::AffineTransform::scale(scale).translated(offsetX, offsetY));

    const auto modeColour = modeColourForId(modeBox.getSelectedId());

    const float fullColourAmount = themeDial != nullptr
                                 ? juce::jlimit(0.0f, 1.0f,
                                     static_cast<float>(themeDial->slider.getValue() * 0.01))
                                 : 1.0f;
    const auto darkSurroundColour = juce::Colour(0xff24292f);
    const auto surroundColour = darkSurroundColour.interpolatedWith(
        modeColour, fullColourAmount);
    g.setColour(surroundColour);
    g.fillRoundedRectangle(8.0f, 52.0f,
                           static_cast<float>(designWidth - 16), 216.0f, 12.0f);

    const auto drawControlPanel = [&g](float x, float y, float width, float height)
    {
        g.setColour(juce::Colour(0xff171b20));
        g.fillRoundedRectangle(x, y, width, height, 8.0f);
        g.setColour(knobTrackColour);
        g.drawRoundedRectangle(x, y, width, height, 8.0f, 1.0f);
    };

    if (presetBrowserOpen)
    {
        drawControlPanel(18.0f, 62.0f, 890.0f, 196.0f);
        g.setColour(knobTrackColour);
        g.drawLine(288.0f, 70.0f, 288.0f, 250.0f, 1.0f);
    }
    else
    {
        drawControlPanel(18.0f, 62.0f, 188.0f, 170.0f);
        drawControlPanel(212.0f, 62.0f, 206.0f, 81.0f);
        drawControlPanel(212.0f, 149.0f, 206.0f, 83.0f);
        drawControlPanel(422.0f, 62.0f, 66.0f, 170.0f);
        drawControlPanel(492.0f, 62.0f, 66.0f, 170.0f);
        drawControlPanel(562.0f, 62.0f, 66.0f, 170.0f);
        drawControlPanel(632.0f, 62.0f, 66.0f, 170.0f);
        drawControlPanel(702.0f, 62.0f, 136.0f, 170.0f);
        drawControlPanel(842.0f, 62.0f, 66.0f, 170.0f);
    }

    const juce::Rectangle<float> modeStrip(
        18.0f, 53.0f, static_cast<float>(designWidth - 36), 7.0f);
    g.setColour(modeColour);
    g.fillRoundedRectangle(modeStrip, 3.5f);

    const auto drawTimingGroup = [&g](const juce::String& text, float y,
                                      float leftCentre, float rightCentre)
    {
        const float centre = (leftCentre + rightCentre) * 0.5f;
        const float titleWidth = text.containsChar('|') ? 62.0f : 42.0f;
        constexpr float channelWidth = 12.0f;

        g.setColour(knobTrackColour);
        const float leftLineStart = leftCentre + channelWidth * 0.65f;
        const float leftLineEnd = centre - titleWidth * 0.5f - 3.0f;
        const float rightLineStart = centre + titleWidth * 0.5f + 3.0f;
        const float rightLineEnd = rightCentre - channelWidth * 0.65f;
        if (leftLineEnd > leftLineStart)
            g.drawLine(leftLineStart, y, leftLineEnd, y, 1.0f);
        if (rightLineEnd > rightLineStart)
            g.drawLine(rightLineStart, y, rightLineEnd, y, 1.0f);
        g.setColour(knobLabelColour);
        g.setFont(juce::Font(juce::FontOptions(10.5f, juce::Font::bold)));
        g.drawText(text, juce::roundToInt(centre - titleWidth * 0.5f),
                   juce::roundToInt(y - 7.0f), juce::roundToInt(titleWidth), 14,
                   juce::Justification::centred);
    };

    if (! presetBrowserOpen)
    {
        if (modeBox.getSelectedId() != 20)
        {
            g.setColour(knobLabelColour);
            g.setFont(juce::Font(juce::FontOptions(7.6f, juce::Font::bold)));
            g.drawText("MULTI", 151, 211, 36, 12,
                       juce::Justification::centredRight);
        }

        if (! multiTapButton.getToggleState()
            || modeBox.getSelectedId() == 20)
        {
            const bool leftSynced = syncLeftButton.getToggleState();
            const bool rightSynced = linkButton.getToggleState()
                                   ? leftSynced : syncRightButton.getToggleState();
            const juce::String timingTitle = leftSynced == rightSynced
                ? (leftSynced ? "NOTE" : "TIME")
                : (leftSynced ? "NOTE | TIME" : "TIME | NOTE");
            drawTimingGroup(timingTitle, 72.0f, 72.0f, 152.0f);

            g.setColour(knobTrackColour);
            g.drawLine(79.0f, 150.0f, 84.0f, 150.0f, 1.0f);
            g.drawLine(140.0f, 150.0f, 145.0f, 150.0f, 1.0f);
            g.setColour(knobLabelColour);
            g.setFont(juce::Font(juce::FontOptions(8.8f, juce::Font::bold)));
            g.drawText("TEMPO SYNC", 84, 143, 56, 14,
                       juce::Justification::centred);
        }
        else
        {
            g.setFont(juce::Font(juce::FontOptions(7.8f, juce::Font::bold)));
            g.drawText("SYNC", 36, 211, 31, 12,
                       juce::Justification::centredLeft);
            g.setFont(juce::Font(juce::FontOptions(8.3f, juce::Font::bold)));
            g.drawText("TIME", 29, 67, 34, 12, juce::Justification::centred);
            g.drawText("PAN", 75, 67, 34, 12, juce::Justification::centred);
            g.drawText("VOL", 121, 67, 34, 12, juce::Justification::centred);
            g.drawText("ON", 164, 67, 34, 12, juce::Justification::centred);
        }
    }
    if (logoImage.isValid())
        g.drawImageWithin(logoImage, 22, 5, 280, 43,
                          juce::RectanglePlacement::xLeft
                              | juce::RectanglePlacement::yMid
                              | juce::RectanglePlacement::onlyReduceInSize,
                          false);
    g.restoreState();
}

void AfterglowAudioProcessorEditor::resized()
{
    const int designHeight = activeDesignHeight();
    content.setTransform(juce::AffineTransform());
    content.setBounds(0, 0, designWidth, designHeight);
    const float scale = juce::jmin(
        static_cast<float>(getWidth()) / static_cast<float>(designWidth),
        static_cast<float>(getHeight()) / static_cast<float>(designHeight));
    const float offsetX = 0.5f * (static_cast<float>(getWidth())
        - static_cast<float>(designWidth) * scale);
    const float offsetY = 0.5f * (static_cast<float>(getHeight())
        - static_cast<float>(designHeight) * scale);
    content.setTransform(
        juce::AffineTransform::scale(scale).translated(offsetX, offsetY));

    constexpr int knobWidth = 60;
    constexpr int labelWidth = 70;

    const auto placeDial = [this, knobWidth, labelWidth](size_t index, int knobX, int labelY)
    {
        if (index >= dials.size())
            return;

        dials[index]->label.setBounds(knobX - (labelWidth - knobWidth) / 2,
                                      labelY, labelWidth, 13);
        dials[index]->slider.setBounds(knobX, labelY + 13, knobWidth, 57);
    };

    if (dials.size() > 0)
    {
        dials[0]->slider.setBounds(42, 80, knobWidth, 57);
        dials[0]->label.setBounds(52, 67, 40, 12);
    }
    if (dials.size() > 1)
    {
        dials[1]->slider.setBounds(122, 80, knobWidth, 57);
        dials[1]->label.setBounds(132, 67, 40, 12);
    }
    for (size_t tap = 0; tap < multiTapCount; ++tap)
    {
        const int rowY = 81 + static_cast<int>(tap) * 32;
        if (tapTimeDials[tap] != nullptr)
        {
            tapTimeDials[tap]->slider.setBounds(32, rowY, 28, 28);
            tapTimeDials[tap]->label.setBounds(62, rowY + 9, 9, 10);
        }
        if (tapPanDials[tap] != nullptr)
        {
            tapPanDials[tap]->slider.setBounds(78, rowY, 28, 28);
            tapPanDials[tap]->label.setBounds(108, rowY + 9, 9, 10);
        }
        if (tapLevelDials[tap] != nullptr)
        {
            tapLevelDials[tap]->slider.setBounds(124, rowY, 28, 28);
            tapLevelDials[tap]->label.setBounds(154, rowY + 9, 9, 10);
        }
        tapEnableButtons[tap].setBounds(170, rowY + 3, 22, 22);
    }

    placeDial(2, 215, 67);   // Feedback
    placeDial(3, 285, 67);   // Cross
    placeDial(4, 355, 67);   // Spread
    placeDial(5, 215, 154);  // Duck
    placeDial(6, 285, 154);  // Duck attack
    placeDial(7, 355, 154);  // Duck release
    placeDial(8, 425, 67);   // Diffusion amount
    placeDial(9, 425, 154);  // Diffusion size
    placeDial(10, 495, 67);  // Modulation amount
    placeDial(11, 495, 154); // Modulation rate
    for (const size_t index : { size_t(8), size_t(9),
                                size_t(10), size_t(11) })
        if (index < dials.size())
            dials[index]->label.setTopLeftPosition(
                dials[index]->label.getX() + 3,
                dials[index]->label.getY());
    placeDial(20, 565, 67);  // Pitch shift
    placeDial(21, 565, 154); // Pitch detune
    placeDial(14, 635, 67);  // High Cut
    placeDial(13, 635, 154); // Low Cut
    placeDial(16, 705, 67);  // Distortion amount
    placeDial(17, 775, 67);  // Age
    placeDial(18, 705, 154); // Wow
    placeDial(19, 775, 154); // Flutter
    placeDial(12, 845, 67);  // Dry / Wet
    placeDial(15, 845, 154); // Output

    logoMenuButton.setBounds(22, 5, 280, 43);
    versionLabel.setBounds(designWidth - 65, 2, 66, 12);
    modeLabel.setBounds(204, 17, 38, 20);
    modeBox.setBounds(242, 13, 100, 26);
    distortionTypeLabel.setBounds(348, 17, 54, 20);
    distortionTypeBox.setBounds(402, 13, 106, 26);
    freezeButton.setBounds(516, 13, 26, 26);
    freezeModeButton.setBounds(546, 17, 18, 18);
    freezeModeLabel.setBounds(563, 15, 64, 20);
    currentPresetLabel.setBounds(632, 13, 94, 26);
    previousPresetButton.setBounds(726, 13, 21, 26);
    nextPresetButton.setBounds(747, 13, 21, 26);
    presetBrowserButton.setBounds(774, 13, 26, 26);
    linkButton.setBounds(30, 176, 84, 24);
    pingPongButton.setBounds(110, 176, 84, 24);
    syncLeftButton.setBounds(63, 141, 18, 18);
    syncRightButton.setBounds(143, 141, 18, 18);
    multiTapSyncButton.setBounds(20, 208, 18, 18);
    multiTapButton.setBounds(186, 208, 18, 18);
    twinTapeCharacterPageButton.setBounds(762, 137, 18, 18);
    if (themeDial != nullptr)
    {
        themeDial->label.setBounds(designWidth - 118, 2, 58, 12);
        themeDial->slider.setBounds(designWidth - 104, 16, 30, 30);
    }
    if (browserMixDial != nullptr)
    {
        browserMixDial->label.setBounds(designWidth - 118, 2, 58, 12);
        browserMixDial->slider.setBounds(designWidth - 104, 16, 30, 30);
    }
    mixLockButton.setBounds(891, 66, 14, 14);
    bypassButton.setBounds(designWidth - 49, 14, 34, 34);
    presetFolderLabel.setBounds(28, 66, 252, 14);
    presetListLabel.setBounds(298, 66, 560, 14);
    presetFolderTree.setBounds(26, 82, 256, 168);
    presetList.setBounds(296, 82, 564, 168);
    presetSaveButton.setBounds(870, 85, 26, 26);
    presetSaveAsButton.setBounds(870, 130, 26, 26);
    presetOpenFolderButton.setBounds(870, 175, 26, 26);
    information.setBounds(25, 236, 670, 28);
    echoOffsetLabel.setBounds(704, 233, 116, 11);
    echoOffsetSlider.setBounds(704, 244, 116, 20);
    randomButton.setBounds(836, 236, 26, 26);
    sequencerButton.setBounds(presetBrowserOpen ? 865 : 872,
                              presetBrowserOpen ? 220 : 236,
                              36, 26);
    if (sequencerPanel != nullptr)
        sequencerPanel->setBounds(8, collapsedDesignHeight,
                                  designWidth - 16,
                                  expandedDesignHeight
                                      - collapsedDesignHeight - 8);

    if (isShowing() && ! presetBrowserOpen
        && getWidth() != observedEditorWidth)
    {
        const int scalePercent = juce::jlimit(
            75, 300, juce::roundToInt(
                100.0f * static_cast<float>(getWidth())
                    / static_cast<float>(designWidth)));
        information.setText(
            "GUI SIZE: " + juce::String(scalePercent) + "%\n"
                + juce::String(getWidth()) + " x "
                + juce::String(getHeight()) + " px",
            juce::dontSendNotification);
    }
}

void AfterglowAudioProcessorEditor::timerCallback()
{
    const bool leftKeyDown = isShowing()
        && juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::leftKey);
    const bool rightKeyDown = isShowing()
        && juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::rightKey);

    if (leftKeyDown && ! previousPresetKeyDown)
        stepPreset(-1);
    if (rightKeyDown && ! nextPresetKeyDown)
        stepPreset(1);

    previousPresetKeyDown = leftKeyDown;
    nextPresetKeyDown = rightKeyDown;

    updateSequencerTargetHighlights();
    if (sequencerOpen && sequencerPanel != nullptr)
        sequencerPanel->timerTick();
    updateAnimatedParameterDisplays();

    const auto processorPresetFile = processor.getCurrentPresetFile();
    if (processorPresetFile != currentPresetFile)
    {
        const auto root = AfterglowAudioProcessor::getPresetsDirectory();
        const auto parent = processorPresetFile.getParentDirectory();
        if (processorPresetFile != juce::File()
            && (parent == root || parent.isAChildOf(root)))
        {
            currentPresetFile = processorPresetFile;
            selectedPresetFolder = parent;
        }
        else
        {
            currentPresetFile = juce::File();
        }

        if (presetBrowserOpen)
            rebuildPresetFolderTree();
        else
            updatePresetNavigationDisplay();
    }

    const int currentWidth = getWidth();
    const double currentTheme = themeDial != nullptr
                              ? themeDial->slider.getValue() : 100.0;
    if (currentWidth != observedEditorWidth
        || std::abs(currentTheme - observedThemeValue) > 0.0001)
    {
        observedEditorWidth = currentWidth;
        observedThemeValue = currentTheme;
        stableSettingsTicks = 0;
        settingsDirty = true;
    }
    else if (settingsDirty && ++stableSettingsTicks >= 5)
    {
        saveInterfaceSettings();
        stableSettingsTicks = 0;
        settingsDirty = false;
    }
}

void AfterglowAudioProcessorEditor::saveInterfaceSettings() const
{
    const auto directory = AfterglowAudioProcessor::getSettingsDirectory();
    if (! directory.isDirectory() && directory.createDirectory().failed())
        return;

    const int scalePercent = juce::jlimit(
        75, 300, juce::roundToInt(100.0f * static_cast<float>(getWidth())
                                  / static_cast<float>(designWidth)));
    const double themePercent = themeDial != nullptr
        ? juce::jlimit(0.0, 100.0, themeDial->slider.getValue()) : 100.0;
    juce::String text;
    text << "; Afterglow interface settings\r\n"
         << "[GUI]\r\n"
         << "ScalePercent=" << scalePercent << "\r\n"
         << "ThemePercent=" << juce::String(themePercent, 1) << "\r\n"
         << "SequencerOpen=" << (sequencerOpen ? 1 : 0) << "\r\n"
         << "SequencerAnimate="
         << (sequencerPanel != nullptr && sequencerPanel->getAnimate()
                 ? 1 : 0)
         << "\r\n";
    directory.getChildFile("Settings.ini").replaceWithText(text);
}
