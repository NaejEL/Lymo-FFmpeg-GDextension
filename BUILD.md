# Build Instructions

This document provides detailed instructions for building the Lymo FFmpeg GDExtension on Windows and Linux.

## Prerequisites

### Common Requirements
- Python 3.6+
- SCons build system (`pip install scons`)
- Git with submodules support

### Windows
- Visual Studio 2019/2022 with C++ tools, OR
- MinGW-w64 (recommended: MSYS2)
- FFmpeg development libraries

### Linux
- GCC 8+ or Clang 8+
- FFmpeg development libraries
- pkg-config

## Dependencies Setup

### 1. Clone Repository with Submodules
```bash
git clone --recursive https://github.com/NaejEL/Lymo-FFmpeg-GDextension.git
cd Lymo-FFmpeg-GDextension
```

If you already cloned without `--recursive`:
```bash
git submodule update --init --recursive
```

### 2. Build godot-cpp
```bash
cd thirdparty/godot-cpp
scons platform=linux target=template_debug  # or windows
scons platform=linux target=template_release
cd ../..
```

### 3. Install FFmpeg Libraries

#### Windows (MSYS2)
```bash
pacman -S mingw-w64-x86_64-ffmpeg
# Libraries will be in /mingw64/lib and headers in /mingw64/include
```

#### Windows (vcpkg)
```bash
vcpkg install ffmpeg[avcodec,avformat,avutil,swscale,swresample]:x64-windows
```

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libswresample-dev
```

#### CentOS/RHEL/Fedora
```bash
# Enable RPM Fusion repository first
sudo dnf install ffmpeg-devel
```

## Building the Extension

### Linux
```bash
# Debug build
scons platform=linux target=template_debug

# Release build  
scons platform=linux target=template_release

# With custom FFmpeg path
scons platform=linux target=template_debug ffmpeg_path=/path/to/ffmpeg
```

### Windows with MSVC
```bash
# Debug build
scons platform=windows target=template_debug

# Release build
scons platform=windows target=template_release

# With custom FFmpeg path
scons platform=windows target=template_debug ffmpeg_path=C:\vcpkg\installed\x64-windows
```

### Windows with MinGW
```bash
# Debug build
scons platform=windows target=template_debug use_mingw=true

# Release build
scons platform=windows target=template_release use_mingw=true
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `platform` | auto-detect | Target platform (linux/windows/macos) |
| `target` | template_debug | Build target (editor/template_debug/template_release) |
| `arch` | x86_64 | Target architecture (x86_32/x86_64/arm64) |
| `use_mingw` | False | Use MinGW compiler on Windows |
| `use_llvm` | False | Use LLVM/Clang compiler |
| `debug_symbols` | True | Generate debug symbols |
| `optimize` | False | Enable optimizations |
| `godot_cpp_path` | thirdparty/godot-cpp | Path to godot-cpp |
| `ffmpeg_path` | thirdparty/ffmpeg | Path to FFmpeg |

## Hardware Acceleration Support

The extension supports various hardware acceleration methods:

### Windows
- **D3D11VA**: DirectX 11 Video Acceleration
- **DXVA2**: DirectX Video Acceleration 2.0
- **CUDA**: NVIDIA CUDA (requires CUDA toolkit)

### Linux
- **VAAPI**: Video Acceleration API (Intel/AMD)
- **CUDA**: NVIDIA CUDA (requires CUDA toolkit)
- **VDPAU**: NVIDIA Video Decode and Presentation API

### Hardware Acceleration Dependencies

#### NVIDIA CUDA (Optional)
- Download and install CUDA Toolkit from NVIDIA
- Ensure FFmpeg is compiled with CUDA support

#### Intel VAAPI (Linux)
```bash
sudo apt install libva-dev libva-drm2 vainfo
```

## Troubleshooting

### Common Issues

#### "godot-cpp not found"
Make sure to initialize submodules:
```bash
git submodule update --init --recursive
```

#### "FFmpeg libraries not found"
Specify the correct FFmpeg path:
```bash
scons ffmpeg_path=/usr/local
```

#### Linking errors on Windows
Ensure you're using the correct runtime libraries. With MSVC, FFmpeg should be compiled with the same runtime (/MD or /MT).

#### Permission denied on Linux
Make sure the binary has execute permissions:
```bash
chmod +x bin/lymo_ffmpeg.so
```

### Performance Optimization

For best performance in projection mapping:
1. Use hardware acceleration when available
2. Build with optimizations enabled: `optimize=true`
3. Use release builds for production
4. Consider using faster pixel formats (YUV420P vs YUV444P)

### Debug Builds

Debug builds include:
- Detailed logging
- Debug symbols
- Runtime checks
- Better error messages

Use debug builds during development and testing.

## Integration with Godot

1. Copy the built library to your Godot project
2. Copy `lymo_ffmpeg.gdextension` to your project root
3. Enable the plugin in Project Settings
4. The FFmpeg classes will be available in GDScript

See the demo project for usage examples.