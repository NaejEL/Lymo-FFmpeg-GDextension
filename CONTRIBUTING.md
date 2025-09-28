# Contributing to Lymo FFmpeg GDExtension

Thank you for your interest in contributing! This document provides guidelines for contributing to the project.

## Development Setup

1. Fork the repository
2. Clone your fork with submodules:
   ```bash
   git clone --recursive https://github.com/yourusername/Lymo-FFmpeg-GDextension.git
   ```
3. Set up the development environment following [BUILD.md](BUILD.md)

## Code Style

### C++
- Follow Google C++ Style Guide
- Use 4-space indentation
- Class names in PascalCase
- Function names in snake_case
- Private members with underscore suffix

### GDScript
- Follow Godot's GDScript style guide
- Use snake_case for variables and functions
- Use PascalCase for classes

### Commit Messages
- Use conventional commit format: `type(scope): description`
- Types: feat, fix, docs, style, refactor, test, chore
- Keep first line under 50 characters
- Include body for complex changes

Example:
```
feat(decoder): add hardware acceleration support

- Implement CUDA, VAAPI, and D3D11VA backends
- Add automatic hardware detection
- Fallback to software decoding when needed
```

## Pull Request Process

1. Create a feature branch from `main`
2. Make your changes with appropriate tests
3. Update documentation if needed
4. Ensure all tests pass
5. Submit a pull request

### PR Requirements
- [ ] Code follows style guidelines
- [ ] Changes are tested
- [ ] Documentation is updated
- [ ] Commit history is clean
- [ ] PR description explains the changes

## Testing

### Building Tests
```bash
scons platform=linux target=template_debug debug_symbols=true
```

### Manual Testing
Test with various video formats and configurations:
- Different codecs (H.264, H.265, VP9, AV1)
- Various resolutions (720p, 1080p, 4K)
- Different pixel formats (YUV420P, YUV444P, RGB)
- Hardware acceleration on/off

### Performance Testing
For projection mapping, verify:
- Real-time playback at target framerates
- Low latency decoding
- Memory usage stability
- GPU acceleration effectiveness

## Bug Reports

When reporting bugs, include:
- Operating system and version
- Godot version
- FFmpeg version
- Hardware information (GPU, drivers)
- Video file details (codec, resolution, format)
- Steps to reproduce
- Expected vs actual behavior
- Console output/logs

## Feature Requests

For new features:
- Explain the use case
- Describe the proposed solution
- Consider performance implications
- Discuss API design if applicable

## Areas for Contribution

### High Priority
- Audio synchronization
- Network streaming support
- Additional hardware acceleration backends
- Performance optimizations
- Documentation improvements

### Medium Priority
- Additional video formats
- Advanced color space handling
- GPU texture upload optimizations
- Memory pool management

### Low Priority
- Video filters and effects
- Subtitle support
- Metadata extraction
- Thumbnail generation

## Code Review

All contributions go through code review:
- Maintainers will review within 48 hours
- Address feedback promptly
- Be open to suggestions and changes
- Help review other contributors' PRs

## Community

- Be respectful and inclusive
- Help newcomers get started
- Share knowledge and best practices
- Follow the code of conduct

## License

By contributing, you agree that your contributions will be licensed under the MIT License.