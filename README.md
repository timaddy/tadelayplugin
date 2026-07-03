# Audio Plugin

A JUCE-based audio plugin supporting VST3, AU, and Standalone formats.

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

## Project Structure

```
Source/
├── PluginProcessor.h/.cpp   # Audio processing and parameter state
└── PluginEditor.h/.cpp      # Plugin UI
CMakeLists.txt               # Build configuration
```

## Parameters

| Name | Range | Default | Description |
|------|-------|---------|-------------|
| Gain | 0.0 – 1.0 | 0.5 | Output gain |
