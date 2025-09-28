#!/usr/bin/env python3

import os
import platform
import subprocess
from pathlib import Path

# Initialize environment
env = Environment()

# Platform detection
def get_platform():
    if platform.system() == "Windows":
        return "windows"
    elif platform.system() == "Linux":
        return "linux"
    elif platform.system() == "Darwin":
        return "macos"
    else:
        return "unknown"

# Build configuration
opts = Variables()
opts.Add(EnumVariable("platform", "Target platform", get_platform(), ["windows", "linux", "macos"]))
opts.Add(EnumVariable("target", "Compilation target", "template_debug", ["editor", "template_release", "template_debug"]))
opts.Add(EnumVariable("arch", "Target architecture", "x86_64", ["x86_32", "x86_64", "arm64"]))
opts.Add(BoolVariable("use_mingw", "Use MinGW on Windows", False))
opts.Add(BoolVariable("use_llvm", "Use LLVM compiler", False))
opts.Add(BoolVariable("debug_symbols", "Generate debug symbols", True))
opts.Add(BoolVariable("optimize", "Enable optimizations", False))
opts.Add(PathVariable("godot_cpp_path", "Path to godot-cpp", "thirdparty/godot-cpp", PathVariable.PathAccept))
opts.Add(PathVariable("ffmpeg_path", "Path to FFmpeg", "thirdparty/ffmpeg", PathVariable.PathAccept))

opts.Update(env)
Help(opts.GenerateHelpText(env))

# Godot-cpp integration
if not os.path.exists(env["godot_cpp_path"]):
    print("Error: godot-cpp not found at", env["godot_cpp_path"])
    print("Please run: git submodule update --init --recursive")
    Exit(1)

# Platform-specific configuration
if env["platform"] == "windows":
    if env["use_mingw"]:
        env.Tool("mingw")
        env.Append(CPPDEFINES=["WIN32", "_WIN32", "_WINDOWS"])
        env.Append(CCFLAGS=["-Wall", "-Wextra"])
        if env["arch"] == "x86_64":
            env.Append(CCFLAGS=["-m64"])
            env.Append(LINKFLAGS=["-m64"])
        else:
            env.Append(CCFLAGS=["-m32"])
            env.Append(LINKFLAGS=["-m32"])
    else:
        env.Tool("msvc")
        env.Append(CPPDEFINES=["WIN32", "_WIN32", "_WINDOWS", "_CRT_SECURE_NO_WARNINGS"])
        env.Append(CCFLAGS=["/W3"])
        if env["arch"] == "x86_64":
            env.Append(CCFLAGS=["/favor:AMD64"])

elif env["platform"] == "linux":
    env.Tool("gcc")
    env.Append(CPPDEFINES=["LINUX"])
    env.Append(CCFLAGS=["-Wall", "-Wextra", "-fPIC"])
    if env["arch"] == "x86_64":
        env.Append(CCFLAGS=["-m64"])
        env.Append(LINKFLAGS=["-m64"])
    else:
        env.Append(CCFLAGS=["-m32"])
        env.Append(LINKFLAGS=["-m32"])

# Build configuration
if env["target"] == "template_debug":
    env.Append(CPPDEFINES=["DEBUG_ENABLED", "DEBUG_METHODS_ENABLED"])
    if env["debug_symbols"]:
        if env["platform"] == "windows" and not env["use_mingw"]:
            env.Append(CCFLAGS=["/Zi"])
            env.Append(LINKFLAGS=["/DEBUG"])
        else:
            env.Append(CCFLAGS=["-g"])
elif env["target"] == "template_release":
    env.Append(CPPDEFINES=["NDEBUG"])
    if env["optimize"]:
        if env["platform"] == "windows" and not env["use_mingw"]:
            env.Append(CCFLAGS=["/O2"])
        else:
            env.Append(CCFLAGS=["-O3"])

# C++ standard
env.Append(CPPDEFINES=["GDEXTENSION"])
if env["platform"] == "windows" and not env["use_mingw"]:
    env.Append(CCFLAGS=["/std:c++17"])
else:
    env.Append(CCFLAGS=["-std=c++17"])

# Include paths
env.Append(CPPPATH=[
    "src/",
    env["godot_cpp_path"] + "/include/",
    env["godot_cpp_path"] + "/gen/include/",
])

