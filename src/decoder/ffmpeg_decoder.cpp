#include "ffmpeg_decoder.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// Static callback for hardware format selection
AVPixelFormat FFmpegDecoder::hw_pix_fmt_callback(AVCodecContext *ctx, const AVPixelFormat *pix_fmts) {
    FFmpegDecoder *decoder = static_cast<FFmpegDecoder*>(ctx->opaque);
    return decoder->get_hw_format(ctx, pix_fmts);
}

FFmpegDecoder::FFmpegDecoder() {
    format_context = nullptr;
    codec_context = nullptr;
    codec = nullptr;
    frame = nullptr;
    hw_frame = nullptr;
    packet = nullptr;
    sws_context = nullptr;
    hw_device_ctx = nullptr;
    
    video_stream_index = -1;
    is_open = false;
    use_hardware_acceleration = true;
    hw_device_type = AV_HWDEVICE_TYPE_NONE;
    
    width = 0;
    height = 0;
    frame_rate = 0.0;
    duration = 0;
    pixel_format = AV_PIX_FMT_NONE;
    has_alpha = false;
    
    // Allocate frames and packet
    frame = av_frame_alloc();
    hw_frame = av_frame_alloc();
    packet = av_packet_alloc();
}

FFmpegDecoder::~FFmpegDecoder() {
    close();
    
    if (frame) {
        av_frame_free(&frame);
    }
    if (hw_frame) {
        av_frame_free(&hw_frame);
    }
    if (packet) {
        av_packet_free(&packet);
    }
}

void FFmpegDecoder::_bind_methods() {
    ClassDB::bind_method(D_METHOD("open_file", "path"), &FFmpegDecoder::open_file);
    ClassDB::bind_method(D_METHOD("open_stream", "data"), &FFmpegDecoder::open_stream);
    ClassDB::bind_method(D_METHOD("close"), &FFmpegDecoder::close);
    
    ClassDB::bind_method(D_METHOD("decode_next_frame"), &FFmpegDecoder::decode_next_frame);
    ClassDB::bind_method(D_METHOD("seek_to_time", "time_seconds"), &FFmpegDecoder::seek_to_time);
    ClassDB::bind_method(D_METHOD("seek_to_frame", "frame_number"), &FFmpegDecoder::seek_to_frame);
    
    ClassDB::bind_method(D_METHOD("is_file_open"), &FFmpegDecoder::is_file_open);
    ClassDB::bind_method(D_METHOD("get_width"), &FFmpegDecoder::get_width);
    ClassDB::bind_method(D_METHOD("get_height"), &FFmpegDecoder::get_height);
    ClassDB::bind_method(D_METHOD("get_frame_rate"), &FFmpegDecoder::get_frame_rate);
    ClassDB::bind_method(D_METHOD("get_duration"), &FFmpegDecoder::get_duration);
    ClassDB::bind_method(D_METHOD("get_has_alpha"), &FFmpegDecoder::get_has_alpha);
    ClassDB::bind_method(D_METHOD("get_pixel_format_name"), &FFmpegDecoder::get_pixel_format_name);
    
    ClassDB::bind_method(D_METHOD("set_use_hardware_acceleration", "enabled"), &FFmpegDecoder::set_use_hardware_acceleration);
    ClassDB::bind_method(D_METHOD("get_use_hardware_acceleration"), &FFmpegDecoder::get_use_hardware_acceleration);
    ClassDB::bind_method(D_METHOD("get_available_hw_decoders"), &FFmpegDecoder::get_available_hw_decoders);
    ClassDB::bind_method(D_METHOD("get_current_hw_decoder"), &FFmpegDecoder::get_current_hw_decoder);
    
    ClassDB::bind_method(D_METHOD("set_color_range", "range"), &FFmpegDecoder::set_color_range);
    ClassDB::bind_method(D_METHOD("set_color_space", "space"), &FFmpegDecoder::set_color_space);
    ClassDB::bind_method(D_METHOD("get_raw_frame_data"), &FFmpegDecoder::get_raw_frame_data);
    
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_hardware_acceleration"), "set_use_hardware_acceleration", "get_use_hardware_acceleration");
}

