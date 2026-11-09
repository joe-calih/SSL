#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
enum FilterType
{
    lowPassType,
    highPassType,
    peakType,
    lowShelfType,
    highShelfType
};

struct FactoryPreset
{
    float hp, lp, hf, hfGain, hm, hmGain, lm, lmGain, lf, lfGain;
    int ratio;
    float threshold, release, gateThreshold, gateRange, gateRelease;
    bool split = true;
    bool analog = false;
};

constexpr FactoryPreset factoryPresets[] {
    { 20, 20000, 8000, 0, 3000, 0, 500, 0, 100, 0, 0, -10, 0.3f, -30, 20, 0.2f },
    { 85, 20000, 12000, 2.0f, 4200, 2.5f, 280, -2.0f, 110, 1.5f, 2, -18, 0.45f, -48, 12, 0.18f },
    { 75, 18000, 10000, 1.0f, 2800, 1.5f, 320, -1.0f, 140, 1.0f, 2, -20, 0.5f, -48, 10, 0.2f },
    { 70, 20000, 15000, 2.5f, 5200, 1.0f, 300, -1.0f, 120, 0.5f, 1, -22, 0.6f, -50, 8, 0.25f },
    { 30, 18000, 10000, 1.0f, 3200, 1.5f, 250, -1.5f, 90, 2.0f, 2, -14, 0.25f, -42, 18, 0.16f },
    { 28, 18000, 9000, 0.5f, 4500, 1.0f, 180, -1.0f, 65, 2.0f, 3, -16, 0.12f, -40, 24, 0.12f },
    { 45, 16000, 9500, 1.5f, 6000, 2.0f, 250, -2.5f, 100, 1.0f, 3, -18, 0.18f, -36, 30, 0.1f },
    { 50, 18000, 11000, 2.0f, 4200, 2.0f, 220, -2.0f, 90, 1.5f, 4, -22, 0.1f, -40, 28, 0.08f },
    { 35, 16000, 9500, 1.0f, 3500, 1.0f, 200, -1.0f, 90, 1.0f, 4, -24, 0.12f, -32, 35, 0.08f },
    { 32, 17000, 12000, 0.5f, 4000, 0.5f, 180, -0.5f, 80, 1.5f, 2, -20, 0.25f, -40, 18, 0.15f },
    { 85, 18000, 12000, 1.5f, 5000, 1.5f, 280, -1.0f, 130, 1.0f, 2, -22, 0.35f, -55, 8, 0.2f },
    { 70, 20000, 10000, 0.0f, 3000, 0.0f, 250, 0.0f, 100, 0.0f, 1, -12, 0.45f, -60, 0, 0.2f },
    { 50, 18000, 14000, 1.0f, 5000, 1.0f, 350, -1.0f, 100, 1.0f, 2, -18, 0.4f, -60, 0, 0.2f },
    { 70, 16000, 10000, 2.0f, 4800, 3.0f, 250, -2.0f, 90, 2.0f, 3, -20, 0.12f, -38, 28, 0.08f },
    { 20, 20000, 9000, 1.0f, 2500, 0.0f, 350, 0.0f, 120, 1.0f, 3, -16, 0.2f, -45, 22, 0.12f },
    { 25, 18000, 10500, 2.0f, 5000, 2.5f, 240, -1.5f, 80, 2.5f, 4, -20, 0.1f, -34, 32, 0.06f },
    { 30, 18000, 12000, 1.0f, 4000, 1.0f, 300, -1.0f, 100, 1.0f, 2, -18, 0.3f, -45, 20, 0.15f },
    { 30, 18000, 10500, 2.0f, 5000, 2.5f, 240, -1.5f, 80, 2.5f, 4, -20, 0.1f, -34, 32, 0.06f, true, true }
};

constexpr const char* factoryPresetNames[] {
    "Default", "Vocal - Lead", "Vocal - Warm", "Vocal - Air",
    "Drum Bus - Glue", "Kick - Punch", "Snare - Crack", "Drums - Parallel",
    "Bass - Tight", "Bass - Warm", "Acoustic Guitar", "Electric Guitar",
    "Piano - Focus", "Mix Bus - Glue", "Mix Bus - Clean", "Spoken Voice",
    "Synth - Forward", "Analog - Drive"
};
static_assert(sizeof(factoryPresets) / sizeof(factoryPresets[0]) == 18);
static_assert(sizeof(factoryPresetNames) / sizeof(factoryPresetNames[0]) == 18);

