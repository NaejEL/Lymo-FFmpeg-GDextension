#include "ffmpeg_video_stream.h"
#include "../decoder/ffmpeg_decoder.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// FFmpegVideoStreamPlayback implementation

FFmpegVideoStreamPlayback::FFmpegVideoStreamPlayback() {
    playback_position = 0.0;
    is_playing = false;
    is_paused = false;
    mix_rate = 48000.0;
    last_frame_time = -1.0;
    frame_cache_valid = false;
    
    texture = Ref<ImageTexture>(memnew(ImageTexture));
}

FFmpegVideoStreamPlayback::~FFmpegVideoStreamPlayback() {
    stop();
}

void FFmpegVideoStreamPlayback::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_decoder", "decoder"), &FFmpegVideoStreamPlayback::set_decoder);
}

void FFmpegVideoStreamPlayback::set_decoder(Ref<FFmpegDecoder> p_decoder) {
    decoder = p_decoder;
    if (decoder.is_valid() && decoder->is_file_open()) {
        // Initialize texture with proper size
        int width = decoder->get_width();
        int height = decoder->get_height();
        
        if (width > 0 && height > 0) {
            // Create initial black frame
            PackedByteArray initial_data;
            int channels = decoder->get_has_alpha() ? 4 : 3;
            initial_data.resize(width * height * channels);
            memset(initial_data.ptrw(), 0, initial_data.size());
            
            Image::Format format = decoder->get_has_alpha() ? Image::FORMAT_RGBA8 : Image::FORMAT_RGB8;
            Ref<Image> initial_image = Image::create_from_data(width, height, false, format, initial_data);
            texture->set_image(initial_image);
        }
    }
}

void FFmpegVideoStreamPlayback::stop() {
    is_playing = false;
    is_paused = false;
    playback_position = 0.0;
    frame_cache_valid = false;
    last_frame_time = -1.0;
}

void FFmpegVideoStreamPlayback::play() {
    if (!decoder.is_valid() || !decoder->is_file_open()) {
        UtilityFunctions::print("Error: No valid decoder for playback");
        return;
    }
    
    is_playing = true;
    is_paused = false;
}

bool FFmpegVideoStreamPlayback::is_playing() const {
    return is_playing && !is_paused;
}

void FFmpegVideoStreamPlayback::set_paused(bool p_paused) {
    is_paused = p_paused;
}

bool FFmpegVideoStreamPlayback::is_paused() const {
    return is_paused;
}

double FFmpegVideoStreamPlayback::get_length() const {
    if (decoder.is_valid()) {
        return decoder->get_duration();
    }
    return 0.0;
}

double FFmpegVideoStreamPlayback::get_playback_position() const {
    return playback_position;
}

void FFmpegVideoStreamPlayback::seek(double p_time) {
    if (!decoder.is_valid()) return;
    
    if (decoder->seek_to_time(p_time)) {
        playback_position = p_time;
        frame_cache_valid = false;
        last_frame_time = -1.0;
    }
}

void FFmpegVideoStreamPlayback::set_audio_track(int p_idx) {
    // TODO: Implement audio track selection
}

Ref<Texture2D> FFmpegVideoStreamPlayback::get_texture() const {
    return texture;
}

void FFmpegVideoStreamPlayback::update(double p_delta) {
    if (!is_playing || is_paused || !decoder.is_valid()) {
        return;
    }
    
    playback_position += p_delta;
    
    // Check if we need a new frame based on video framerate
    double frame_rate = decoder->get_frame_rate();
    if (frame_rate <= 0) return;
    
    double frame_duration = 1.0 / frame_rate;
    double current_frame_time = floor(playback_position / frame_duration) * frame_duration;
    
    // Only decode new frame if we've moved to a different frame time
    if (!frame_cache_valid || current_frame_time != last_frame_time) {
        // Seek to correct position if needed (for precise frame timing)
        if (abs(current_frame_time - last_frame_time) > frame_duration * 1.5) {
            decoder->seek_to_time(current_frame_time);
        }
        
        Ref<Image> frame = decoder->decode_next_frame();
        if (frame.is_valid()) {
            texture->set_image(frame);
            cached_frame = frame;
            frame_cache_valid = true;
            last_frame_time = current_frame_time;
        }
    }
    
    // Check for end of video
    double duration = get_length();
    if (duration > 0 && playback_position >= duration) {
        stop();
    }
}

int FFmpegVideoStreamPlayback::get_channels() const {
    // TODO: Return actual audio channel count
    return 2;
}

int FFmpegVideoStreamPlayback::get_mix_rate() const {
    return mix_rate;
}

// FFmpegVideoStream implementation

FFmpegVideoStream::FFmpegVideoStream() {
    decoder = Ref<FFmpegDecoder>(memnew(FFmpegDecoder));
}

FFmpegVideoStream::~FFmpegVideoStream() {
    if (decoder.is_valid()) {
        decoder->close();
    }
}

void FFmpegVideoStream::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_file", "file"), &FFmpegVideoStream::set_file);
    ClassDB::bind_method(D_METHOD("get_file"), &FFmpegVideoStream::get_file);
    
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "file", PROPERTY_HINT_FILE, "*.mp4,*.avi,*.mkv,*.mov,*.webm"), "set_file", "get_file");
}

void FFmpegVideoStream::set_file(const String &p_file) {
    file_path = p_file;
    
    if (decoder.is_valid()) {
        decoder->close();
        if (!p_file.is_empty()) {
            if (!decoder->open_file(p_file)) {
                UtilityFunctions::print("Error: Failed to open video file: ", p_file);
            }
        }
    }
}

String FFmpegVideoStream::get_file() const {
    return file_path;
}

Ref<VideoStreamPlayback> FFmpegVideoStream::_instantiate_playback() {
    Ref<FFmpegVideoStreamPlayback> playback = Ref<FFmpegVideoStreamPlayback>(memnew(FFmpegVideoStreamPlayback));
    
    if (decoder.is_valid() && decoder->is_file_open()) {
        // Create a new decoder instance for this playback
        Ref<FFmpegDecoder> playback_decoder = Ref<FFmpegDecoder>(memnew(FFmpegDecoder));
        playback_decoder->set_use_hardware_acceleration(decoder->get_use_hardware_acceleration());
        
        if (playback_decoder->open_file(file_path)) {
            playback->set_decoder(playback_decoder);
        } else {
            UtilityFunctions::print("Error: Failed to create playback decoder for: ", file_path);
        }
    }
    
    return playback;
}