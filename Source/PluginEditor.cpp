#include "PluginEditor.h"
#include "ChannelStripArtworkData.h"
#include <cmath>

class ChannelStripEditor::Dial final : public juce::Slider
{
public:
    explicit Dial(juce::Colour knobColour) : colour(knobColour)
    {
        setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        setRotaryParameters(juce::degreesToRadians(225.0f),
                            juce::degreesToRadians(495.0f), true);
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }

    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced(2.0f);
        const auto diameter = juce::jmin(bounds.getWidth(), bounds.getHeight());
        const auto knob = bounds.withSizeKeepingCentre(diameter, diameter);
        const auto centre = knob.getCentre();
        const auto radius = diameter * 0.48f;

        g.setColour(juce::Colour(0xff050606));
        g.fillEllipse(knob);
        juce::ColourGradient body(colour.brighter(0.55f), centre.x - radius * 0.45f,
                                  centre.y - radius * 0.7f,
                                  colour.darker(0.5f), centre.x + radius * 0.65f,
                                  centre.y + radius * 0.7f, false);
        g.setGradientFill(body);
        g.fillEllipse(knob.reduced(2.2f));
        g.setColour(juce::Colours::white.withAlpha(0.15f));
        g.drawEllipse(knob.reduced(2.2f), 0.7f);

        const auto proportion = (getValue() - getMinimum()) /
                                juce::jmax(0.0001, getMaximum() - getMinimum());
        const auto angle = juce::degreesToRadians(225.0 + 270.0 * proportion);
        const auto pointerStart = radius * 0.18f;
        const auto pointerEnd = radius * 0.75f;
        juce::Path pointer;
        pointer.startNewSubPath(centre.x + std::cos(angle) * pointerStart,
                                centre.y + std::sin(angle) * pointerStart);
        pointer.lineTo(centre.x + std::cos(angle) * pointerEnd,
                       centre.y + std::sin(angle) * pointerEnd);
        g.setColour(juce::Colours::white.withAlpha(0.95f));
        g.strokePath(pointer, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));
    }

private:
    juce::Colour colour;
};

class ChannelStripEditor::PanelButton final : public juce::ToggleButton
{
public:
    explicit PanelButton(const juce::String& label) : ToggleButton(label)
    {
        setClickingTogglesState(true);
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }

    void paintButton(juce::Graphics& g, bool, bool) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.0f);
        g.setColour(juce::Colour(0xff080909));
        g.fillRoundedRectangle(bounds, 1.5f);
        g.setColour(juce::Colour(0xff777777));
        g.drawRoundedRectangle(bounds, 1.5f, 0.8f);
        g.setColour(juce::Colour(0xffc7c7c7));
        g.setFont(juce::Font(juce::FontOptions(8.5f, juce::Font::bold)));
        g.drawFittedText(getButtonText(), getLocalBounds().reduced(1), juce::Justification::centred, 1);
        g.setColour(getToggleState() ? juce::Colour(0xff76d978) : juce::Colour(0xff19371d));
        g.fillEllipse(bounds.getCentreX() - 2.0f, bounds.getBottom() - 4.5f, 4.0f, 4.0f);
    }
};

class ChannelStripEditor::OutputFader final : public juce::Slider
{
public:
    OutputFader()
    {
        setSliderStyle(juce::Slider::LinearVertical);
        setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        setRange(-18.0, 18.0, 0.1);
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }

    void paint(juce::Graphics& g) override
    {
        const auto track = getLocalBounds().toFloat().withSizeKeepingCentre(4.0f, getHeight() - 4.0f);
        g.setColour(juce::Colour(0xff090a0a));
        g.fillRoundedRectangle(track, 1.5f);
        const auto proportion = (getValue() - getMinimum()) / (getMaximum() - getMinimum());
        const auto y = juce::jmap(static_cast<float>(proportion),
                                  track.getBottom() - 3.0f, track.getY() + 3.0f);
        auto thumb = juce::Rectangle<float>(getWidth() * 0.18f, 18.0f)
                        .withCentre({ getWidth() * 0.5f, y });
        g.setColour(juce::Colour(0xff777777));
        g.fillRoundedRectangle(thumb, 2.0f);
        g.setColour(juce::Colour(0xffd0d0d0));
        g.drawLine(thumb.getX() + 2.0f, thumb.getCentreY(),
                   thumb.getRight() - 2.0f, thumb.getCentreY(), 1.0f);
    }
};