float decibelsToGain(float db) noexcept
{
    return std::pow(10.0f, db / 20.0f);
}

float smoothCoefficient(float timeSeconds, double sampleRate)
{
    return timeSeconds <= 0.0f ? 0.0f
                              : std::exp(-1.0f / (timeSeconds * static_cast<float>(sampleRate)));
}
}

void ChannelStripProcessor::Biquad::configure(int type, double sampleRate, float frequency,
                                              float q, float gainDb)
{
    const auto nyquist = static_cast<float>(sampleRate * 0.49);
    const auto f = juce::jlimit(10.0f, nyquist, frequency);
    const auto w0 = juce::MathConstants<float>::twoPi * f / static_cast<float>(sampleRate);
    const auto cosine = std::cos(w0);
    const auto sine = std::sin(w0);
    const auto alpha = sine / (2.0f * juce::jmax(0.1f, q));
    const auto a = decibelsToGain(gainDb * 0.5f);
    float numerator0 = 1.0f, numerator1 = 0.0f, numerator2 = 0.0f;
    float denominator0 = 1.0f, denominator1 = 0.0f, denominator2 = 0.0f;

    switch (type)
    {
        case lowPassType:
            numerator0 = (1.0f - cosine) * 0.5f;
            numerator1 = 1.0f - cosine;
            numerator2 = numerator0;
            denominator0 = 1.0f + alpha;
            denominator1 = -2.0f * cosine;
            denominator2 = 1.0f - alpha;
            break;
        case highPassType:
            numerator0 = (1.0f + cosine) * 0.5f;
            numerator1 = -(1.0f + cosine);
            numerator2 = numerator0;
            denominator0 = 1.0f + alpha;
            denominator1 = -2.0f * cosine;
            denominator2 = 1.0f - alpha;
            break;
        case peakType:
            numerator0 = 1.0f + alpha * a;
            numerator1 = -2.0f * cosine;
            numerator2 = 1.0f - alpha * a;
            denominator0 = 1.0f + alpha / a;
            denominator1 = -2.0f * cosine;
            denominator2 = 1.0f - alpha / a;
            break;
        case lowShelfType:
        case highShelfType:
        {
            const auto shelfAlpha = sine * 0.5f * std::sqrt(2.0f);
            const auto beta = 2.0f * std::sqrt(a) * shelfAlpha;
            if (type == lowShelfType)
            {
                numerator0 = a * ((a + 1.0f) - (a - 1.0f) * cosine + beta);
                numerator1 = 2.0f * a * ((a - 1.0f) - (a + 1.0f) * cosine);
                numerator2 = a * ((a + 1.0f) - (a - 1.0f) * cosine - beta);
                denominator0 = (a + 1.0f) + (a - 1.0f) * cosine + beta;
                denominator1 = -2.0f * ((a - 1.0f) + (a + 1.0f) * cosine);
                denominator2 = (a + 1.0f) + (a - 1.0f) * cosine - beta;
            }
            else
            {
                numerator0 = a * ((a + 1.0f) + (a - 1.0f) * cosine + beta);
                numerator1 = -2.0f * a * ((a - 1.0f) + (a + 1.0f) * cosine);
                numerator2 = a * ((a + 1.0f) + (a - 1.0f) * cosine - beta);
                denominator0 = (a + 1.0f) - (a - 1.0f) * cosine + beta;
                denominator1 = 2.0f * ((a - 1.0f) - (a + 1.0f) * cosine);
                denominator2 = (a + 1.0f) - (a - 1.0f) * cosine - beta;
            }
            break;
        }
    }

    const auto inverseA0 = 1.0f / denominator0;
    b0 = numerator0 * inverseA0;
    b1 = numerator1 * inverseA0;
    b2 = numerator2 * inverseA0;
    a1 = denominator1 * inverseA0;
    a2 = denominator2 * inverseA0;
}

void ChannelStripProcessor::Biquad::processSamples(float* samples, int count) noexcept
{
    for (int i = 0; i < count; ++i)
    {
        const auto input = samples[i];
        const auto output = b0 * input + z1;
        z1 = b1 * input - a1 * output + z2;
        z2 = b2 * input - a2 * output;
        samples[i] = output;
    }
}

