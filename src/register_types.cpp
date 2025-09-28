#include "register_types.h"

#include "stream/ffmpeg_video_stream.h"
#include "decoder/ffmpeg_decoder.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void initialize_lymo_ffmpeg_module() {
    ClassDB::register_class<FFmpegVideoStream>();
    ClassDB::register_class<FFmpegDecoder>();
}

void uninitialize_lymo_ffmpeg_module() {
    // Cleanup if needed
}

extern "C" {
    GDExtensionBool GDE_EXPORT gdextension_initialize(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(initialize_lymo_ffmpeg_module);
        init_obj.register_terminator(uninitialize_lymo_ffmpeg_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}