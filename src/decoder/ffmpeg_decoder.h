#ifndef FFMPEG_DECODER_H
#define FFMPEG_DECODER_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/avutil.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/hwcontext.h>
    #include <libswscale/swscale.h>
}

namespace godot {

class FFmpegDecoder : public RefCounted {
    GDCLASS(FFmpegDecoder, RefCounted)

private:
    AVFormatContext *format_context;
    AVCodecContext *codec_context;
    AVCodec *codec;
    AVFrame *frame;
    AVFrame *hw_frame;
    AVPacket *packet;
    SwsContext *sws_context;
    
    int video_stream_index;
    bool is_open;
    bool use_hardware_acceleration;
    AVHWDeviceType hw_device_type;
    AVBufferRef *hw_device_ctx;
    
    // Video properties
    int width;
    int height;
    double frame_rate;
    int64_t duration;
    AVPixelFormat pixel_format;
    bool has_alpha;
    
    // Hardware acceleration methods
    bool init_hardware_acceleration();
    void cleanup_hardware_acceleration();
    AVPixelFormat get_hw_format(AVCodecContext *ctx, const AVPixelFormat *pix_fmts);
    static AVPixelFormat hw_pix_fmt_callback(AVCodecContext *ctx, const AVPixelFormat *pix_fmts);
    
    // Frame conversion
    Ref<Image> convert_frame_to_image(AVFrame *frame);
    bool setup_scaler(AVPixelFormat src_format, int src_width, int src_height);
    
protected:
    static void _bind_methods();

public:
    FFmpegDecoder();
    ~FFmpegDecoder();
    
    // Core functionality
    bool open_file(const String &path);
    bool open_stream(const PackedByteArray &data);
    void close();
    
    // Decoding
    Ref<Image> decode_next_frame();
    bool seek_to_time(double time_seconds);
    bool seek_to_frame(int64_t frame_number);
    
    // Properties
    bool is_file_open() const { return is_open; }
    int get_width() const { return width; }
    int get_height() const { return height; }
    double get_frame_rate() const { return frame_rate; }
    double get_duration() const { return duration > 0 ? (double)duration / AV_TIME_BASE : 0.0; }
    bool get_has_alpha() const { return has_alpha; }
    String get_pixel_format_name() const;
    
    // Hardware acceleration
    void set_use_hardware_acceleration(bool enabled);
    bool get_use_hardware_acceleration() const { return use_hardware_acceleration; }
    PackedStringArray get_available_hw_decoders();
    String get_current_hw_decoder();
    
    // Advanced features for projection mapping
    void set_color_range(int range); // For YUV color range handling
    void set_color_space(int space); // For color space conversion
    PackedByteArray get_raw_frame_data(); // For custom shader processing
};

}

#endif // FFMPEG_DECODER_H