ChannelStripProcessor::ChannelStripProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout ChannelStripProcessor::createParameterLayout()
{
    using Float = juce::AudioParameterFloat;
    using Bool = juce::AudioParameterBool;
    using Choice = juce::AudioParameterChoice;
    using Range = juce::NormalisableRange<float>;
    using ID = juce::ParameterID;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    const auto addFloat = [&layout](const char* id, const char* name, float minimum,
                                    float maximum, float initial, float skew = 1.0f)
    {
        layout.add(std::make_unique<Float>(ID { id, 1 }, name,
                                           Range(minimum, maximum, 0.0f, skew), initial));
    };
    const auto addBool = [&layout](const char* id, const char* name, bool initial)
    {
        layout.add(std::make_unique<Bool>(ID { id, 1 }, name, initial));
    };

    addFloat("input", "Input", -18.0f, 18.0f, 0.0f);
    addFloat("output", "Output", -18.0f, 18.0f, 0.0f);
    addFloat("hpFreq", "High-pass frequency", 20.0f, 350.0f, 20.0f, 0.55f);
    addFloat("lpFreq", "Low-pass frequency", 1000.0f, 20000.0f, 20000.0f, 0.45f);
    addFloat("hfFreq", "High frequency", 2000.0f, 20000.0f, 8000.0f, 0.45f);
    addFloat("hfGain", "High gain", -18.0f, 18.0f, 0.0f);
    addFloat("hmFreq", "High-mid frequency", 600.0f, 8000.0f, 3000.0f, 0.5f);
    addFloat("hmGain", "High-mid gain", -6.0f, 6.0f, 0.0f);
    addFloat("lmFreq", "Low-mid frequency", 100.0f, 1500.0f, 500.0f, 0.5f);
    addFloat("lmGain", "Low-mid gain", -6.0f, 6.0f, 0.0f);
    addFloat("lfFreq", "Low frequency", 30.0f, 450.0f, 100.0f, 0.5f);
    addFloat("lfGain", "Low gain", -18.0f, 18.0f, 0.0f);
    layout.add(std::make_unique<Choice>(
        ID { "compRatio", 1 }, "Compressor ratio",
        juce::StringArray { "1:1", "2:1", "3:1", "4:1", "10:1", "Limit" }, 0));
    addFloat("compThreshold", "Compressor threshold", -30.0f, 0.0f, -10.0f);
    addFloat("compRelease", "Compressor release", 0.1f, 1.2f, 0.3f, 0.6f);
    addFloat("gateThreshold", "Gate threshold", -60.0f, 0.0f, -30.0f);
    addFloat("gateRange", "Gate range", 0.0f, 40.0f, 20.0f);
    addFloat("gateRelease", "Gate release", 0.05f, 1.0f, 0.2f, 0.6f);

    addBool("split", "Split filter path", false);
    addBool("eqBypass", "EQ bypass", false);
    addBool("compBypass", "Compressor bypass", false);
    addBool("gateOn", "Gate enabled", true);
    addBool("analog", "Analog coloration", false);
    addBool("phase", "Phase invert", false);
    addBool("hmRange", "High-mid x3 range", false);
    addBool("lmRange", "Low-mid x3 range", false);
    return layout;
}

void ChannelStripProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    lastFilterSettings.fill(std::numeric_limits<float>::quiet_NaN());
    for (auto& filter : highPass) filter.reset();
    for (auto& filter : lowPass) filter.reset();
    for (auto& filter : highShelf) filter.reset();
    for (auto& filter : highMid) filter.reset();
    for (auto& filter : lowMid) filter.reset();
    for (auto& filter : lowShelf) filter.reset();
    compressorEnvelope.fill(0.0f);
    gateEnvelope.fill(0.0f);
    gateGain.fill(1.0f);
    updateFilters();
}

void ChannelStripProcessor::releaseResources()
{
}

bool ChannelStripProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto output = layouts.getMainOutputChannelSet();
    return output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo();
}

float ChannelStripProcessor::parameter(const char* id) const
{
    return parameters.getRawParameterValue(id)->load();
}