ChannelStripEditor::ChannelStripEditor(ChannelStripProcessor& p)
    : AudioProcessorEditor(&p), processor(p),
      faceplate(juce::ImageCache::getFromMemory(ChannelStripArtworkData::faceplate_png,
                                                ChannelStripArtworkData::faceplate_pngSize))
{
    setSize(366, 680);
    setResizable(false, false);

    presets.addItem("Default Preset", 1);
    presets.addItem("Vocal", 2);
    presets.addItem("Punch", 3);
    presets.addItem("Drum Bus", 4);
    presets.addItem("Clean", 5);
    presets.addItem("Aggressive", 6);
    presets.setSelectedId(processor.getCurrentProgram() + 1, juce::dontSendNotification);
    presets.setTooltip("Load a factory preset");
    presets.onChange = [this] { loadPreset(presets.getSelectedItemIndex()); };
    addAndMakeVisible(presets);

    addDial("hpFreq", { 34, 105, 38, 38 }, juce::Colour(0xffc9c5b9));
    addDial("lpFreq", { 125, 63, 38, 38 }, juce::Colour(0xffc9c5b9));
    addDial("compRatio", { 207, 63, 38, 38 }, juce::Colour(0xffc9c5b9));
    addDial("compThreshold", { 296, 103, 38, 38 }, juce::Colour(0xffc9c5b9));
    addDial("compRelease", { 206, 142, 38, 38 }, juce::Colour(0xffc9c5b9));
    addDial("hfGain", { 34, 182, 38, 38 }, juce::Colour(0xffb52f3a));
    addDial("hfFreq", { 125, 220, 38, 38 }, juce::Colour(0xffb52f3a));
    addDial("hmGain", { 34, 260, 38, 38 }, juce::Colour(0xff16845b));
    addDial("hmFreq", { 125, 300, 38, 38 }, juce::Colour(0xff16845b));
    addDial("lmGain", { 34, 340, 38, 38 }, juce::Colour(0xff1687ad));
    addDial("lmFreq", { 125, 461, 38, 38 }, juce::Colour(0xff1687ad));
    addDial("lfGain", { 34, 579, 38, 38 }, juce::Colour(0xffc9c5b9));
    addDial("lfFreq", { 125, 541, 38, 38 }, juce::Colour(0xffc9c5b9));
    addDial("gateThreshold", { 296, 182, 38, 38 }, juce::Colour(0xff16845b));
    addDial("gateRange", { 206, 221, 38, 38 }, juce::Colour(0xff16845b));
    addDial("gateRelease", { 206, 300, 38, 38 }, juce::Colour(0xff16845b));
    addDial("input", { 206, 541, 38, 38 }, juce::Colour(0xffc9c5b9));

    addButton("split", "SPLIT", { 124, 123, 40, 23 });
    addButton("hmRange", "HMF x3", { 124, 180, 40, 23 });
    addButton("lmRange", "LMF x3", { 124, 602, 40, 22 });
    addButton("eqBypass", "BYP", { 124, 386, 23, 35 });
    addButton("compBypass", "BYP", { 205, 386, 23, 35 });
    addButton("gateOn", "GATE", { 294, 239, 42, 21 });
    addButton("analog", "ANALOG", { 211, 474, 34, 19 });
    addButton("phase", "Ø", { 205, 602, 39, 22 });

    outputFader = std::make_unique<OutputFader>();
    outputFader->setBounds(330, 448, 27, 105);
    addAndMakeVisible(*outputFader);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, "output", *outputFader);

    startTimerHz(30);
}

ChannelStripEditor::~ChannelStripEditor()
{
    stopTimer();
}

void ChannelStripEditor::addDial(const char* id, juce::Rectangle<int> bounds, juce::Colour colour)
{
    auto dial = std::make_unique<Dial>(colour);
    dial->setBounds(bounds);
    dial->setName(juce::String(id));
    if (const auto* parameter = processor.parameters.getParameter(id))
        dial->setTooltip(parameter->getName(64));
    addAndMakeVisible(*dial);
    sliderAttachments.push_back(
        std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, id, *dial));
    controls.push_back(std::move(dial));
}

void ChannelStripEditor::addButton(const char* id, const juce::String& text,
                                   juce::Rectangle<int> bounds)
{
    auto button = std::make_unique<PanelButton>(text);
    button->setBounds(bounds);
    button->setTooltip(text);
    addAndMakeVisible(*button);
    buttonAttachments.push_back(
        std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            processor.parameters, id, *button));
    controls.push_back(std::move(button));
}

void ChannelStripEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff111313));
    if (faceplate.isValid())
        g.drawImage(faceplate, getLocalBounds().toFloat());

    const auto input = juce::jlimit(-60.0f, 0.0f, processor.inputLevelDb.load());
    const auto output = juce::jlimit(-60.0f, 0.0f, processor.outputLevelDb.load());
    const auto drawMeter = [&g](int x, float db)
    {
        g.setColour(juce::Colour(0xff111313));
        g.fillRect(x, 438, 11, 180);
        const auto lit = juce::jlimit(0, 30, juce::roundToInt((db + 60.0f) * 0.5f));
        for (int i = 0; i < lit; ++i)
        {
            const auto y = 614 - i * 6;
            const auto colour = i >= 27 ? juce::Colour(0xffdc303a)
                              : i >= 24 ? juce::Colour(0xffffc33c)
                                        : juce::Colour(0xff45e44b);
            g.setColour(colour);
            g.fillRect(x, y, 9, 5);
        }
    };
    drawMeter(303, input);
    drawMeter(318, output);
}

void ChannelStripEditor::resized()
{
    presets.setBounds(7, 18, 351, 17);
    presets.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1a1b1b));
    presets.setColour(juce::ComboBox::textColourId, juce::Colour(0xffdedede));
    presets.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff777777));
}

void ChannelStripEditor::loadPreset(int index)
{
    processor.setCurrentProgram(index);
}

void ChannelStripEditor::timerCallback()
{
    repaint(295, 430, 42, 193);
    const auto selectedId = processor.getCurrentProgram() + 1;
    if (presets.getSelectedId() != selectedId)
        presets.setSelectedId(selectedId, juce::dontSendNotification);
}
