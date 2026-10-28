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
const juce::Colour powerLedColour(0xff82d16a);
const juce::Colour presetAccentColour(0xff72b9a9);
constexpr auto presetFileExtension = ".agpreset";
constexpr auto sequencerTargetDragPrefix = "afterglow-sequencer-target:";
constexpr int delayModeCount = 18;
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
constexpr std::array<const char*, 16> distortionSummaries
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
    "Clean flat-threshold clipping without added noise or tone filtering"
};
constexpr std::array<const char*, 16> distortionDetails
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
    "DIST lowers the clipping threshold for progressively harder, squared-off transients."
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
        default: return juce::Colour(0xff1479b8); // Digital
    }
}

struct InterfaceSettings
{
    int scalePercent = 100;
    double themePercent = 100.0;
    bool sequencerOpen = false;
    double sequencerHue = juce::Colour(0xff34d6c6).getHue();
    double sequencerSaturation = 0.72;
    double sequencerLuminance = 0.82;
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
            settings.scalePercent = juce::jlimit(75, 200, value.getIntValue());
        else if (key.equalsIgnoreCase("ThemePercent"))
            settings.themePercent = juce::jlimit(0.0, 100.0,
                                                  static_cast<double>(value.getFloatValue()));
        else if (key.equalsIgnoreCase("SequencerOpen"))
            settings.sequencerOpen = value.getIntValue() != 0;
        else if (key.equalsIgnoreCase("SequencerHue"))
            settings.sequencerHue = juce::jlimit(
                0.0, 1.0, static_cast<double>(value.getFloatValue()));
        else if (key.equalsIgnoreCase("SequencerSaturation"))
            settings.sequencerSaturation = juce::jlimit(
                0.0, 1.0, static_cast<double>(value.getFloatValue()));
        else if (key.equalsIgnoreCase("SequencerLuminance"))
            settings.sequencerLuminance = juce::jlimit(
                0.2, 1.0, static_cast<double>(value.getFloatValue()));
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
                   std::function<void(const juce::File&)> selectionCallback,
                   int depth = 0)
        : directory(std::move(directoryToUse)),
          onSelected(std::move(selectionCallback))
    {
        if (depth >= 32)
            return;

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
                addSubItem(new FolderTreeItem(child, onSelected, depth + 1));
    }

    bool mightContainSubItems() override
    {
        return getNumSubItems() > 0;
    }

    juce::String getUniqueName() const override
    {
        return directory.getFullPathName();
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

        const auto folderBounds = juce::Rectangle<float>(
            2.0f, static_cast<float>(height) * 0.5f - 5.0f, 13.0f, 10.0f);
        g.setColour(isSelected() ? knobIndicatorColour
                                 : knobLabelColour.brighter(0.12f));
        g.fillRoundedRectangle(folderBounds, 1.5f);
        g.fillRect(folderBounds.getX() + 1.5f, folderBounds.getY() - 2.0f,
                   5.0f, 3.0f);
        g.setColour(isSelected() ? juce::Colours::white
                                 : knobValueColour);
        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        g.drawFittedText(directory == AfterglowAudioProcessor::getPresetsDirectory()
                             ? "Presets" : directory.getFileName(),
                         21, 0, juce::jmax(0, width - 24), height,
                         juce::Justification::centredLeft, 1);
    }

    void itemSelectionChanged(bool isNowSelected) override
    {
        if (isNowSelected && onSelected)
            onSelected(directory);
        repaintItem();
    }

    FolderTreeItem* findItemForDirectory(const juce::File& target)
    {
        if (directory == target)
            return this;

        for (int index = 0; index < getNumSubItems(); ++index)
            if (auto* child = dynamic_cast<FolderTreeItem*>(getSubItem(index)))
                if (auto* match = child->findItemForDirectory(target))
                    return match;
        return nullptr;
    }