void ChannelStripProcessor::updateFilters()
{
    const auto hpFrequency = parameter("hpFreq");
    const auto lpFrequency = parameter("lpFreq");
    const auto hfFrequency = parameter("hfFreq");
    const auto hfGain = parameter("hfGain");
    const auto hmFrequency = parameter("hmFreq");
    const auto hmGain = parameter("hmGain");
    const auto lmFrequency = parameter("lmFreq");
    const auto lmGain = parameter("lmGain");
    const auto lfFrequency = parameter("lfFreq");
    const auto lfGain = parameter("lfGain");
    const auto hmRange = parameter("hmRange");
    const auto lmRange = parameter("lmRange");
    const std::array<float, 12> settings {
        hpFrequency, lpFrequency, hfFrequency, hfGain, hmFrequency, hmGain,
        lmFrequency, lmGain, lfFrequency, lfGain, hmRange, lmRange
    };
    if (settings == lastFilterSettings)
        return;

    lastFilterSettings = settings;
    for (int channel = 0; channel < 2; ++channel)
    {
        highPass[channel].configure(highPassType, currentSampleRate, hpFrequency, 0.707f, 0.0f);
        lowPass[channel].configure(lowPassType, currentSampleRate, lpFrequency, 0.707f, 0.0f);
        highShelf[channel].configure(highShelfType, currentSampleRate, hfFrequency, 0.707f, hfGain);
        highMid[channel].configure(peakType, currentSampleRate, hmFrequency, 1.0f,
                                   hmGain * (hmRange > 0.5f ? 3.0f : 1.0f));
        lowMid[channel].configure(peakType, currentSampleRate, lmFrequency, 1.0f,
                                  lmGain * (lmRange > 0.5f ? 3.0f : 1.0f));
        lowShelf[channel].configure(lowShelfType, currentSampleRate, lfFrequency, 0.707f, lfGain);
    }
}

void ChannelStripProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const auto channels = juce::jmin(buffer.getNumChannels(), 2);
    const auto samples = buffer.getNumSamples();
    if (channels == 0 || samples == 0)
        return;

    updateFilters();

    const auto splitEnabled = parameter("split") > 0.5f;
    const auto eqEnabled = parameter("eqBypass") < 0.5f;
    const auto compEnabled = parameter("compBypass") < 0.5f;
    const auto gateEnabled = parameter("gateOn") > 0.5f;
    const auto analogEnabled = parameter("analog") > 0.5f;
    const auto phase = parameter("phase") > 0.5f ? -1.0f : 1.0f;
    const auto inputGain = decibelsToGain(parameter("input")) * phase;
    const auto outputGain = decibelsToGain(parameter("output"));
    const auto ratioChoice = juce::roundToInt(parameter("compRatio") * 5.0f);
    constexpr float ratios[] { 1.0f, 2.0f, 3.0f, 4.0f, 10.0f, 20.0f };
    const auto ratio = ratios[juce::jlimit(0, 5, ratioChoice)];
    const auto compThreshold = parameter("compThreshold");
    const auto compRelease = parameter("compRelease");
    const auto thresholdGain = decibelsToGain(compThreshold);
    const auto compressionCurve = 1.0f - 1.0f / ratio;
    const auto gateThreshold = parameter("gateThreshold");
    const auto gateRange = parameter("gateRange");
    const auto gateRelease = parameter("gateRelease");
    const auto attackCoefficient = smoothCoefficient(0.01f, currentSampleRate);
    const auto compReleaseCoefficient = smoothCoefficient(compRelease, currentSampleRate);
    const auto gateOpenCoefficient = smoothCoefficient(0.002f, currentSampleRate);
    const auto gateCloseCoefficient = smoothCoefficient(gateRelease, currentSampleRate);
    const auto gateThresholdGain = decibelsToGain(gateThreshold);
    const auto gateFloor = decibelsToGain(-gateRange);
    float maximumInput = 0.0f;
    float maximumOutput = 0.0f;

    for (int channel = 0; channel < channels; ++channel)
    {
        auto* data = buffer.getWritePointer(channel);
        for (int i = 0; i < samples; ++i)
        {
            data[i] *= inputGain;
            maximumInput = juce::jmax(maximumInput, std::abs(data[i]));
        }

        if (splitEnabled)
        {
            highPass[channel].processSamples(data, samples);
            lowPass[channel].processSamples(data, samples);
        }
        if (eqEnabled)
        {
            highShelf[channel].processSamples(data, samples);
            highMid[channel].processSamples(data, samples);
            lowMid[channel].processSamples(data, samples);
            lowShelf[channel].processSamples(data, samples);
        }

        auto& compEnv = compressorEnvelope[static_cast<size_t>(channel)];
        auto& gateEnv = gateEnvelope[static_cast<size_t>(channel)];
        auto& gateGainForChannel = gateGain[static_cast<size_t>(channel)];
        for (int i = 0; i < samples; ++i)
        {
            auto sample = data[i];
            const auto magnitude = std::abs(sample);
            const auto compCoefficient = magnitude > compEnv ? attackCoefficient : compReleaseCoefficient;
            compEnv = compCoefficient * compEnv + (1.0f - compCoefficient) * magnitude;

            if (compEnabled && ratio > 1.0f && compEnv > 1.0e-8f)
            {
                if (compEnv > thresholdGain)
                    sample *= std::pow(thresholdGain / compEnv, compressionCurve);
            }

            if (gateEnabled)
            {
                const auto gateCoefficient = magnitude > gateEnv ? gateOpenCoefficient : gateCloseCoefficient;
                gateEnv = gateCoefficient * gateEnv + (1.0f - gateCoefficient) * magnitude;
                const auto targetGain = gateEnv < gateThresholdGain ? gateFloor : 1.0f;
                const auto gainCoefficient = targetGain > gateGainForChannel
                    ? gateOpenCoefficient : gateCloseCoefficient;
                gateGainForChannel = gainCoefficient * gateGainForChannel
                                   + (1.0f - gainCoefficient) * targetGain;
                sample *= gateGainForChannel;
            }

            if (analogEnabled)
                sample = std::tanh(sample * 1.05f) / std::tanh(1.05f);
            data[i] = sample * outputGain;
            maximumOutput = juce::jmax(maximumOutput, std::abs(data[i]));
        }
    }

    inputLevelDb.store(juce::Decibels::gainToDecibels(maximumInput, -60.0f));
    outputLevelDb.store(juce::Decibels::gainToDecibels(maximumOutput, -60.0f));
}

