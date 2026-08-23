#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D rgbaTexture;
uniform float time;
uniform float grainIntensity;

// Internal tuning constants (not exposed as pins)
const float kSharpGrainScale = 0.03; // per-pixel shot-noise contribution
const float kBlockGrainScale = 0.04; // 8x8 macroblock compression-noise contribution
const float kBlockSize       = 8.0;  // webcam-style macroblock size in pixels

// Hash-based pseudo-random noise in [0,1), varies per-pixel and over time
float hash(vec2 p, float t)
{
    p = fract(p * vec2(443.8975, 397.2973) + t);
    p += dot(p, p + 19.19);
    return fract((p.x + p.y) * p.x);
}

// Returns pseudo-random noise in [-1, 1)
float dynamicNoise(vec2 uv, float time)
{
    return hash(uv, time) * 2.0 - 1.0;
}

void main()
{
    vec4 srcColor = texture(rgbaTexture, TexCoords);

    // Resolve texture resolution without needing a resolution uniform/pin
    ivec2 texSize = textureSize(rgbaTexture, 0);
    vec2 resolution = vec2(texSize);
    vec2 pixelCoord = TexCoords * resolution;

    // Luma-masked shot noise: strongest in midtones, fades out at pure black/white
    float luma = dot(srcColor.rgb, vec3(0.299, 0.587, 0.114));
    float lumaMask = 4.0 * luma * (1.0 - luma);

    // Sharp per-pixel shot noise ("Sensor Grain")
    float sharpNoise = dynamicNoise(pixelCoord, time);
    float sharpAmount = grainIntensity * kSharpGrainScale;

    // Blocky per-8x8-macroblock noise ("webcam macroblock noise")
    vec2 blockCoord = floor(pixelCoord / kBlockSize) * kBlockSize;
    float blockNoise = dynamicNoise(blockCoord, time);
    float blockAmount = grainIntensity * kBlockGrainScale;

    vec3 grain = vec3(sharpNoise * sharpAmount + blockNoise * blockAmount) * lumaMask;

    vec3 outColor = clamp(srcColor.rgb + grain, 0.0, 1.0);

    // Alpha untouched
    FragColor = vec4(outColor, srcColor.a);
}
