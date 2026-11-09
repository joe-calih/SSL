# Loops Channel Strip

Version 2.0.2 is a 64-bit Windows VST3 effect built with JUCE. It preserves the supplied 366 x 680 faceplate and adds a Windows installer, refined machined-metal styling, factory presets, adjustable interface scale, and Loops company information.

Loops describes its service as a curated, royalty-free sample catalog and music-production workflow for producers. Visit [loops.co.ke](https://loops.co.ke/) or its [About page](https://loops.co.ke/about).

## Features

- VST3 plug-in for FL Studio and other VST3-compatible DAWs. Version 2 does not build a standalone app.
- High/low-pass filters, four-band EQ, compressor, gate/expander, input/output gain, and phase invert.
- Lower-cost DSP: EQ filter coefficients are only recalculated when relevant controls change; compressor metering uses a linear-domain curve.
- 18 factory presets: Default; Vocal Lead, Warm, and Air; Drum Bus Glue; Kick Punch; Snare Crack; Drums Parallel; Bass Tight and Warm; Acoustic Guitar; Electric Guitar; Piano Focus; Mix Bus Glue and Clean; Spoken Voice; Synth Forward; Analog Drive.
- UI scale selector from 10% to 200% in 10% steps. The original interface layout is the 100% reference.
- About and licence information with links to Loops and JUCE.
- Windows installer installs/removes the VST3 in the shared 64-bit VST3 directory. The completion page can open Loops.co.ke.
- Windows releases run Tracktion's `pluginval` at strictness level 5 before packaging, validating core plug-in and state save/restore behavior.
- Host automation and saved plug-in state.

The EQ and dynamics are original, conventional DSP implementations; factory presets are starting points, not exact measurements or official presets from another manufacturer. This independent plug-in is not affiliated with or endorsed by Solid State Logic.

## Install on Windows

Download `ChannelStrip-v2.0.2-Windows-Setup.exe` from the [latest GitHub release](https://github.com/joe-calih/SSL/releases/latest), run it, and follow the setup wizard. Administrator permission is required to install into the shared VST3 folder. Rescan plug-ins in the DAW if it does not appear automatically. Uninstall it through Windows Installed Apps.

## Build on Windows

Install Visual Studio with its **Desktop development with C++** workload and CMake 3.22 or later. Then run:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --target ChannelStrip_VST3
```

For the installer, also install Inno Setup 6 and run:

```powershell
iscc /DAppVersion=2.0.2 installer\ChannelStrip.iss
```

The build produces the VST3 under `build\ChannelStrip_artefacts\Release\VST3`. The setup executable is generated in `artifacts`.

## Releases and maintenance

The GitHub Actions workflow builds the VST3 and creates an installer and GitHub release for `v*` tags. It also uploads the installer as a workflow artifact. Update the version consistently in `CMakeLists.txt`, `installer\ChannelStrip.iss`, and the workflow before making the next version tag.

Channel Strip and the corresponding source in this repository are licensed under GPLv3; see [`LICENSE`](LICENSE). This build uses JUCE 8.0.8 under GPLv3; see [`installer\Licenses.txt`](installer/Licenses.txt) for third-party notices and official licence links.