# FFmpeg configuration using pkg-config
def configure_ffmpeg(env):
    # Core FFmpeg libraries (required)
    core_ffmpeg_libs = [
        "libavcodec", "libavformat", "libavutil", "libswscale", "libswresample", "libavfilter", "libavdevice"
    ]
    
    # Optional codec libraries for maximum format support
    optional_codec_libs = [
        "vpx",        # VP8/VP9 codec
        "x264",       # H.264 encoder
        "x265",       # H.265/HEVC encoder  
        "opus",       # Opus audio codec
        "vorbis",     # Vorbis audio codec
        "theora",     # Theora video codec
        "lame",       # MP3 encoder
        "aom",        # AV1 encoder
        "dav1d"       # AV1 decoder
    ]
    
    # Check if pkg-config is available
    try:
        # Configure core FFmpeg libraries
        for lib in core_ffmpeg_libs:
            result = subprocess.run(["pkg-config", "--exists", lib], capture_output=True)
            if result.returncode == 0:
                cflags = subprocess.run(["pkg-config", "--cflags", lib], capture_output=True, text=True).stdout.strip()
                libs = subprocess.run(["pkg-config", "--libs", lib], capture_output=True, text=True).stdout.strip()
                
                if cflags:
                    env.MergeFlags(cflags)
                if libs:
                    env.MergeFlags(libs)
                print(f"✓ Found {lib}")
            else:
                print(f"✗ Warning: {lib} not found via pkg-config")
        
        # Configure optional codec libraries
        for lib in optional_codec_libs:
            result = subprocess.run(["pkg-config", "--exists", lib], capture_output=True)
            if result.returncode == 0:
                cflags = subprocess.run(["pkg-config", "--cflags", lib], capture_output=True, text=True).stdout.strip()
                libs = subprocess.run(["pkg-config", "--libs", lib], capture_output=True, text=True).stdout.strip()
                
                if cflags:
                    env.MergeFlags(cflags)
                if libs:
                    env.MergeFlags(libs)
                print(f"✓ Found optional codec: {lib}")
            else:
                print(f"ℹ Optional codec {lib} not available")
                
    except FileNotFoundError:
        print("Warning: pkg-config not found, falling back to manual configuration")
        # Fallback to manual FFmpeg configuration
        ffmpeg_include = env["ffmpeg_path"] + "/include"
        ffmpeg_lib = env["ffmpeg_path"] + "/lib"
        
        if os.path.exists(ffmpeg_include):
            env.Append(CPPPATH=[ffmpeg_include])
        else:
            print("Warning: FFmpeg include path not found:", ffmpeg_include)
        
        if os.path.exists(ffmpeg_lib):
            env.Append(LIBPATH=[ffmpeg_lib])
        else:
            print("Warning: FFmpeg lib path not found:", ffmpeg_lib)
        
        # Basic FFmpeg libraries
        ffmpeg_libs = ["avcodec", "avformat", "avutil", "swscale", "swresample"]
        env.Append(LIBS=ffmpeg_libs)

    # Windows-specific libraries
    if env["platform"] == "windows":
        env.Append(LIBS=["ws2_32", "secur32", "bcrypt", "mfplat", "mfuuid", "strmiids"])

configure_ffmpeg(env)

# Godot-cpp library
godot_cpp_lib = env["godot_cpp_path"] + "/bin/"
if env["platform"] == "windows":
    godot_cpp_lib += "libgodot-cpp"
else:
    godot_cpp_lib += "libgodot-cpp"

godot_cpp_lib += "." + env["platform"] + "." + env["target"]
if env["arch"] != "x86_64":
    godot_cpp_lib += "." + env["arch"]

if env["platform"] == "windows":
    godot_cpp_lib += ".lib"
else:
    godot_cpp_lib += ".a"

if os.path.exists(godot_cpp_lib):
    env.Append(LIBS=[File(godot_cpp_lib)])
else:
    print("Warning: godot-cpp library not found:", godot_cpp_lib)
    print("Please build godot-cpp first")

# Source files
sources = []
sources += Glob("src/*.cpp")
sources += Glob("src/decoder/*.cpp")
sources += Glob("src/stream/*.cpp")

# Output library name
library_name = "lymo_ffmpeg"
if env["platform"] == "windows":
    library_name += ".dll"
elif env["platform"] == "linux":
    library_name += ".so"

# Build library
library = env.SharedLibrary("bin/" + library_name, sources)

# Copy to demo project if it exists
demo_addons_path = "demo/addons/lymo_ffmpeg/bin/"
if os.path.exists("demo/"):
    env.Command(demo_addons_path + library_name, library, Copy("$TARGET", "$SOURCE"))

Default(library)