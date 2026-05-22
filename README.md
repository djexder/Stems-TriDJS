# TriDJs Stems Suite

AI-powered audio stem separation for music producers, DJs, and creators.

**TriDJs Stems Suite** is a Windows desktop application built with [JUCE 7](https://juce.com) (C++17) and [LibTorch](https://pytorch.org) that uses the **HTDemucs** neural network model to separate mixed audio into individual stems (vocals, drums, bass, and other instruments).

## Features

- AI stem separation using a trained HTDemucs model
- Real-time audio playback of separated stems
- Built with JUCE audio framework for low-latency processing
- Standalone Windows desktop application
- Inno Setup installer for easy deployment

## Technologies

| Component | Technology |
|-----------|-----------|
| GUI / Audio Framework | [JUCE 7.0.9](https://juce.com) (C++17) |
| AI Inference | [LibTorch](https://pytorch.org/cppdocs) (PyTorch C++ API) |
| AI Model | HTDemucs (hybrid transformer demucs) |
| Math Libraries | Intel MKL + OpenMP |
| Build System | [CMake](https://cmake.org) (≥ 3.18) |
| Installer | [Inno Setup](https://jrsoftware.org/isinfo.php) |

## Project Structure

```
├── CMakeLists.txt          # CMake build configuration
├── setup.iss               # Inno Setup installer script
├── Source/                 # C++ source files
│   ├── Main.cpp
│   ├── MainComponent.cpp/h
│   ├── StemEngine.cpp/h    # AI stem separation engine
│   └── UpdateChecker.cpp/h
├── libtorch/               # LibTorch redistributable
├── modelo/                 # Trained AI model
│   └── htdemucs_compilado.pt
├── redist/                 # Intel MKL runtime DLLs
└── build/                  # Build output (generated)
```

## Building

### Prerequisites

- [Visual Studio 2022](https://visualstudio.microsoft.com) with C++ desktop workload
- [CMake](https://cmake.org/download) ≥ 3.18
- [LibTorch](https://pytorch.org) (included in `libtorch/`)

### Build Steps

```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The executable and all required DLLs will be in `build/TriDJs_Separador_Stems_artefacts/Release/`.

## Installer

To generate the installer, open `setup.iss` with [Inno Setup](https://jrsoftware.org/isinfo.php) and compile it. Output goes to `installer/`.

## License

This project is licensed under the MIT License. See [LICENSE.txt](LICENSE.txt). See [TERMS_OF_USE.md](TERMS_OF_USE.md) for usage terms.

---

This project uses [SignPath Foundation](https://signpath.io) for free code signing services.

Built by [TriDJs](https://www.tridjs.com.br)
