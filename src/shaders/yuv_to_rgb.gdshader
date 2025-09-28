shader_type canvas_item;

// YUV to RGB conversion shader for FFmpeg video streams
// Supports various YUV formats and color spaces for projection mapping

uniform sampler2D y_texture : source_color, filter_linear;
uniform sampler2D u_texture : source_color, filter_linear;
uniform sampler2D v_texture : source_color, filter_linear;
uniform sampler2D a_texture : source_color, filter_linear; // Optional alpha channel

uniform bool has_alpha : hint_default_true = false;
uniform bool full_range : hint_default_true = false; // TV range (16-235) vs Full range (0-255)
uniform int color_space : hint_range(0, 3) = 0; // 0=BT.601, 1=BT.709, 2=BT.2020, 3=SMPTE-240M

// Color space conversion matrices
// BT.601 (SD)
const mat3 BT601_MATRIX = mat3(
    vec3(1.164, 1.164, 1.164),
    vec3(0.0, -0.392, 2.017),
    vec3(1.596, -0.813, 0.0)
);

// BT.709 (HD)
const mat3 BT709_MATRIX = mat3(
    vec3(1.164, 1.164, 1.164),
    vec3(0.0, -0.213, 2.112),
    vec3(1.793, -0.533, 0.0)
);

// BT.2020 (UHD)
const mat3 BT2020_MATRIX = mat3(
    vec3(1.164, 1.164, 1.164),
    vec3(0.0, -0.187, 2.141),
    vec3(1.717, -0.652, 0.0)
);

// SMPTE-240M
const mat3 SMPTE240M_MATRIX = mat3(
    vec3(1.164, 1.164, 1.164),
    vec3(0.0, -0.230, 2.078),
    vec3(1.794, -0.542, 0.0)
);

vec3 yuv_to_rgb(vec3 yuv, mat3 conversion_matrix, bool is_full_range) {
    vec3 offset;
    vec3 scale;
    
    if (is_full_range) {
        // Full range: Y [0, 255], UV [0, 255]
        offset = vec3(0.0, -0.5, -0.5);
        scale = vec3(1.0, 1.0, 1.0);
    } else {
        // TV range: Y [16, 235], UV [16, 240]
        offset = vec3(-16.0/255.0, -0.5, -0.5);
        scale = vec3(255.0/219.0, 255.0/224.0, 255.0/224.0);
    }
    
    // Apply offset and scale
    yuv = (yuv + offset) * scale;
    
    // Convert to RGB
    return conversion_matrix * yuv;
}

void fragment() {
    vec2 uv = TEXTURE_PIXEL_SIZE * floor(UV / TEXTURE_PIXEL_SIZE);
    
    // Sample YUV channels
    float y = texture(y_texture, uv).r;
    float u = texture(u_texture, uv).r;
    float v = texture(v_texture, uv).r;
    
    vec3 yuv = vec3(y, u, v);
    
    // Select conversion matrix based on color space
    mat3 conversion_matrix;
    switch (color_space) {
        case 1:
            conversion_matrix = BT709_MATRIX;
            break;
        case 2:
            conversion_matrix = BT2020_MATRIX;
            break;
        case 3:
            conversion_matrix = SMPTE240M_MATRIX;
            break;
        default:
            conversion_matrix = BT601_MATRIX;
            break;
    }
    
    // Convert YUV to RGB
    vec3 rgb = yuv_to_rgb(yuv, conversion_matrix, full_range);
    
    // Clamp to valid range
    rgb = clamp(rgb, 0.0, 1.0);
    
    // Sample alpha if available
    float alpha = 1.0;
    if (has_alpha) {
        alpha = texture(a_texture, uv).r;
    }
    
    COLOR = vec4(rgb, alpha);
}