bool FFmpegDecoder::open_file(const String &path) {
    close();
    
    // Open input file
    const char* file_path = path.utf8().get_data();
    if (avformat_open_input(&format_context, file_path, nullptr, nullptr) < 0) {
        UtilityFunctions::print("Error: Could not open file ", path);
        return false;
    }
    
    // Retrieve stream information
    if (avformat_find_stream_info(format_context, nullptr) < 0) {
        UtilityFunctions::print("Error: Could not find stream information");
        close();
        return false;
    }
    
    // Find video stream
    video_stream_index = av_find_best_stream(format_context, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    if (video_stream_index < 0) {
        UtilityFunctions::print("Error: Could not find video stream");
        close();
        return false;
    }
    
    // Get stream info
    AVStream *video_stream = format_context->streams[video_stream_index];
    width = video_stream->codecpar->width;
    height = video_stream->codecpar->height;
    pixel_format = (AVPixelFormat)video_stream->codecpar->format;
    duration = format_context->duration;
    
    // Calculate frame rate
    if (video_stream->r_frame_rate.den != 0) {
        frame_rate = (double)video_stream->r_frame_rate.num / video_stream->r_frame_rate.den;
    } else if (video_stream->avg_frame_rate.den != 0) {
        frame_rate = (double)video_stream->avg_frame_rate.num / video_stream->avg_frame_rate.den;
    }
    
    // Check for alpha channel
    has_alpha = (pixel_format == AV_PIX_FMT_RGBA || pixel_format == AV_PIX_FMT_BGRA || 
                 pixel_format == AV_PIX_FMT_YUVA420P || pixel_format == AV_PIX_FMT_YUVA444P);
    
    // Allocate codec context
    codec_context = avcodec_alloc_context3(codec);
    if (!codec_context) {
        UtilityFunctions::print("Error: Could not allocate codec context");
        close();
        return false;
    }
    
    // Copy codec parameters
    if (avcodec_parameters_to_context(codec_context, video_stream->codecpar) < 0) {
        UtilityFunctions::print("Error: Could not copy codec parameters");
        close();
        return false;
    }
    
    // Initialize hardware acceleration if enabled
    if (use_hardware_acceleration) {
        init_hardware_acceleration();
    }
    
    // Open codec
    if (avcodec_open2(codec_context, codec, nullptr) < 0) {
        UtilityFunctions::print("Error: Could not open codec");
        close();
        return false;
    }
    
    is_open = true;
    UtilityFunctions::print("Successfully opened video: ", width, "x", height, " @ ", frame_rate, " fps");
    
    return true;
}

bool FFmpegDecoder::open_stream(const PackedByteArray &data) {
    // TODO: Implement memory-based stream opening
    UtilityFunctions::print("Stream opening not yet implemented");
    return false;
}

void FFmpegDecoder::close() {
    if (sws_context) {
        sws_freeContext(sws_context);
        sws_context = nullptr;
    }
    
    if (codec_context) {
        avcodec_free_context(&codec_context);
    }
    
    if (format_context) {
        avformat_close_input(&format_context);
    }
    
    cleanup_hardware_acceleration();
    
    is_open = false;
    video_stream_index = -1;
    width = height = 0;
    frame_rate = 0.0;
    duration = 0;
    pixel_format = AV_PIX_FMT_NONE;
    has_alpha = false;
}

Ref<Image> FFmpegDecoder::decode_next_frame() {
    if (!is_open) {
        return Ref<Image>();
    }
    
    while (av_read_frame(format_context, packet) >= 0) {
        if (packet->stream_index == video_stream_index) {
            int ret = avcodec_send_packet(codec_context, packet);
            if (ret < 0) {
                av_packet_unref(packet);
                continue;
            }
            
            ret = avcodec_receive_frame(codec_context, frame);
            if (ret == 0) {
                av_packet_unref(packet);
                
                // Handle hardware decoded frame
                AVFrame *display_frame = frame;
                if (frame->format == hw_device_type && hw_frame) {
                    if (av_hwframe_transfer_data(hw_frame, frame, 0) < 0) {
                        UtilityFunctions::print("Error transferring hardware frame to system memory");
                        continue;
                    }
                    display_frame = hw_frame;
                }
                
                return convert_frame_to_image(display_frame);
            }
        }
        av_packet_unref(packet);
    }
    
    return Ref<Image>();
}

bool FFmpegDecoder::seek_to_time(double time_seconds) {
    if (!is_open) return false;
    
    int64_t timestamp = (int64_t)(time_seconds * AV_TIME_BASE);
    if (av_seek_frame(format_context, -1, timestamp, AVSEEK_FLAG_BACKWARD) < 0) {
        return false;
    }
    
    avcodec_flush_buffers(codec_context);
    return true;
}

bool FFmpegDecoder::seek_to_frame(int64_t frame_number) {
    if (!is_open || frame_rate <= 0) return false;
    
    double time_seconds = frame_number / frame_rate;
    return seek_to_time(time_seconds);
}

String FFmpegDecoder::get_pixel_format_name() const {
    if (pixel_format == AV_PIX_FMT_NONE) return "Unknown";
    return String(av_get_pix_fmt_name(pixel_format));
}

bool FFmpegDecoder::init_hardware_acceleration() {
    // Try common hardware acceleration types
    AVHWDeviceType types[] = {
        AV_HWDEVICE_TYPE_CUDA,
        AV_HWDEVICE_TYPE_VAAPI,
        AV_HWDEVICE_TYPE_D3D11VA,
        AV_HWDEVICE_TYPE_DXVA2,
        AV_HWDEVICE_TYPE_VIDEOTOOLBOX
    };
    
    for (AVHWDeviceType type : types) {
        if (av_hwdevice_ctx_create(&hw_device_ctx, type, nullptr, nullptr, 0) == 0) {
            hw_device_type = type;
            codec_context->hw_device_ctx = av_buffer_ref(hw_device_ctx);
            codec_context->get_format = hw_pix_fmt_callback;
            codec_context->opaque = this;
            
            UtilityFunctions::print("Hardware acceleration enabled: ", av_hwdevice_get_type_name(type));
            return true;
        }
    }
    
    UtilityFunctions::print("Hardware acceleration not available, using software decoding");
    return false;
}

void FFmpegDecoder::cleanup_hardware_acceleration() {
    if (hw_device_ctx) {
        av_buffer_unref(&hw_device_ctx);
        hw_device_type = AV_HWDEVICE_TYPE_NONE;
    }
}

AVPixelFormat FFmpegDecoder::get_hw_format(AVCodecContext *ctx, const AVPixelFormat *pix_fmts) {
    const AVPixelFormat *p;
    
    for (p = pix_fmts; *p != -1; p++) {
        const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(*p);
        if (desc->flags & AV_PIX_FMT_FLAG_HWACCEL) {
            return *p;
        }
    }
    
    return AV_PIX_FMT_NONE;
}

Ref<Image> FFmpegDecoder::convert_frame_to_image(AVFrame *src_frame) {
    if (!src_frame) return Ref<Image>();
    
    // Setup scaler for RGB conversion
    AVPixelFormat target_format = has_alpha ? AV_PIX_FMT_RGBA : AV_PIX_FMT_RGB24;
    if (!setup_scaler((AVPixelFormat)src_frame->format, src_frame->width, src_frame->height)) {
        return Ref<Image>();
    }
    
    // Allocate destination frame
    AVFrame *rgb_frame = av_frame_alloc();
    int bytes_per_pixel = has_alpha ? 4 : 3;
    int buffer_size = av_image_get_buffer_size(target_format, width, height, 1);
    uint8_t *buffer = (uint8_t*)av_malloc(buffer_size);
    
    av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, buffer, target_format, width, height, 1);
    
    // Convert frame
    sws_scale(sws_context, src_frame->data, src_frame->linesize, 0, height,
              rgb_frame->data, rgb_frame->linesize);
    
    // Create Godot Image
    PackedByteArray image_data;
    image_data.resize(width * height * bytes_per_pixel);
    memcpy(image_data.ptrw(), buffer, buffer_size);
    
    Image::Format godot_format = has_alpha ? Image::FORMAT_RGBA8 : Image::FORMAT_RGB8;
    Ref<Image> image = Image::create_from_data(width, height, false, godot_format, image_data);
    
    // Cleanup
    av_free(buffer);
    av_frame_free(&rgb_frame);
    
    return image;
}

