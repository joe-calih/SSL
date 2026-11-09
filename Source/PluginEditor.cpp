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
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.drawEllipse(knob.reduced(2.2f), 0.7f);
        g.setColour(juce::Colours::black.withAlpha(0.42f));
        g.drawEllipse(knob.reduced(3.1f), 1.0f);

        const auto proportion = (getValue() - getMinimum()) /
                                juce::jmax(0.0001, getMaximum() - getMinimum());
        const auto angle = juce::degreesToRadians(225.0 + 270.0 * proportion);
        for (int tick = 0; tick <= 10; ++tick)
        {
            const auto tickAngle = juce::degreesToRadians(225.0 + tick * 27.0);
            const auto outer = radius * 0.9f;
            const auto inner = radius * (tick % 5 == 0 ? 0.77f : 0.82f);
            juce::Path marker;
            marker.startNewSubPath(centre.x + std::cos(tickAngle) * inner,
                                   centre.y + std::sin(tickAngle) * inner);
            marker.lineTo(centre.x + std::cos(tickAngle) * outer,
                          centre.y + std::sin(tickAngle) * outer);
            g.setColour(juce::Colours::black.withAlpha(0.6f));
            g.strokePath(marker, juce::PathStrokeType(tick % 5 == 0 ? 1.2f : 0.7f));
        }

        juce::Graphics::ScopedSaveState saveState(g);
        g.addTransform(juce::AffineTransform::rotation(
            static_cast<float>(angle) + juce::MathConstants<float>::halfPi,
            centre.x, centre.y));
        juce::Path engravedMark;
        engravedMark.startNewSubPath(-1.6f, -radius * 0.8f);
        engravedMark.lineTo(1.6f, -radius * 0.8f);
        engravedMark.lineTo(0.95f, -radius * 0.25f);
        engravedMark.lineTo(-0.95f, -radius * 0.25f);
        engravedMark.closeSubPath();
        g.setColour(juce::Colours::black.withAlpha(0.78f));
        g.fillPath(engravedMark, juce::AffineTransform::translation(0.0f, 0.8f));
        g.setColour(juce::Colour(0xff68645c));
        g.fillPath(engravedMark);
        g.setColour(juce::Colour(0xffd8d3c7).withAlpha(0.7f));
        g.strokePath(engravedMark, juce::PathStrokeType(0.6f));
        juce::Path glint;
        glint.startNewSubPath(-0.7f, -radius * 0.72f);
        glint.lineTo(-0.35f, -radius * 0.36f);
        g.setColour(juce::Colours::white.withAlpha(0.62f));
        g.strokePath(glint, juce::PathStrokeType(0.55f));
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
    setResizable(false, false);

    for (int preset = 0; preset < processor.getNumPrograms(); ++preset)
        presets.addItem(processor.getProgramName(preset), preset + 1);
    presets.setSelectedId(processor.getCurrentProgram() + 1, juce::dontSendNotification);
    presets.setTooltip("Load a factory preset");
    presets.onChange = [this] { loadPreset(presets.getSelectedItemIndex()); };
    addAndMakeVisible(presets);

    for (int percent = 10; percent <= 200; percent += 10)
        uiScale.addItem(juce::String(percent) + "%", percent);
    uiScale.setSelectedId(scalePercent, juce::dontSendNotification);
    uiScale.setTooltip("Plugin window size");
    uiScale.onChange = [this]
    {
        scalePercent = uiScale.getSelectedId();
        setSize(juce::roundToInt(366.0f * scalePercent / 100.0f),
                juce::roundToInt(680.0f * scalePercent / 100.0f));
    };
    addAndMakeVisible(uiScale);

    aboutButton.setTooltip("About Loops and plugin licenses");
    aboutButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff222424));
    aboutButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffd8d0b8));
    aboutButton.setColour(juce::TextButton::textColourOnId, juce::Colour(0xffe6dcc0));
    aboutButton.onClick = [this] { showAbout(); };
    addAndMakeVisible(aboutButton);

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

    setSize(366, 680);
    startTimerHz(30);
}

