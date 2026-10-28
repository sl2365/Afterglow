#pragma once
#include "PluginProcessor.h"
#include <cmath>
#include <functional>

class AfterglowAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                            public juce::DragAndDropContainer,
                                            private juce::Timer,
                                            private juce::ListBoxModel
{
public:
    explicit AfterglowAudioProcessorEditor(AfterglowAudioProcessor&);
    ~AfterglowAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseEnter(const juce::MouseEvent&) override;
private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    class DetentedSlider final : public juce::Slider
    {
    public:
        std::function<bool()> onSavedStateReset;

        void setCentreDetent(double value, double radius)
        {
            centreDetent = value;
            detentRadius = juce::jmax(0.0, radius);
            centreDetentEnabled = true;
            getProperties().set("hasCentreDetent", true);
            getProperties().set("centreDetentValue", value);
        }

        void clearCentreDetent()
        {
            centreDetentEnabled = false;
            getProperties().remove("hasCentreDetent");
            getProperties().remove("centreDetentValue");
        }

        double snapValue(double attemptedValue, DragMode dragMode) override
        {
            if (centreDetentEnabled && dragMode != notDragging
                && std::abs(attemptedValue - centreDetent) <= detentRadius)
                return centreDetent;

            return attemptedValue;
        }

        void mouseDoubleClick(const juce::MouseEvent& event) override
        {
            if (onSavedStateReset != nullptr && onSavedStateReset())
                return;

            juce::Slider::mouseDoubleClick(event);
        }

    private:
        double centreDetent = 0.0;
        double detentRadius = 0.0;
        bool centreDetentEnabled = false;
    };

    class SteppedComboBox final : public juce::ComboBox
    {
    public:
        void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override
        {
            if (! isEnabled() || getNumItems() <= 0 || std::abs(wheel.deltaY) < 0.0001f)
                return;

            const int direction = wheel.deltaY > 0.0f ? -1 : 1;
            const auto now = juce::Time::getMillisecondCounter();
            if (direction == lastWheelDirection && now - lastWheelTimeMs < 100u)
                return;

            lastWheelDirection = direction;
            lastWheelTimeMs = now;
            const int currentIndex = juce::jmax(0, getSelectedItemIndex());
            const int nextIndex = juce::jlimit(0, getNumItems() - 1,
                                                currentIndex + direction);
            if (nextIndex != getSelectedItemIndex())
                setSelectedItemIndex(nextIndex, juce::sendNotificationSync);
        }

    private:
        juce::uint32 lastWheelTimeMs = 0;
        int lastWheelDirection = 0;
    };

    class PresetDisplayLabel final : public juce::Label
    {
    public:
        std::function<void(int)> onStep;
        std::function<void()> onShowMenu;

        void paint(juce::Graphics&) override;
        void mouseDown(const juce::MouseEvent&) override;
        void mouseWheelMove(const juce::MouseEvent&,
                            const juce::MouseWheelDetails&) override;

    private:
        juce::uint32 lastWheelTimeMs = 0;
        int lastWheelDirection = 0;
    };

    class ParameterDragLabel final : public juce::Label
    {
    public:
        std::function<juce::String()> parameterIdProvider;
        bool dragHandleOnly = false;
        bool dragHandleRight = false;
        void setTargetAppearance(bool assigned, juce::Colour accent);
        void paint(juce::Graphics&) override;
        void mouseDrag(const juce::MouseEvent&) override;

    private:
        bool targetAssigned = false;
        juce::Colour targetAccent { 0xff34d6c6 };
    };

    struct Dial
    {
        DetentedSlider slider;
        ParameterDragLabel label;
        std::unique_ptr<SliderAttachment> attachment;
        juce::String help;
        juce::String parameterId;
    };
    class SequencerPanel;
    class LookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        explicit LookAndFeel(bool compactPopupFont = false);
        juce::Label* createSliderTextBox(juce::Slider&) override;
        juce::Font getPopupMenuFont() override;
        void drawRotarySlider(juce::Graphics&, int, int, int, int,
                              float, float, float, juce::Slider&) override;
        void drawLinearSlider(juce::Graphics&, int, int, int, int,
                              float, float, float,
                              juce::Slider::SliderStyle,
                              juce::Slider&) override;
        void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
                              bool, bool) override;
        void drawPopupMenuItem(juce::Graphics&, const juce::Rectangle<int>&,
                               bool, bool, bool, bool, bool,
                               const juce::String&, const juce::String&,
                               const juce::Drawable*, const juce::Colour*) override;
        void drawPopupMenuBackground(juce::Graphics&, int, int) override;
        juce::PopupMenu::Options getOptionsForComboBoxPopupMenu(
            juce::ComboBox&, juce::Label&) override;
        void setPopupBorderColour(juce::Colour colour)
        {
            popupBorderColour = colour;
        }
        juce::Colour getPopupBorderColour() const noexcept
        {
            return popupBorderColour;
        }

    private:
        juce::Colour popupBorderColour { 0xff1479b8 };
        bool useCompactPopupFont = false;
    };
    class FolderTreeItem;
    AfterglowAudioProcessor& processor;
    LookAndFeel lookAndFeel;
    juce::Component content;
    std::vector<std::unique_ptr<Dial>> dials;
    std::vector<Dial*> modeDials;
    Dial* leftTimeDial = nullptr;
    Dial* rightTimeDial = nullptr;
    Dial* feedbackDial = nullptr;
    Dial* modulationAmountDial = nullptr;
    static constexpr size_t multiTapCount = 4;
    std::array<Dial*, multiTapCount> tapTimeDials {};
    std::array<Dial*, multiTapCount> tapPanDials {};
    std::array<Dial*, multiTapCount> tapLevelDials {};
    Dial* pitchShiftDial = nullptr;
    Dial* pitchDetuneDial = nullptr;
    Dial* themeDial = nullptr;
    juce::Image logoImage;
    juce::Label versionLabel;
    SteppedComboBox modeBox;
    juce::Label modeLabel;
    std::unique_ptr<juce::ParameterAttachment> modeAttachment;
    SteppedComboBox distortionTypeBox;
    juce::Label distortionTypeLabel;
    std::unique_ptr<juce::ParameterAttachment> distortionTypeAttachment;
    juce::ToggleButton freezeButton;
    std::unique_ptr<juce::ParameterAttachment> freezeAttachment;
    juce::ToggleButton freezeModeButton;
    juce::Label freezeModeLabel;
    std::unique_ptr<juce::ParameterAttachment> freezeModeAttachment;
    juce::ToggleButton linkButton { "Link L/R" };
    std::unique_ptr<juce::ParameterAttachment> linkAttachment;
    juce::ToggleButton syncLeftButton;
    juce::ToggleButton syncRightButton;
    std::unique_ptr<juce::ParameterAttachment> syncLeftAttachment;
    std::unique_ptr<juce::ParameterAttachment> syncRightAttachment;
    juce::ToggleButton pingPongButton { "Ping-Pong" };
    std::unique_ptr<juce::ParameterAttachment> pingPongAttachment;
    std::unique_ptr<juce::ParameterAttachment> delayStyleAttachment;
    juce::ToggleButton multiTapButton;
    std::unique_ptr<juce::ParameterAttachment> multiTapAttachment;
    juce::ToggleButton multiTapSyncButton;
    std::unique_ptr<juce::ParameterAttachment> multiTapSyncAttachment;
    std::array<juce::ToggleButton, multiTapCount> tapEnableButtons;
    std::array<std::unique_ptr<juce::ParameterAttachment>, multiTapCount>
        tapEnableAttachments;
    juce::ToggleButton bypassButton;
    std::unique_ptr<juce::ParameterAttachment> bypassAttachment;
    juce::ToggleButton presetBrowserButton;
    juce::ToggleButton previousPresetButton;
    juce::ToggleButton nextPresetButton;
    PresetDisplayLabel currentPresetLabel;
    juce::TreeView presetFolderTree;
    juce::ListBox presetList { "Preset list", this };
    juce::Label presetFolderLabel;
    juce::Label presetListLabel;
    juce::ToggleButton presetSaveButton;
    juce::ToggleButton presetSaveAsButton;
    juce::ToggleButton presetOpenFolderButton;
    juce::ToggleButton sequencerButton { "SEQ" };
    std::unique_ptr<SequencerPanel> sequencerPanel;
    std::unique_ptr<FolderTreeItem> presetFolderRoot;
    std::unique_ptr<juce::FileChooser> presetFileChooser;
    juce::Array<juce::File> visiblePresetFiles;
    juce::File selectedPresetFolder;
    juce::File currentPresetFile;
    juce::Label information;
    static constexpr int designWidth = 926;
    static constexpr int collapsedDesignHeight = 276;
    static constexpr int expandedDesignHeight = 530;
    int activeDesignHeight() const noexcept
    {
        return sequencerOpen ? expandedDesignHeight : collapsedDesignHeight;
    }
    Dial* addDial(const char*, const char*, const char*, bool attachToParameter = true);
    bool restoreDialFromSavedState(Dial&);
    void synchroniseLinkedTimeControls();
    void synchroniseDelayStyleFromTimingControls();
    void updateTimeControlState();
    void updateModeControlState();
    void updateContextSensitiveHelp(int selectedMode);
    void setPresetBrowserOpen(bool);
    void updatePresetBrowserVisibility();
    void rebuildPresetFolderTree();
    void selectPresetFolder(const juce::File&);
    void refreshPresetList();
    void updatePresetNavigationDisplay();
    void setSequencerOpen(bool);
    void updateSequencerTargetHighlights();
    void stepPreset(int direction);
    void showPresetQuickMenu();
    void loadPresetFile(const juce::File&);
    void loadPresetAtBrowserCell(int row, int column);
    void saveCurrentPreset();
    void showPresetSaveAs();
    int getNumRows() override;
    void paintListBoxItem(int, juce::Graphics&, int, int, bool) override;
    void listBoxItemClicked(int, const juce::MouseEvent&) override;
    void returnKeyPressed(int) override;
    void setFreezeActiveFromUi(bool active);
    void updateFreezeModeDisplay();
    void timerCallback() override;
    void saveInterfaceSettings() const;
    int observedEditorWidth = designWidth;
    double observedThemeValue = 100.0;
    int stableSettingsTicks = 0;
    bool settingsDirty = false;
    bool updatingFreezeControls = false;
    bool presetBrowserOpen = false;
    bool sequencerOpen = false;
    int selectedPresetBrowserColumn = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AfterglowAudioProcessorEditor)
};
