# Delay + LPF

A JUCE-based stereo delay plugin with a feedback low-pass filter. Supports VST3, AU, and Standalone formats.

## Requirements

- CMake 3.22+
- C++17 compiler (Xcode on macOS, MSVC on Windows, GCC/Clang on Linux)
- Git (to fetch JUCE via FetchContent)

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The built plugin will be copied to your system plugin folder automatically.

## Parameters

| Name       | Range              | Default | Description                          |
|------------|--------------------|---------|--------------------------------------|
| Delay Time | 0.01 s – 2.0 s     | 0.5 s   | Time between the dry and delayed signal |
| Feedback   | 0% – 95%           | 40%     | How much of the delayed signal feeds back |
| LPF Cutoff | 200 Hz – 20 kHz    | 4 kHz   | Low-pass filter on the feedback path |
| Mix        | 0.0 – 1.0          | 0.5     | Dry/wet blend                        |

## Project Structure

```
Source/
├── PluginProcessor.h/.cpp   # Delay + LPF DSP and parameter state
└── PluginEditor.h/.cpp      # Four-knob UI
CMakeLists.txt               # Build configuration (JUCE via FetchContent)
```