juce::AudioProcessorEditor* ChannelStripProcessor::createEditor()
{
    return new ChannelStripEditor(*this);
}

void ChannelStripProcessor::setCurrentProgram(int index)
{
    loadFactoryPreset(index);
}

void ChannelStripProcessor::loadFactoryPreset(int index)
{
    currentProgram = juce::jlimit(0, getNumPrograms() - 1, index);
    const auto& preset = factoryPresets[currentProgram];
    const auto set = [this](const char* id, float value)
    {
        if (auto* parameter = parameters.getParameter(id))
            parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
    };
    set("hpFreq", preset.hp);
    set("lpFreq", preset.lp);
    set("hfFreq", preset.hf);
    set("hfGain", preset.hfGain);
    set("hmFreq", preset.hm);
    set("hmGain", preset.hmGain);
    set("lmFreq", preset.lm);
    set("lmGain", preset.lmGain);
    set("lfFreq", preset.lf);
    set("lfGain", preset.lfGain);
    set("compRatio", static_cast<float>(preset.ratio));
    set("compThreshold", preset.threshold);
    set("compRelease", preset.release);
    set("gateThreshold", preset.gateThreshold);
    set("gateRange", preset.gateRange);
    set("gateRelease", preset.gateRelease);
    set("split", preset.split ? 1.0f : 0.0f);
    set("analog", preset.analog ? 1.0f : 0.0f);
    set("eqBypass", 0.0f);
    set("compBypass", 0.0f);
    set("gateOn", 1.0f);
    set("input", 0.0f);
    set("output", 0.0f);
}

const juce::String ChannelStripProcessor::getProgramName(int index)
{
    return factoryPresetNames[juce::jlimit(0, getNumPrograms() - 1, index)];
}

void ChannelStripProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    auto state = parameters.copyState();
    state.setProperty("factoryProgram", currentProgram, nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destination);
}

void ChannelStripProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
    {
        const auto restoredState = juce::ValueTree::fromXml(*xml);
        currentProgram = juce::jlimit(0, getNumPrograms() - 1,
                                      static_cast<int>(restoredState.getProperty("factoryProgram", 0)));
        parameters.replaceState(restoredState);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChannelStripProcessor();
}
