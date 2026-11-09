# Channel Strip

A 64-bit Windows VST3 channel-strip plug-in and standalone app, built with JUCE. The fixed-size 366 x 680 faceplate uses the image supplied for this project; rotary controls, switches, preset selector, output fader, and meters are interactive.

JUCE is distributed under GPLv3 or a commercial license. Select and comply with the applicable JUCE license before distributing builds.

## Features

- High-pass and low-pass filters
- Four-band EQ (low/high shelves and low/high-mid peaking bands)
- Compressor with selectable ratios, threshold, and release
- Gate/expander with threshold, range, and release
- Input/output gain, phase invert, and six factory presets
- Parameter state saved by the host

The processing is a practical channel-strip implementation with conventional filter and dynamics behavior. It is not a bit-for-bit model of any proprietary analog circuit or commercial plug-in.

## Build on Windows

Install Visual Studio 2022 with the **Desktop development with C++** workload and CMake. Then run:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --target ChannelStrip_VST3 ChannelStrip_Standalone
```

The VST3 bundle is generated under `build\ChannelStrip_artefacts\Release\VST3`. Copy `Channel Strip.vst3` to `%CommonProgramFiles%\VST3`, restart FL Studio, and rescan plug-ins. The standalone app is generated in the same artifacts tree.

## Automated build

The `Build Windows plugin` GitHub Actions workflow builds the VST3 and standalone app on Windows and publishes them as a downloadable workflow artifact. Push this project to a GitHub repository, then run the workflow from the Actions tab.

## Controls

Hover over controls in the plug-in for parameter names. Select a preset from the top menu. Frequencies and gains are adjusted by dragging the associated knobs; the output fader adjusts output level.
