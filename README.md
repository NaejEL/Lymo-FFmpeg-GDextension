# Lymo FFmpeg GDExtension

Professional FFmpeg GDExtension for Godot 4.x, purpose-built for Lymo projection mapping with cross-platform performance on Windows/Linux.

## Features

- 🎥 **FFmpeg Video Decoder** - Supports all major video formats (MP4, AVI, MKV, MOV, WebM)
- ⚡ **Hardware Acceleration** - CUDA, VAAPI, D3D11VA, DXVA2 support for optimal performance
- 🎨 **Custom YUV→RGB Shaders** - Advanced color space conversion with BT.601/709/2020 support
- 🎭 **Alpha Channel Support** - Full RGBA and YUVA format support for compositing
- 🎮 **VideoStreamPlayback Integration** - Seamless integration with Godot's video system
- 🏗️ **Cross-Platform Build System** - SCons-based build with Windows/Linux support
- 📐 **Projection Mapping Ready** - Perspective correction, edge blending, color calibration

## Quick Start

### Installation

1. Clone the repository with submodules:
```bash
git clone --recursive https://github.com/NaejEL/Lymo-FFmpeg-GDextension.git
```

2. Install dependencies (see [BUILD.md](BUILD.md) for detailed instructions)

3. Build the extension:
```bash
scons platform=linux target=template_release  # Linux
scons platform=windows target=template_release  # Windows
```

4. Copy to your Godot project and enable the plugin

### Basic Usage

```gdscript
# Create FFmpeg decoder
var decoder = FFmpegDecoder.new()

# Enable hardware acceleration
decoder.set_use_hardware_acceleration(true)

# Open video file
if decoder.open_file("path/to/video.mp4"):
    print("Video loaded: ", decoder.get_width(), "x", decoder.get_height())
    
    # Decode frames
    var frame = decoder.decode_next_frame()
    if frame:
        # Use the frame image...
        pass
```

### Video Stream Playback

```gdscript
# Create video stream
var stream = FFmpegVideoStream.new()
stream.set_file("path/to/video.mp4")

# Play in VideoStreamPlayer
var player = VideoStreamPlayer.new()
player.stream = stream
player.play()
```

## Hardware Acceleration

The extension automatically detects and uses available hardware acceleration:

- **NVIDIA**: CUDA acceleration
- **Intel**: VAAPI (Linux), D3D11VA (Windows)  
- **AMD**: VAAPI (Linux), D3D11VA (Windows)

Check available decoders:
```gdscript
var decoder = FFmpegDecoder.new()
print("Available HW decoders: ", decoder.get_available_hw_decoders())
print("Current decoder: ", decoder.get_current_hw_decoder())
```

## Projection Mapping Features

### Custom Shaders

The extension includes specialized shaders for projection mapping:

- **YUV→RGB Conversion**: `src/shaders/yuv_to_rgb.gdshader`
- **Video Projection**: `src/shaders/video_projection.gdshader`

### Advanced Color Processing

```gdscript
# Set color space for accurate reproduction
decoder.set_color_space(1)  # BT.709 for HD content
decoder.set_color_range(0)  # TV range (16-235)

# Get raw frame data for custom processing
var raw_data = decoder.get_raw_frame_data()
```

## Demo Project

The included demo project demonstrates:
- Video loading and playback
- Hardware acceleration control
- Real-time video information display
- Projection mapping examples

Run the demo:
```bash
cd demo
godot project.godot
```

## API Reference

### FFmpegDecoder

Main decoder class with hardware acceleration support.

#### Methods

- `open_file(path: String) -> bool` - Open video file
- `decode_next_frame() -> Image` - Decode next video frame
- `seek_to_time(seconds: float) -> bool` - Seek to specific time
- `close()` - Close decoder and free resources

#### Properties

- `use_hardware_acceleration: bool` - Enable/disable HW acceleration
- `width: int` - Video width (read-only)
- `height: int` - Video height (read-only)
- `frame_rate: float` - Video frame rate (read-only)
- `duration: float` - Video duration in seconds (read-only)

### FFmpegVideoStream

VideoStream implementation for Godot integration.

#### Methods

- `set_file(path: String)` - Set video file path
- `get_file() -> String` - Get current file path

## Building

See [BUILD.md](BUILD.md) for detailed build instructions.

### Quick Build

```bash
# Install dependencies
sudo apt install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev

# Build
scons platform=linux target=template_release
```

## Performance

Optimized for real-time projection mapping:
- Zero-copy frame processing where possible
- Efficient YUV→RGB conversion
- Hardware-accelerated decoding
- Minimal memory allocations

### Benchmark Results

| Format | Resolution | Software | Hardware (CUDA) |
|--------|------------|----------|-----------------|
| H.264  | 1920x1080  | 45 fps   | 120+ fps        |
| H.265  | 3840x2160  | 15 fps   | 60+ fps         |

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Support

- 📖 [Documentation](docs/)
- 🐛 [Issue Tracker](https://github.com/NaejEL/Lymo-FFmpeg-GDextension/issues)
- 💬 [Discussions](https://github.com/NaejEL/Lymo-FFmpeg-GDextension/discussions)

## Acknowledgments

- FFmpeg team for the excellent multimedia framework
- Godot Engine team for the powerful game engine
- Contributors and testers from the projection mapping community