private:
    juce::File directory;
    std::function<void(const juce::File&)> onSelected;
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
                steps[static_cast<size_t>(step)]
                    = processor.parameters.getParameter(
                        afterglow::sequencer::stepParameterId(step));
            bipolar = processor.parameters.getRawParameterValue(
                "sequencerBipolar");
            start = processor.parameters.getRawParameterValue("sequencerStart");
            end = processor.parameters.getRawParameterValue("sequencerEnd");
            peak = processor.parameters.getRawParameterValue("sequencerPeak");
            move = processor.parameters.getRawParameterValue("sequencerMove");
            attack = processor.parameters.getRawParameterValue(
                "sequencerAttack");
            release = processor.parameters.getRawParameterValue(
                "sequencerRelease");
            setMouseCursor(juce::MouseCursor::CrosshairCursor);
            setTooltip("Click or drag to set the steps; right-click for pattern tools");
        }

        void setAccent(juce::Colour colour)
        {
            accent = colour;
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
            const bool isBipolar = bipolar != nullptr
                                 && bipolar->load() >= 0.5f;
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
                value = afterglow::sequencer::applyPeakMove(
                    value, patternMinimum, patternMaximum,
                    peakValue, moveValue, isBipolar);

            const auto displayedValue = [isBipolar](float canonical)
            {
                return isBipolar ? juce::jlimit(0.0f, 1.0f, canonical)
                    : juce::jlimit(0.0f, 1.0f, 2.0f * canonical - 1.0f);
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
                const float displayed = displayedValue(
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
                    graphics.fillRect(x + 1.0f,
                        graph.getBottom() - graph.getHeight() * displayed,
                        juce::jmax(1.0f, columnWidth - 2.0f),
                        graph.getHeight() * displayed);
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
                    const float y = graph.getBottom()
                        - graph.getHeight() * displayedValue(value);
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
        }

        void mouseDown(const juce::MouseEvent& event) override
        {
            if (event.mods.isPopupMenu())
            {
                showPatternMenu();
                return;
            }
            lastEditedCell = -1;
            editAt(event.position);
        }

        void mouseDrag(const juce::MouseEvent& event) override
        {
            editAt(event.position);
        }

        void mouseUp(const juce::MouseEvent&) override
        {
            lastEditedCell = -1;
        }

        void applyWaveform(afterglow::sequencer::Waveform waveform)
        {
            const bool isBipolar = bipolar != nullptr
                                 && bipolar->load() >= 0.5f;
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

        std::function<void()> onSaveRequested;

    private:
        juce::Rectangle<float> graphBounds() const
        {
            return getLocalBounds().toFloat().reduced(5.0f)
                .withTrimmedTop(12.0f);
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
            const bool isBipolar = bipolar != nullptr
                                 && bipolar->load() >= 0.5f;
            const float displayed = juce::jlimit(
                0.0f, 1.0f, (graph.getBottom() - position.y)
                                  / graph.getHeight());
            const float canonical = isBipolar ? displayed
                                              : 0.5f + 0.5f * displayed;
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
                    const bool isBipolar = grid.bipolar != nullptr
                        && grid.bipolar->load() >= 0.5f;
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

        AfterglowAudioProcessor& processor;
        LookAndFeel& sharedPopupLookAndFeel;
        LookAndFeel& compactPopupLookAndFeel;
        std::array<juce::RangedAudioParameter*,
                   afterglow::sequencer::stepCount> steps {};
        std::atomic<float>* bipolar = nullptr;
        std::atomic<float>* start = nullptr;
        std::atomic<float>* end = nullptr;
        std::atomic<float>* peak = nullptr;
        std::atomic<float>* move = nullptr;
        std::atomic<float>* attack = nullptr;
        std::atomic<float>* release = nullptr;
        juce::Colour accent { controlAccentColour };
        int lastEditedCell = -1;
        inline static std::array<float,
                                 afterglow::sequencer::stepCount> clipboard {};
        inline static bool clipboardValid = false;
    };

    class TargetList final : public juce::Component,
                             public juce::DragAndDropTarget,
                             public juce::SettableTooltipClient
    {
    public:
        explicit TargetList(AfterglowAudioProcessor& audioProcessor)
            : processor(audioProcessor)
        {
            for (int slot = 0; slot < afterglow::sequencer::targetSlotCount;
                 ++slot)
            {
                choices[static_cast<size_t>(slot)]
                    = processor.parameters.getParameter(
                        afterglow::sequencer::targetParameterId(slot));
                enabled[static_cast<size_t>(slot)]
                    = processor.parameters.getParameter(
                        afterglow::sequencer::targetEnabledParameterId(slot));
            }
            setTooltip("Drop an Afterglow knob label here. Click the dot to bypass a target or the cross to remove it.");
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
            const auto slots = assignedSlots();
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
                graphics.setColour(isEnabled ? knobValueColour
                                             : knobLabelColour.darker(0.35f));
                graphics.setFont(juce::FontOptions(10.5f,
                    isEnabled ? juce::Font::bold : juce::Font::plain));
                graphics.drawFittedText(afterglow::sequencer::targetName(
                        choiceForSlot(slot)), 19, y + 1, getWidth() - 42,
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
            if (firstVisibleRow > 0)
            {
                graphics.setColour(controlAccentColour);
                graphics.drawText("▲", getWidth() - 18, 2, 14, 14,
                                  juce::Justification::centred);
            }
            if (firstVisibleRow + visibleRows < static_cast<int>(slots.size()))
            {
                graphics.setColour(controlAccentColour);
                graphics.drawText("▼", getWidth() - 18, getHeight() - 15,
                                  14, 13, juce::Justification::centred);
            }
        }

        void mouseDown(const juce::MouseEvent& event) override
        {
            if (event.y < 20)
                return;
            const int row = (event.y - 20) / rowHeight + firstVisibleRow;
            const auto slots = assignedSlots();
            if (! juce::isPositiveAndBelow(row,
                                           static_cast<int>(slots.size())))
                return;
            const int slot = slots[static_cast<size_t>(row)];
            if (event.x >= getWidth() - 24)
                setParameterActualValue(choices[static_cast<size_t>(slot)], 0.0f);
            else
                setParameterActualValue(enabled[static_cast<size_t>(slot)],
                                        enabledForSlot(slot) ? 0.0f : 1.0f);
            repaint();
        }

        void mouseWheelMove(const juce::MouseEvent&,
                            const juce::MouseWheelDetails& wheel) override
        {
            const auto slots = assignedSlots();
            const int visibleRows = juce::jmax(1, (getHeight() - 20) / rowHeight);
            const int maximumFirst = juce::jmax(
                0, static_cast<int>(slots.size()) - visibleRows);
            firstVisibleRow = juce::jlimit(
                0, maximumFirst,
                firstVisibleRow + (wheel.deltaY > 0.0f ? -1 : 1));
            repaint();
        }

    private:
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

        std::vector<int> assignedSlots() const
        {
            std::vector<int> result;
            for (int slot = 0; slot < afterglow::sequencer::targetSlotCount;
                 ++slot)
                if (choiceForSlot(slot) > 0)
                    result.push_back(slot);
            return result;
        }

        AfterglowAudioProcessor& processor;
        std::array<juce::RangedAudioParameter*,
                   afterglow::sequencer::targetSlotCount> choices {};
        std::array<juce::RangedAudioParameter*,
                   afterglow::sequencer::targetSlotCount> enabled {};
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
          targets(audioProcessor)
    {
        setLookAndFeel(&sharedLookAndFeel);
        addMouseListener(this, true);
        addAndMakeVisible(grid);
        addAndMakeVisible(targets);
        const std::array<juce::Component*, 23> components
        {{ &hostSync, &enabledButton, &bipolarButton,
           &waveformBox, &userSequenceBox, &directionBox,
           &rateSlider, &startSlider, &endSlider, &attackSlider,
           &releaseSlider, &peakSlider, &moveSlider, &colourSlider,
           &saturationSlider, &luminanceSlider,
           &attackLabel, &releaseLabel, &peakLabel, &moveLabel,
           &nudgeLeft, &nudgeRight, &rangeLinkButton }};
        for (auto* component : components)
            addAndMakeVisible(component);

        hostSync.setButtonText("HOST SYNC");
        enabledButton.setButtonText("ON");
        bipolarButton.setButtonText("BIPOLAR");
        for (auto* button : { &hostSync, &enabledButton, &bipolarButton })
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
        nudgeLeft.setName("Nudge pattern left");
        nudgeRight.setName("Nudge pattern right");
        nudgeLeft.setClickingTogglesState(false);
        nudgeRight.setClickingTogglesState(false);
        nudgeLeft.getProperties().set("presetArrowDirection", -1);
        nudgeRight.getProperties().set("presetArrowDirection", 1);
        rangeLinkButton.setButtonText({});
        rangeLinkButton.setClickingTogglesState(true);
        rangeLinkButton.getProperties().set("rangeLinkIcon", true);
        rangeLinkButton.getProperties().set("activeColour",
                                             controlAccentColour.toString());
        nudgeLeft.onClick = [this] { grid.nudge(-1); };
        nudgeRight.onClick = [this] { grid.nudge(1); };

        hostSync.setTooltip("Synchronise the sequencer clock to the host tempo");
        enabledButton.setTooltip("Enable or disable sequencer modulation");
        bipolarButton.setTooltip("Switch the steps between unipolar and bipolar modulation");
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
        peakSlider.setTooltip("Expand or compress the sequence around its peak values");
        moveSlider.setTooltip("Move the complete sequence up or down");
        nudgeLeft.setTooltip("Move the complete pattern one step left");
        nudgeRight.setTooltip("Move the complete pattern one step right");

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
        configureTargetLabel(attackLabel, "ATTACK", "sequencerAttack");
        configureTargetLabel(releaseLabel, "RELEASE", "sequencerRelease");
        configureTargetLabel(peakLabel, "PEAK", "sequencerPeak");
        configureTargetLabel(moveLabel, "MOVE", "sequencerMove");

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
        bipolarAttachment = std::make_unique<
            juce::AudioProcessorValueTreeState::ButtonAttachment>(
                processor.parameters, "sequencerBipolar", bipolarButton);
        rangeLinkAttachment = std::make_unique<
            juce::AudioProcessorValueTreeState::ButtonAttachment>(
                processor.parameters, "sequencerRangeLink", rangeLinkButton);

        configureKnob(rateSlider, 0.0, 13.0, 1.0);
        configureKnob(startSlider, 1.0, 31.0, 1.0);
        configureKnob(endSlider, 2.0, 32.0, 1.0);
        configureKnob(attackSlider, 0.0, 1.0, 0.001);
        configureKnob(releaseSlider, 0.0, 1.0, 0.001);
        configureVerticalSlider(peakSlider, -1.0, 1.0, 0.001);
        configureVerticalSlider(moveSlider, -1.0, 1.0, 0.001);
        peakSlider.setCentreDetent(0.0, 0.03);
        moveSlider.setCentreDetent(0.0, 0.03);
        colourSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        colourSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        colourSlider.setRange(0.0, 1.0, 0.001);
        colourSlider.setValue(juce::Colour(0xff34d6c6).getHue(),
                              juce::dontSendNotification);
        colourSlider.onValueChange = [this] { updateAccent(); };
        colourSlider.setDoubleClickReturnValue(
            true, juce::Colour(0xff34d6c6).getHue());
        saturationSlider.setSliderStyle(
            juce::Slider::RotaryHorizontalVerticalDrag);
        saturationSlider.setTextBoxStyle(
            juce::Slider::NoTextBox, false, 0, 0);
        saturationSlider.setRange(0.0, 1.0, 0.001);
        saturationSlider.setValue(0.72, juce::dontSendNotification);
        saturationSlider.onValueChange = [this] { updateAccent(); };
        saturationSlider.setDoubleClickReturnValue(true, 0.72);
        luminanceSlider.setSliderStyle(
            juce::Slider::RotaryHorizontalVerticalDrag);
        luminanceSlider.setTextBoxStyle(
            juce::Slider::NoTextBox, false, 0, 0);
        luminanceSlider.setRange(0.2, 1.0, 0.001);
        luminanceSlider.setValue(0.82, juce::dontSendNotification);
        luminanceSlider.onValueChange = [this] { updateAccent(); };
        luminanceSlider.setDoubleClickReturnValue(true, 0.82);

        rateAttachment = makeSliderAttachment("sequencerRate", rateSlider);
        startAttachment = makeSliderAttachment("sequencerStart", startSlider);
        endAttachment = makeSliderAttachment("sequencerEnd", endSlider);
        attackAttachment = makeSliderAttachment("sequencerAttack", attackSlider);
        releaseAttachment = makeSliderAttachment("sequencerRelease", releaseSlider);
        peakAttachment = makeSliderAttachment("sequencerPeak", peakSlider);
        moveAttachment = makeSliderAttachment("sequencerMove", moveSlider);
        configureSavedReset(rateSlider, "sequencerRate");
        configureSavedReset(startSlider, "sequencerStart");
        configureSavedReset(endSlider, "sequencerEnd");
        configureSavedReset(attackSlider, "sequencerAttack");
        configureSavedReset(releaseSlider, "sequencerRelease");
        configureSavedReset(peakSlider, "sequencerPeak");
        configureSavedReset(moveSlider, "sequencerMove");
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
        attackSlider.textFromValueFunction = releaseSlider.textFromValueFunction
            = [](double value) { return juce::String(value * 100.0, 0) + "%"; };
        peakSlider.textFromValueFunction = moveSlider.textFromValueFunction
            = [](double value) { return juce::String(value * 100.0, 0) + "%"; };

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
        update(attackLabel);
        update(releaseLabel);
        update(peakLabel);
        update(moveLabel);
    }
    void timerTick()
    {
        grid.repaint();
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
        drawPanel({ 10.0f, 10.0f, static_cast<float>(getWidth() - 20), 72.0f });
        drawPanel({ 10.0f, 88.0f, static_cast<float>(getWidth() - 20), 148.0f });
        graphics.setColour(knobLabelColour);
        graphics.setFont(juce::FontOptions(10.5f, juce::Font::bold));
        graphics.drawText("RATE", 366, 11, 60, 13,
                          juce::Justification::centred);
        graphics.drawText("START", 434, 11, 60, 13,
                          juce::Justification::centred);
        graphics.drawText("END", 526, 11, 60, 13,
                          juce::Justification::centred);
        graphics.drawText("DIRECTION", 598, 11, 106, 13,
                          juce::Justification::centred);
        graphics.drawText("HUE", 778, 11, 46, 13,
                          juce::Justification::centred);
        graphics.drawText("SAT", 820, 11, 38, 13,
                          juce::Justification::centred);
        graphics.drawText("LUM", 858, 11, 38, 13,
                          juce::Justification::centred);
        graphics.drawText("NUDGE", 45, 194, 70, 13,
                          juce::Justification::centred);
    }

    void resized() override
    {
        hostSync.setBounds(20, 34, 88, 26);
        waveformBox.setBounds(126, 34, 106, 26);
        userSequenceBox.setBounds(242, 34, 106, 26);
        rateSlider.setBounds(366, 24, 60, 57);
        startSlider.setBounds(434, 24, 60, 57);
        rangeLinkButton.setBounds(498, 35, 24, 24);
        endSlider.setBounds(526, 24, 60, 57);
        directionBox.setBounds(598, 34, 106, 26);
        colourSlider.setBounds(786, 32, 30, 30);
        saturationSlider.setBounds(824, 32, 30, 30);
        luminanceSlider.setBounds(862, 32, 30, 30);
        enabledButton.setBounds(20, 96, 34, 24);
        bipolarButton.setBounds(60, 96, 76, 24);
        attackLabel.setBounds(18, 122, 60, 13);
        releaseLabel.setBounds(82, 122, 60, 13);
        peakLabel.setBounds(153, 92, 48, 13);
        moveLabel.setBounds(153, 160, 48, 13);
        attackSlider.setBounds(18, 135, 60, 57);
        releaseSlider.setBounds(82, 135, 60, 57);
        peakSlider.setBounds(163, 106, 28, 58);
        moveSlider.setBounds(163, 174, 28, 58);
        nudgeLeft.setBounds(51, 206, 26, 26);
        nudgeRight.setBounds(83, 206, 26, 26);
        grid.setBounds(210, 96, 554, 132);
        targets.setBounds(774, 96, getWidth() - 792, 132);
    }

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    void mouseEnter(const juce::MouseEvent& event) override
    {
        if (showHelp == nullptr)
            return;

        for (auto* component = event.eventComponent;
             component != nullptr && component != this;
             component = component->getParentComponent())
        {
            if (auto* tooltipClient = dynamic_cast<juce::TooltipClient*>(component))
            {
                const auto tooltip = tooltipClient->getTooltip();
                if (tooltip.isNotEmpty())
                {
                    showHelp("SEQUENCER\n" + tooltip);
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
        for (auto* stepElement : xml->getChildWithTagNameIterator("Step"))
        {
            const int step = stepElement->getIntAttribute("index", -1);
            if (! juce::isPositiveAndBelow(step,
                                           afterglow::sequencer::stepCount))
                continue;
            setParameterActualValue(processor.parameters.getParameter(
                afterglow::sequencer::stepParameterId(step)),
                static_cast<float>(stepElement->getDoubleAttribute("value", 1.0)));
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
    juce::ToggleButton rangeLinkButton;
    juce::ComboBox waveformBox;
    juce::ComboBox userSequenceBox;
    juce::ComboBox directionBox;
    juce::ToggleButton nudgeLeft;
    juce::ToggleButton nudgeRight;
    DetentedSlider rateSlider;
    DetentedSlider startSlider;
    DetentedSlider endSlider;
    DetentedSlider attackSlider;
    DetentedSlider releaseSlider;
    DetentedSlider peakSlider;
    DetentedSlider moveSlider;
    ParameterDragLabel attackLabel;
    ParameterDragLabel releaseLabel;
    ParameterDragLabel peakLabel;
    ParameterDragLabel moveLabel;
    juce::Slider colourSlider;
    juce::Slider saturationSlider;
    juce::Slider luminanceSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
        hostSyncAttachment, enabledAttachment, bipolarAttachment,
        rangeLinkAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        directionAttachment;
    std::unique_ptr<SliderAttachment> rateAttachment, startAttachment,
        endAttachment, attackAttachment, releaseAttachment, peakAttachment,
        moveAttachment;
    juce::Array<juce::File> userSequenceFiles;
    std::unique_ptr<juce::FileChooser> sequenceFileChooser;
    juce::Colour accent { 0xff34d6c6 };
    int linkedRangeLength = afterglow::sequencer::stepCount - 1;
    bool updatingLinkedRange = false;
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
    graphics.drawText("::", markerX - markerPadding, 0.0f,
                      markerAdvance + markerPadding * 2.0f,
                      static_cast<float>(getHeight()),
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
    if (style != juce::Slider::LinearVertical)
    {
        juce::LookAndFeel_V4::drawLinearSlider(
            graphics, x, y, width, height, sliderPosition,
            minSliderPosition, maxSliderPosition, style, slider);
        return;
    }

    const float centreX = static_cast<float>(x + width / 2);
    const float top = static_cast<float>(y + 1);
    const float bottom = static_cast<float>(y + height - 1);
    const float centreY = 0.5f * (top + bottom);
    const auto accent = slider.findColour(
        juce::Slider::rotarySliderFillColourId);
    graphics.setColour(knobTrackColour.withAlpha(0.90f));
    graphics.drawLine(centreX, top, centreX, bottom, 1.25f);
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
    const bool sequencerLabelStyle = static_cast<bool>(
        button.getProperties().getWithDefault("sequencerLabelStyle", false));
    const bool standardTextButton = static_cast<bool>(
        button.getProperties().getWithDefault("standardTextButton", false));
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
        || standardTextButton)
    {
        const auto buttonBounds = bounds.reduced(0.5f);
        g.setColour(active && enabled ? activeColour.darker(0.32f)
                                     : juce::Colour(0xff242a30));
        g.fillRoundedRectangle(buttonBounds, 3.0f);
        g.setColour(active && enabled ? activeColour.brighter(0.12f)
                                     : (shouldDrawButtonAsHighlighted
                                            ? knobLabelColour
                                            : juce::Colour(0xff59616a)));
        g.drawRoundedRectangle(buttonBounds, 3.0f, 1.0f);

        const auto iconColour = presetArrowDirection != 0
            ? knobIndicatorColour.withAlpha(enabled ? 0.9f : 0.2f)
            : (enabled
                   ? (active ? juce::Colours::white : knobValueColour)
                   : knobLabelColour.darker(0.45f));
        g.setColour(iconColour);
        if (standardTextButton)
        {
            g.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
            g.drawFittedText(button.getButtonText(),
                             button.getLocalBounds().reduced(5, 2),
                             juce::Justification::centred, 1);
        }
        else if (rangeLinkIcon)
        {
            const auto centre = bounds.getCentre();
            const auto upper = juce::Rectangle<float>(10.0f, 7.0f)
                .withCentre(centre.translated(-3.5f, -2.5f));
            const auto lower = juce::Rectangle<float>(10.0f, 7.0f)
                .withCentre(centre.translated(3.5f, 2.5f));
            g.drawRoundedRectangle(upper, 3.0f, 1.4f);
            g.drawRoundedRectangle(lower, 3.0f, 1.4f);
            g.drawLine(centre.x - 2.0f, centre.y - 1.5f,
                       centre.x + 2.0f, centre.y + 1.5f, 1.4f);
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
            juce::Path arrow;
            if (presetArrowDirection < 0)
            {
                arrow.startNewSubPath(centre.x + 2.0f, centre.y - 7.0f);
                arrow.lineTo(centre.x - 3.0f, centre.y);
                arrow.lineTo(centre.x + 2.0f, centre.y + 7.0f);
            }
            else
            {
                arrow.startNewSubPath(centre.x - 2.0f, centre.y - 7.0f);
                arrow.lineTo(centre.x + 3.0f, centre.y);
                arrow.lineTo(centre.x - 2.0f, centre.y + 7.0f);
            }
            g.strokePath(arrow, juce::PathStrokeType(2.0f));
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
    const float diameter = centredLed
                         ? juce::jmin(bounds.getWidth(), bounds.getHeight()) - 6.0f
                         : 10.0f;
    const juce::Font toggleFont(sequencerLabelStyle
        ? juce::FontOptions(10.5f, juce::Font::bold)
        : juce::FontOptions(11.0f));
    const float textWidth = statusLedOnly ? 0.0f
        : juce::GlyphArrangement::getStringWidth(toggleFont, button.getButtonText());
    const float contentWidth = diameter + 5.0f + textWidth;
    const float contentX = bounds.getCentreX() - contentWidth * 0.5f;
    const juce::Point<float> centre = centredLed
        ? bounds.getCentre()
        : juce::Point<float>(contentX + diameter * 0.5f, bounds.getCentreY());
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
    const int textX = juce::roundToInt(contentX + diameter + 5.0f);
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
    versionLabel.setText("v1.57.3", juce::dontSendNotification);
    versionLabel.setJustificationType(juce::Justification::centred);
    versionLabel.setColour(juce::Label::textColourId, knobIndicatorColour);
    versionLabel.setFont(juce::Font(juce::FontOptions(8.5f, juce::Font::bold)));
    content.addAndMakeVisible(versionLabel);
    leftTimeDial = addDial("timeL", "LEFT TIME", "Left delay time. Tempo Sync switches this same knob between milliseconds and musical note values.");
    rightTimeDial = addDial("timeR", "RIGHT TIME", "Right delay time. Tempo Sync switches this same knob between milliseconds and musical note values.");
    leftTimeDial->label.setText("L", juce::dontSendNotification);
    rightTimeDial->label.setText("R", juce::dontSendNotification);
    rightTimeDial->label.dragHandleRight = true;
    feedbackDial = addDial("feedback", "FDBK", "Sets how much of each repeat returns to the delay input. Higher values create a longer tail; 100% sustains and values above it regenerate.");
    addDial("cross", "CROSS", "0% repeats each channel into itself; 100% swaps left and right on each repeat. Ping-Pong temporarily forces full cross-routing.");
    auto* spreadDial = addDial("spread", "SPREAD", "Stereo width of the delayed signal: 0% centres the repeats, +100% is full width, and negative values reverse their left/right placement.");
    spreadDial->slider.setCentreDetent(0.0, 2.0);
    spreadDial->slider.setDoubleClickReturnValue(true, 0.0);
    addDial("duck", "DUCK", "Reduces the audible delayed signal while new dry input is present. The feedback tail continues internally without being attenuated.");
    addDial("duckAttack", "ATTACK", "How quickly the ducking envelope turns down the delayed signal after new input arrives.");
    addDial("duckRelease", "RELEASE", "How quickly the delayed signal returns after the dry input level falls.");
    addDial("diffusionAmount", "DIFF AMT", "Sets diffusion density and decay. At 0% it is off; high values form a reverb-like tail.");
    addDial("diffusionSize", "DIFF SIZE", "Sets the diffusion span relative to the displayed delay while preserving the overall echo time. Pitch Rev deliberately adds the span instead.");
    modulationAmountDial = addDial("modulationAmount", "MOD AMT", "Depth of stereo delay-time movement in the selected mode.");
    addDial("modulationRate", "MOD RATE", "Speed of delay-time modulation, including LoFi's chaotic drift, Crystal's clean triangle motion and Phaser's notch sweep.");
    auto* mixDial = addDial("mix", "MIX", "0% is fully dry, 50% is an equal mix and 100% is fully wet. The centre has a tactile detent.");
    mixDial->slider.setCentreDetent(50.0, 2.0);
    mixDial->slider.setDoubleClickReturnValue(true, 50.0);
    addDial("lowCut", "LO CUT", "Remove low frequencies from every echo, including the first. At minimum this filter is bypassed.");
    addDial("highCut", "HI CUT", "Remove high frequencies from every echo, including the first. At maximum this filter is bypassed.");
    auto* outputDial = addDial("output", "OUT", "Adjust the final output level. The 0 dB centre position has a tactile detent.");
    outputDial->slider.setCentreDetent(0.0, 0.6);

    modeBox.addItem("Analogue", 13);
    modeBox.addItem("Brigade", 3);
    modeBox.addItem("Chaotic", 14);
    modeBox.addItem("Chaotic Rev", 16);
    modeBox.addItem("Chromatic", 12);
    modeBox.addItem("Crystal", 9);
    modeBox.addItem("Digital", 1);
    modeBox.addItem("Forward Rev", 17);
    modeBox.addItem("Ghost", 5);
    modeBox.addItem("HiFi", 4);
    modeBox.addItem("LoFi", 8);
    modeBox.addItem("Phaser", 10);
    modeBox.addItem("Pitch", 6);
    modeBox.addItem("Pitch Lush", 11);
    modeBox.addItem("Pitch Rev", 7);
    modeBox.addItem("Pure Reverse", 15);
    modeBox.addItem("Tape I", 2);
    modeBox.addItem("Tape II", 18);
    modeBox.setJustificationType(juce::Justification::centred);
    modeBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff19151f));
    modeBox.setColour(juce::ComboBox::textColourId, knobValueColour);
    modeBox.setColour(juce::ComboBox::outlineColourId, knobTrackColour);
    modeBox.setColour(juce::ComboBox::arrowColourId, knobIndicatorColour);
    modeBox.setScrollWheelEnabled(true);
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
    freezeModeButton.getProperties().set(
        "activeColour", controlAccentColour.toString());
    freezeModeButton.setClickingTogglesState(true);
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

    modeDials.push_back(addDial("drive", "DIST", "Amount of the selected input distortion applied once as audio enters the delay. Repeats are not re-distorted through the feedback loop."));
    modeDials.push_back(addDial("age", "AGE", "Changes the selected mode's age or character. The description follows the active mode."));
    modeDials.push_back(addDial("wow", "WOW", "Slow delay-time movement and pitch drift. Available in every delay mode."));
    modeDials.push_back(addDial("flutter", "FLUTTER", "Faster, finer delay-time movement with slight stereo variation. Available in every delay mode."));

    pitchShiftDial = addDial("pitchShift", "SHIFT", "Pitch, Pitch Rev and Pitch Lush transposition from -24 to +24 semitones. Zero preserves the original pitch.");
    pitchShiftDial->slider.setCentreDetent(0.0, 0.15);
    pitchShiftDial->slider.setDoubleClickReturnValue(true, 0.0);
    pitchDetuneDial = addDial("pitchDetune", "DETUNE", "Pitch, Pitch Rev and Pitch Lush stereo offset. Positive values raise Right and lower Left by half the displayed difference; negative values reverse them.");
    pitchDetuneDial->slider.setCentreDetent(0.0, 0.03);
    pitchDetuneDial->slider.setDoubleClickReturnValue(true, 0.0);
    for (size_t tap = 0; tap < multiTapCount; ++tap)
    {
        const auto tapNumber = juce::String(static_cast<int>(tap) + 1);
        const auto timeHelp = "Tap " + tapNumber
            + " delay position. Global Multi-Tap Sync switches every Time knob between milliseconds and note divisions without overwriting either value.";
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
            + " output level. Active tap levels use fixed RMS normalisation so enabling several taps cannot create an uncontrolled level jump.";
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
    themeDial = addDial("", "THEME", "Turn clockwise for the full mode-coloured surround; turn left for charcoal. The colour strip blends into the surround at maximum.", false);
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
        button.getProperties().set("activeColour", tempoSyncLedColour.toString());
        button.setClickingTogglesState(true);
        content.addAndMakeVisible(button);
    };
    configureSyncButton(syncLeftButton);
    configureSyncButton(syncRightButton);
    configureSyncButton(multiTapSyncButton);
    syncLeftButton.setTooltip("Switch the Left Time knob between milliseconds and note divisions");
    syncRightButton.setTooltip("Switch the Right Time knob between milliseconds and note divisions");
    multiTapSyncButton.setTooltip("Switch all four Multi-Tap Time knobs between milliseconds and note divisions");

    if (auto* parameter = processor.parameters.getParameter("delayStyle"))
        delayStyleAttachment = std::make_unique<juce::ParameterAttachment>(
            *parameter, [](float) {});

    multiTapButton.getProperties().set("statusLedOnly", true);
    multiTapButton.getProperties().set(
        "activeColour", knobIndicatorColour.toString());
    multiTapButton.setClickingTogglesState(true);
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
        button.getProperties().set(
            "activeColour", controlAccentColour.toString());
        button.setClickingTogglesState(true);
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

    presetBrowserButton.setName("Presets");
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

    information.setText("AFTERGLOW 1.57.3 | Drag a parameter label onto the sequencer target list\nSEQ opens or closes the modulation panel without disturbing the delay layout.", juce::dontSendNotification);
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
    sequencerPanel->setHue(storedSettings.sequencerHue);
    sequencerPanel->setSaturation(storedSettings.sequencerSaturation);
    sequencerPanel->setLuminance(storedSettings.sequencerLuminance);
    updateSequencerTargetHighlights();
    sequencerOpen = storedSettings.sequencerOpen;
    sequencerPanel->setVisible(sequencerOpen);

    sequencerButton.setClickingTogglesState(true);
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
                    1852, sequencerOpen ? 1104 : 552);
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
    content.removeMouseListener(this);
    presetFolderTree.setRootItem(nullptr);
    presetFolderRoot.reset();
    setLookAndFeel(nullptr);
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
                information.setText("AFTERGLOW\n" + tooltip,
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
        const bool headerControl = dial.get() == themeDial;
        dial->slider.setVisible(headerControl || ! presetBrowserOpen);
        dial->label.setVisible(headerControl || ! presetBrowserOpen);
    }

    presetFolderLabel.setVisible(presetBrowserOpen);
    presetListLabel.setVisible(presetBrowserOpen);
    presetFolderTree.setVisible(presetBrowserOpen);
    presetList.setVisible(presetBrowserOpen);
    presetSaveButton.setVisible(presetBrowserOpen);
    presetSaveAsButton.setVisible(presetBrowserOpen);
    presetOpenFolderButton.setVisible(presetBrowserOpen);
    information.setVisible(! presetBrowserOpen);

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
        0.75f, 2.0f,
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
                    1852, shouldOpen ? 1104 : 552);
    setSize(juce::roundToInt(static_cast<float>(designWidth) * scale),
            juce::roundToInt(static_cast<float>(activeDesignHeight()) * scale));
    saveInterfaceSettings();
    resized();
    repaint();
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

void AfterglowAudioProcessorEditor::rebuildPresetFolderTree()
{
    const auto root = AfterglowAudioProcessor::getPresetsDirectory();
    root.createDirectory();
    presetFolderTree.setRootItem(nullptr);
    presetFolderRoot.reset();
    presetFolderRoot = std::make_unique<FolderTreeItem>(
        root, [this](const juce::File& folder) { selectPresetFolder(folder); });
    presetFolderTree.setRootItem(presetFolderRoot.get());
    presetFolderRoot->setOpen(true);

    auto target = selectedPresetFolder;
    if (! target.isDirectory()
        || (target != root && ! target.isAChildOf(root)))
        target = root;

    if (auto* item = presetFolderRoot->findItemForDirectory(target))
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

    selectedPresetFolder = folder;
    refreshPresetList();
}

void AfterglowAudioProcessorEditor::refreshPresetList()
{
    visiblePresetFiles = presetFilesInFolder(selectedPresetFolder);

    presetList.updateContent();
    presetList.deselectAllRows();
    if (currentPresetFile.getParentDirectory() == selectedPresetFolder)
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
    const auto root = AfterglowAudioProcessor::getPresetsDirectory();
    auto folder = currentPresetFile.existsAsFile()
        ? currentPresetFile.getParentDirectory()
        : selectedPresetFolder;
    if (! folder.isDirectory()
        || (folder != root && ! folder.isAChildOf(root)))
        folder = root;

    const auto files = presetFilesInFolder(folder);
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

void AfterglowAudioProcessorEditor::stepPreset(int direction)
{
    const auto root = AfterglowAudioProcessor::getPresetsDirectory();
    auto folder = currentPresetFile.existsAsFile()
        ? currentPresetFile.getParentDirectory()
        : selectedPresetFolder;
    if (! folder.isDirectory()
        || (folder != root && ! folder.isAChildOf(root)))
        folder = root;

    const auto files = presetFilesInFolder(folder);
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
    const auto root = AfterglowAudioProcessor::getPresetsDirectory();
    auto folder = currentPresetFile.existsAsFile()
        ? currentPresetFile.getParentDirectory()
        : selectedPresetFolder;
    if (! folder.isDirectory()
        || (folder != root && ! folder.isAChildOf(root)))
        folder = root;

    const auto files = presetFilesInFolder(folder);
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
        g.setColour(isCurrent ? knobIndicatorColour : knobValueColour);
        g.setFont(juce::Font(juce::FontOptions(
            11.0f, isCurrent ? juce::Font::bold : juce::Font::plain)));
        g.drawFittedText(file.getFileNameWithoutExtension(), cellX + 10, 0,
                         juce::jmax(0, cellWidth - 34), height,
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
    selectedPresetBrowserColumn = event.position.x
        >= static_cast<float>(eventWidth) * 0.5f ? 1 : 0;
    if (row * 2 + selectedPresetBrowserColumn >= visiblePresetFiles.size())
        selectedPresetBrowserColumn = 0;
    presetList.repaintRow(row);
    loadPresetAtBrowserCell(row, selectedPresetBrowserColumn);
}

void AfterglowAudioProcessorEditor::returnKeyPressed(int row)
{
    loadPresetAtBrowserCell(row, selectedPresetBrowserColumn);
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
    const auto result = processor.loadPresetFromFile(file);
    if (result.failed())
    {
        information.setText("PRESET LOAD FAILED | " + result.getErrorMessage(),
                            juce::dontSendNotification);
        return;
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
    const bool multiTap = multiTapButton.getToggleState();
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
    multiTapButton.setVisible(controlsVisible);
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
                + " tempo-synchronised note division. Global Sync changes all four Time knobs without overwriting their stored millisecond values."
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
                feedbackDial->help = "Regeneration passes through the bucket-brigade colour. Short times and high values can emphasise resonant tones and noise.";
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
            default: characterDial->help = "AGE is not used in this mode."; break;
        }
    }
}

void AfterglowAudioProcessorEditor::updateModeControlState()
{
    const int selectedMode = modeBox.getSelectedId();
    const bool ageEnabled = (selectedMode >= 1 && selectedMode <= 9)
                         || selectedMode == 12 || selectedMode == 13
                         || selectedMode == 18;
    const bool phaserEnabled = selectedMode == 10;
    const bool ghostEnabled = selectedMode == 5;
    const bool lushPitchEnabled = selectedMode == 11;
    const bool pitchEnabled = selectedMode == 6 || selectedMode == 7
                           || lushPitchEnabled;
    for (auto* dial : modeDials)
    {
        dial->slider.setEnabled(true);
        dial->label.setEnabled(true);
    }
    if (modeDials.size() > 1)
    {
        auto* characterDial = modeDials[1];
        characterDial->attachment.reset();
        if (lushPitchEnabled)
        {
            characterDial->label.setText("P MIX", juce::dontSendNotification);
            characterDial->parameterId = "pitchMix";
            characterDial->attachment = std::make_unique<SliderAttachment>(
                processor.parameters, "pitchMix", characterDial->slider);
        }
        else
        {
            characterDial->label.setText("AGE", juce::dontSendNotification);
            characterDial->parameterId = "age";
            characterDial->attachment = std::make_unique<SliderAttachment>(
                processor.parameters, "age", characterDial->slider);
        }
        const bool characterEnabled = ageEnabled || lushPitchEnabled;
        characterDial->slider.setEnabled(characterEnabled);
        characterDial->label.setEnabled(characterEnabled);
    }
    if (pitchShiftDial != nullptr && pitchDetuneDial != nullptr)
    {
        pitchShiftDial->attachment.reset();
        pitchDetuneDial->attachment.reset();
        if (phaserEnabled)
        {
            pitchShiftDial->label.setText("RES", juce::dontSendNotification);
            pitchShiftDial->parameterId = "phaserResonance";
            pitchShiftDial->help = "Internal Phaser resonance. Higher settings reinforce the moving notches inside the regenerating delay path.";
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
            pitchShiftDial->help = "Ghost frequency shift from -1000 to +1000 Hz. The centre position applies no common shift; small values create slow spectral movement.";
            pitchShiftDial->slider.setCentreDetent(0.0, 0.75);
            pitchShiftDial->slider.setDoubleClickReturnValue(true, 0.0);
            pitchShiftDial->attachment = std::make_unique<SliderAttachment>(
                processor.parameters, "ghostShift", pitchShiftDial->slider);

            pitchDetuneDial->label.setText("DETUNE", juce::dontSendNotification);
            pitchDetuneDial->parameterId = "ghostDetune";
            pitchDetuneDial->help = "Ghost left/right frequency-shift separation from 0 to 100 Hz. Low values add subtle stereo motion; high values pan and beat rapidly.";
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
            pitchDetuneDial->help = "Pitch, Pitch Rev and Pitch Lush stereo offset. Positive values raise Right and lower Left; negative values reverse them.";
            pitchDetuneDial->slider.setCentreDetent(0.0, 0.03);
            pitchDetuneDial->slider.setDoubleClickReturnValue(true, 0.0);
            pitchDetuneDial->attachment = std::make_unique<SliderAttachment>(
                processor.parameters, "pitchDetune", pitchDetuneDial->slider);
        }

        const bool pitchPanelEnabled = pitchEnabled || phaserEnabled
                                    || ghostEnabled;
        pitchShiftDial->slider.setEnabled(pitchPanelEnabled);
        pitchShiftDial->label.setEnabled(pitchPanelEnabled);
        pitchDetuneDial->slider.setEnabled(pitchPanelEnabled);
        pitchDetuneDial->label.setEnabled(pitchPanelEnabled);
    }

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
        g.setColour(knobLabelColour);
        g.setFont(juce::Font(juce::FontOptions(7.6f, juce::Font::bold)));
        g.drawText("MULTI", 151, 211, 36, 12,
                   juce::Justification::centredRight);

        if (! multiTapButton.getToggleState())
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
        tapEnableButtons[tap].setBounds(172, rowY + 5, 18, 18);
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

    versionLabel.setBounds(designWidth - 65, 2, 66, 12);
    modeLabel.setBounds(204, 17, 38, 20);
    modeBox.setBounds(242, 13, 100, 26);
    distortionTypeLabel.setBounds(348, 17, 54, 20);
    distortionTypeBox.setBounds(402, 13, 106, 26);
    freezeButton.setBounds(516, 13, 26, 26);
    freezeModeButton.setBounds(548, 19, 14, 14);
    freezeModeLabel.setBounds(563, 15, 64, 20);
    currentPresetLabel.setBounds(632, 13, 94, 26);
    previousPresetButton.setBounds(726, 13, 21, 26);
    nextPresetButton.setBounds(747, 13, 21, 26);
    presetBrowserButton.setBounds(774, 13, 26, 26);
    linkButton.setBounds(30, 176, 84, 24);
    pingPongButton.setBounds(110, 176, 84, 24);
    syncLeftButton.setBounds(65, 143, 14, 14);
    syncRightButton.setBounds(145, 143, 14, 14);
    multiTapSyncButton.setBounds(22, 210, 14, 14);
    multiTapButton.setBounds(188, 210, 14, 14);
    if (themeDial != nullptr)
    {
        themeDial->label.setBounds(designWidth - 118, 2, 58, 12);
        themeDial->slider.setBounds(designWidth - 104, 16, 30, 30);
    }
    bypassButton.setBounds(designWidth - 47, 16, 30, 30);
    presetFolderLabel.setBounds(28, 66, 252, 14);
    presetListLabel.setBounds(298, 66, 560, 14);
    presetFolderTree.setBounds(26, 82, 256, 168);
    presetList.setBounds(296, 82, 564, 168);
    presetSaveButton.setBounds(870, 93, 26, 26);
    presetSaveAsButton.setBounds(870, 138, 26, 26);
    presetOpenFolderButton.setBounds(870, 183, 26, 26);
    information.setBounds(25, 236, 683, 28);
    sequencerButton.setBounds(presetBrowserOpen ? 865 : 859,
                              presetBrowserOpen ? 220 : 236,
                              36, 26);
    if (sequencerPanel != nullptr)
        sequencerPanel->setBounds(8, collapsedDesignHeight,
                                  designWidth - 16,
                                  expandedDesignHeight
                                      - collapsedDesignHeight - 8);
}

void AfterglowAudioProcessorEditor::timerCallback()
{
    updateSequencerTargetHighlights();
    if (sequencerOpen && sequencerPanel != nullptr)
        sequencerPanel->timerTick();

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
        75, 200, juce::roundToInt(100.0f * static_cast<float>(getWidth())
                                  / static_cast<float>(designWidth)));
    const double themePercent = themeDial != nullptr
        ? juce::jlimit(0.0, 100.0, themeDial->slider.getValue()) : 100.0;
    juce::String text;
    text << "; Afterglow interface settings\r\n"
         << "[GUI]\r\n"
         << "ScalePercent=" << scalePercent << "\r\n"
         << "ThemePercent=" << juce::String(themePercent, 1) << "\r\n"
         << "SequencerOpen=" << (sequencerOpen ? 1 : 0) << "\r\n"
         << "SequencerHue="
         << juce::String(sequencerPanel != nullptr
                             ? sequencerPanel->getHue()
                             : juce::Colour(0xff34d6c6).getHue(), 4)
         << "\r\n"
         << "SequencerSaturation="
         << juce::String(sequencerPanel != nullptr
                             ? sequencerPanel->getSaturation()
                             : 0.72, 4)
         << "\r\n"
         << "SequencerLuminance="
         << juce::String(sequencerPanel != nullptr
                             ? sequencerPanel->getLuminance()
                             : 0.82, 4)
         << "\r\n";
    directory.getChildFile("Settings.ini").replaceWithText(text);
}