bool FFmpegDecoder::setup_scaler(AVPixelFormat src_format, int src_width, int src_height) {
    AVPixelFormat target_format = has_alpha ? AV_PIX_FMT_RGBA : AV_PIX_FMT_RGB24;
    
    if (sws_context) {
        sws_freeContext(sws_context);
    }
    
    sws_context = sws_getContext(src_width, src_height, src_format,
                                width, height, target_format,
                                SWS_BILINEAR, nullptr, nullptr, nullptr);
    
    return sws_context != nullptr;
}

void FFmpegDecoder::set_use_hardware_acceleration(bool enabled) {
    use_hardware_acceleration = enabled;
}

PackedStringArray FFmpegDecoder::get_available_hw_decoders() {
    PackedStringArray result;
    
    AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;
    while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE) {
        result.push_back(String(av_hwdevice_get_type_name(type)));
    }
    
    return result;
}

String FFmpegDecoder::get_current_hw_decoder() {
    if (hw_device_type != AV_HWDEVICE_TYPE_NONE) {
        return String(av_hwdevice_get_type_name(hw_device_type));
    }
    return "Software";
}

void FFmpegDecoder::set_color_range(int range) {
    // TODO: Implement color range setting for YUV processing
}

void FFmpegDecoder::set_color_space(int space) {
    // TODO: Implement color space setting
}

PackedByteArray FFmpegDecoder::get_raw_frame_data() {
    // TODO: Return raw frame data for custom shader processing
    return PackedByteArray();
}