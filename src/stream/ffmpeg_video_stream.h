#ifndef FFMPEG_VIDEO_STREAM_H
#define FFMPEG_VIDEO_STREAM_H

#include <godot_cpp/classes/video_stream.hpp>
#include <godot_cpp/classes/video_stream_playback.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class FFmpegDecoder;

class FFmpegVideoStreamPlayback : public VideoStreamPlayback {
    GDCLASS(FFmpegVideoStreamPlayback, VideoStreamPlayback)

private:
    Ref<FFmpegDecoder> decoder;
    Ref<ImageTexture> texture;
    double playback_position;
    bool is_playing;
    bool is_paused;
    double mix_rate;
    
    // Performance optimization
    double last_frame_time;
    bool frame_cache_valid;
    Ref<Image> cached_frame;
    
protected:
    static void _bind_methods();

public:
    FFmpegVideoStreamPlayback();
    ~FFmpegVideoStreamPlayback();
    
    void set_decoder(Ref<FFmpegDecoder> p_decoder);
    
    // VideoStreamPlayback interface
    virtual void stop() override;
    virtual void play() override;
    virtual bool is_playing() const override;
    virtual void set_paused(bool p_paused) override;
    virtual bool is_paused() const override;
    virtual double get_length() const override;
    virtual double get_playback_position() const override;
    virtual void seek(double p_time) override;
    virtual void set_audio_track(int p_idx) override;
    virtual Ref<Texture2D> get_texture() const override;
    virtual void update(double p_delta) override;
    virtual int get_channels() const override;
    virtual int get_mix_rate() const override;
};

class FFmpegVideoStream : public VideoStream {
    GDCLASS(FFmpegVideoStream, VideoStream)

private:
    String file_path;
    Ref<FFmpegDecoder> decoder;
    
protected:
    static void _bind_methods();

public:
    FFmpegVideoStream();
    ~FFmpegVideoStream();
    
    void set_file(const String &p_file);
    String get_file() const;
    
    // VideoStream interface
    virtual Ref<VideoStreamPlayback> _instantiate_playback() override;
};

}

#endif // FFMPEG_VIDEO_STREAM_H