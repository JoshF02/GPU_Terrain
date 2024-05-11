#version 450

layout (local_size_x = 16, local_size_y = 16) in;

uniform float amplitude;
uniform float frequency;
uniform int octaves;
uniform float persistence;
uniform float lacunarity;
uniform vec2 offset;

layout(binding = 0, rgba8) uniform image2D resultImage;


const int gradientSizeTable = 256;
int perm[gradientSizeTable * 2];

void permute() {
    for (int i = 0; i < gradientSizeTable; ++i) {
        perm[i] = i;
    }
    for (int i = 0; i < gradientSizeTable; ++i) {
        int j = int(mod(float(perm[i]) * 1.618033988749895, float(gradientSizeTable)));
        int temp = perm[i];
        perm[i] = perm[j];
        perm[j] = temp;
    }
    for (int i = 0; i < gradientSizeTable; ++i) {
        perm[i + gradientSizeTable] = perm[i];
    }
}


float fade(float t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

float lerp(float t, float a, float b) {
    return a + t * (b - a);
}

float grad(int hash, float x, float y) {
    int h = hash & 7;      
    float u = h < 4 ? x : y;
    float v = h < 4 ? y : x;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}



float noise(float x, float y) {
    int X = int(floor(x)) & (gradientSizeTable - 1);
    int Y = int(floor(y)) & (gradientSizeTable - 1);

    x -= floor(x);
    y -= floor(y);

    float u = fade(x);
    float v = fade(y);

    int A = perm[X] + Y;
    int B = perm[X + 1] + Y;

    return lerp(v, 
                lerp(u, grad(perm[A], x, y), grad(perm[B], x - 1, y)), 
                lerp(u, grad(perm[A + 1], x, y - 1), grad(perm[B + 1], x - 1, y - 1)));
}

void main() {
    permute();

    ivec2 globalID = ivec2(gl_GlobalInvocationID.xy);
    vec2 coords = ((vec2(globalID) / 256) + offset);

    float height = 0.0;
    float amp = amplitude;
    float freq = frequency;
    
    for (int i = 0; i <= octaves; i++) {
        vec2 scaledCoords = coords * freq;
        height += noise(scaledCoords.x, scaledCoords.y) * amp;
        amp *= persistence;
        freq *= lacunarity;
    }

    imageStore(resultImage, globalID, vec4(globalID.x, height, globalID.y, 1.0));
}