ChannelStripEditor::~ChannelStripEditor()
{
    stopTimer();
}

void ChannelStripEditor::addDial(const char* id, juce::Rectangle<int> bounds, juce::Colour colour)
{
    auto dial = std::make_unique<Dial>(colour);
    const auto scale = getWidth() / 366.0f;
    dial->setBounds(juce::roundToInt(bounds.getX() * scale),
                    juce::roundToInt(bounds.getY() * scale),
                    juce::roundToInt(bounds.getWidth() * scale),
                    juce::roundToInt(bounds.getHeight() * scale));
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
    const auto scale = getWidth() / 366.0f;
    button->setBounds(juce::roundToInt(bounds.getX() * scale),
                      juce::roundToInt(bounds.getY() * scale),
                      juce::roundToInt(bounds.getWidth() * scale),
                      juce::roundToInt(bounds.getHeight() * scale));
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

    const auto scale = getWidth() / 366.0f;
    drawPanelTitle(g, { 34.0f * scale, 39.0f * scale, 62.0f * scale, 13.0f * scale }, "FILTERS");
    drawPanelTitle(g, { 280.0f * scale, 39.0f * scale, 68.0f * scale, 13.0f * scale }, "DYNAMICS");
    drawPanelTitle(g, { 95.0f * scale, 369.0f * scale, 68.0f * scale, 14.0f * scale }, "4-BAND EQ");
    drawPanelTitle(g, { 292.0f * scale, 369.0f * scale, 57.0f * scale, 14.0f * scale }, "GAIN");
    drawPanelTitle(g, { 203.0f * scale, 467.0f * scale, 54.0f * scale, 14.0f * scale }, "ANALOG");
    drawPanelTitle(g, { 190.0f * scale, 524.0f * scale, 72.0f * scale, 13.0f * scale }, "INPUT LEVEL");
    drawPanelTitle(g, { 298.0f * scale, 419.0f * scale, 58.0f * scale, 13.0f * scale }, "OUTPUT");

    const auto input = juce::jlimit(-60.0f, 0.0f, processor.inputLevelDb.load());
    const auto output = juce::jlimit(-60.0f, 0.0f, processor.outputLevelDb.load());
    const auto drawMeter = [&g, scale](int x, float db)
    {
        g.setColour(juce::Colour(0xff111313));
        g.fillRect(juce::roundToInt(x * scale), juce::roundToInt(438 * scale),
                   juce::roundToInt(11 * scale), juce::roundToInt(180 * scale));
        const auto lit = juce::jlimit(0, 30, juce::roundToInt((db + 60.0f) * 0.5f));
        for (int i = 0; i < lit; ++i)
        {
            const auto y = juce::roundToInt((614 - i * 6) * scale);
            const auto colour = i >= 27 ? juce::Colour(0xffdc303a)
                              : i >= 24 ? juce::Colour(0xffffc33c)
                                        : juce::Colour(0xff45e44b);
            g.setColour(colour);
            g.fillRect(juce::roundToInt(x * scale), y,
                       juce::roundToInt(9 * scale), juce::roundToInt(5 * scale));
        }
    };
    drawMeter(303, input);
    drawMeter(318, output);
}

