shader_type canvas_item;

// Advanced video projection shader for Lymo projection mapping
// Supports perspective correction, edge blending, and color calibration

uniform sampler2D video_texture : source_color, filter_linear;
uniform bool enable_perspective_correction : hint_default_true = true;
uniform bool enable_edge_blending : hint_default_true = false;
uniform bool enable_color_correction : hint_default_true = false;

// Perspective correction
uniform vec2 top_left = vec2(0.0, 0.0);
uniform vec2 top_right = vec2(1.0, 0.0);
uniform vec2 bottom_left = vec2(0.0, 1.0);
uniform vec2 bottom_right = vec2(1.0, 1.0);

// Edge blending
uniform float blend_left : hint_range(0.0, 0.5) = 0.0;
uniform float blend_right : hint_range(0.0, 0.5) = 0.0;
uniform float blend_top : hint_range(0.0, 0.5) = 0.0;
uniform float blend_bottom : hint_range(0.0, 0.5) = 0.0;
uniform float blend_gamma : hint_range(0.1, 3.0) = 2.2;

// Color correction
uniform float brightness : hint_range(-1.0, 1.0) = 0.0;
uniform float contrast : hint_range(0.0, 3.0) = 1.0;
uniform float saturation : hint_range(0.0, 3.0) = 1.0;
uniform vec3 color_gain = vec3(1.0, 1.0, 1.0);
uniform vec3 color_offset = vec3(0.0, 0.0, 0.0);

// Gamma correction
uniform float gamma : hint_range(0.1, 3.0) = 2.2;

// Perspective correction using bilinear interpolation
vec2 perspective_correct(vec2 uv) {
    // Bilinear interpolation of corner points
    vec2 top = mix(top_left, top_right, uv.x);
    vec2 bottom = mix(bottom_left, bottom_right, uv.x);
    return mix(top, bottom, uv.y);
}

// Edge blending calculation
float edge_blend_factor(vec2 uv) {
    float factor = 1.0;
    
    // Left edge
    if (blend_left > 0.0 && uv.x < blend_left) {
        factor *= pow(uv.x / blend_left, blend_gamma);
    }
    
    // Right edge  
    if (blend_right > 0.0 && uv.x > (1.0 - blend_right)) {
        factor *= pow((1.0 - uv.x) / blend_right, blend_gamma);
    }
    
    // Top edge
    if (blend_top > 0.0 && uv.y < blend_top) {
        factor *= pow(uv.y / blend_top, blend_gamma);
    }
    
    // Bottom edge
    if (blend_bottom > 0.0 && uv.y > (1.0 - blend_bottom)) {
        factor *= pow((1.0 - uv.y) / blend_bottom, blend_gamma);
    }
    
    return factor;
}

// Color correction
vec3 apply_color_correction(vec3 color) {
    // Brightness and contrast
    color = (color - 0.5) * contrast + 0.5 + brightness;
    
    // Color gain and offset
    color = color * color_gain + color_offset;
    
    // Saturation
    float luminance = dot(color, vec3(0.299, 0.587, 0.114));
    color = mix(vec3(luminance), color, saturation);
    
    return color;
}

void fragment() {
    vec2 corrected_uv = UV;
    
    // Apply perspective correction
    if (enable_perspective_correction) {
        corrected_uv = perspective_correct(UV);
    }
    
    // Sample video texture
    vec4 video_color = texture(video_texture, corrected_uv);
    
    // Apply color correction
    if (enable_color_correction) {
        video_color.rgb = apply_color_correction(video_color.rgb);
    }
    
    // Apply gamma correction
    video_color.rgb = pow(video_color.rgb, vec3(1.0 / gamma));
    
    // Apply edge blending
    if (enable_edge_blending) {
        float blend_factor = edge_blend_factor(UV);
        video_color.rgb *= blend_factor;
    }
    
    // Clamp final color
    video_color.rgb = clamp(video_color.rgb, 0.0, 1.0);
    
    COLOR = video_color;
}