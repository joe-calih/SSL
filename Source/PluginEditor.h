#pragma once

#include "PluginProcessor.h"
#include <vector>

class ChannelStripEditor final : public juce::AudioProcessorEditor,
                                 private juce::Timer
{
public:
    explicit ChannelStripEditor(ChannelStripProcessor&);
    ~ChannelStripEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    class Dial;
    class PanelButton;
    class OutputFader;

    void addDial(const char* id, juce::Rectangle<int> bounds, juce::Colour colour);
    void addButton(const char* id, const juce::String& text, juce::Rectangle<int> bounds);
    void loadPreset(int index);
    void timerCallback() override;

    ChannelStripProcessor& processor;
    juce::Image faceplate;
    juce::ComboBox presets;
    std::vector<std::unique_ptr<juce::Component>> controls;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;
    std::unique_ptr<OutputFader> outputFader;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelStripEditor)
};