void ChannelStripEditor::resized()
{
    const auto scale = getWidth() / 366.0f;
    presets.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1a1b1b));
    presets.setColour(juce::ComboBox::textColourId, juce::Colour(0xffdedede));
    presets.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff777777));
    uiScale.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1a1b1b));
    uiScale.setColour(juce::ComboBox::textColourId, juce::Colour(0xffdedede));
    uiScale.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff777777));

    const auto reposition = [scale](juce::Component& component, int x, int y, int width, int height)
    {
        component.setBounds(juce::roundToInt(x * scale), juce::roundToInt(y * scale),
                            juce::roundToInt(width * scale), juce::roundToInt(height * scale));
    };
    reposition(presets, 7, 18, 220, 17);
    reposition(uiScale, 232, 18, 80, 17);
    reposition(aboutButton, 321, 18, 31, 17);
    const juce::Rectangle<int> dialBounds[] {
        {34,105,38,38}, {125,63,38,38}, {207,63,38,38}, {296,103,38,38}, {206,142,38,38},
        {34,182,38,38}, {125,220,38,38}, {34,260,38,38}, {125,300,38,38}, {34,340,38,38},
        {125,461,38,38}, {34,579,38,38}, {125,541,38,38}, {296,182,38,38}, {206,221,38,38},
        {206,300,38,38}, {206,541,38,38}
    };
    const juce::Rectangle<int> buttonBounds[] {
        {124,123,40,23}, {124,180,40,23}, {124,602,40,22}, {124,386,23,35},
        {205,386,23,35}, {294,239,42,21}, {211,474,34,19}, {205,602,39,22}
    };
    size_t controlIndex = 0;
    for (const auto& bounds : dialBounds)
        reposition(*controls[controlIndex++], bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight());
    for (const auto& bounds : buttonBounds)
        reposition(*controls[controlIndex++], bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight());
    reposition(*outputFader, 330, 448, 27, 105);
}

void ChannelStripEditor::drawPanelTitle(juce::Graphics& g, juce::Rectangle<float> bounds,
                                        const juce::String& text)
{
    g.setColour(juce::Colour(0xff111414).withAlpha(0.96f));
    g.fillRoundedRectangle(bounds, 1.5f);
    g.setColour(juce::Colour(0xff8a877e).withAlpha(0.72f));
    g.drawRoundedRectangle(bounds, 1.5f, 0.65f);
    g.setColour(juce::Colour(0xffe0d9c8));
    g.setFont(juce::Font(juce::FontOptions("Bahnschrift", 7.5f, juce::Font::bold)));
    g.drawFittedText(text, bounds.toNearestInt(), juce::Justification::centred, 1);
}

void ChannelStripEditor::showAbout()
{
    const auto message =
        "Channel Strip v2.0.4\n\n"
        "Created by Loops, a music-production platform for high-quality, royalty-free "
        "samples. Its catalogue spans hip-hop, house, techno, Afrobeats, and cinematic "
        "sounds, with search, previews, and creator tools.\n\n"
        "Website: loops.co.ke\n\n"
        "About Loops: loops.co.ke/about\n\n"
        "Channel Strip and its source are licensed under GPLv3. This plugin uses JUCE "
        "under its GPLv3 terms. See the included LICENSE file for the full licence.";
    juce::PopupMenu menu;
    menu.addItem(1, "About Loops and Channel Strip");
    menu.addSeparator();
    menu.addItem(2, "Visit Loops.co.ke");
    menu.addItem(3, "GNU GPLv3 licence");
    menu.addItem(4, "JUCE licensing");
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&aboutButton),
        [message](int result)
        {
            if (result == 1)
                juce::AlertWindow::showMessageBoxAsync(
                    juce::MessageBoxIconType::InfoIcon, "About Loops", message, "OK");
            else if (result == 2)
                juce::URL("https://loops.co.ke/").launchInDefaultBrowser();
            else if (result == 3)
                juce::URL("https://www.gnu.org/licenses/gpl-3.0.html").launchInDefaultBrowser();
            else if (result == 4)
                juce::URL("https://juce.com/legal/").launchInDefaultBrowser();
        });
}

void ChannelStripEditor::loadPreset(int index)
{
    processor.setCurrentProgram(index);
}

void ChannelStripEditor::timerCallback()
{
    const auto scale = getWidth() / 366.0f;
    repaint(juce::roundToInt(295 * scale), juce::roundToInt(430 * scale),
            juce::roundToInt(42 * scale), juce::roundToInt(193 * scale));
    const auto selectedId = processor.getCurrentProgram() + 1;
    if (presets.getSelectedId() != selectedId)
        presets.setSelectedId(selectedId, juce::dontSendNotification);
}
