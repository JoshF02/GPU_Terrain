#version 450

precision highp float;

layout (local_size_x = 16, local_size_y = 16) in;

uniform float amplitude;
uniform float frequency;
uniform int octaves;
uniform float persistence;
uniform float lacunarity;
uniform vec2 offset;
uniform int size;

uniform int perm256[256];

layout(binding = 0, rgba32f) uniform image2D resultImage;


const int gradientSizeTable = 256;
int perm[gradientSizeTable * 2];

void permute() {
    for (int i = 0; i < gradientSizeTable; ++i) {
        perm[i] = perm256[i];
        perm[i + gradientSizeTable] = perm256[i];
    }
}


float fade(float t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}


float grad(int hash, float x, float y) {
    int h = hash & 7;      
    float u = h < 4 ? x : y;
    float v = h < 4 ? y : x;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}


float noise(float x, float y) {
    float F2 = 0.5 * (sqrt(3.0) - 1.0);
    float s = (x + y) * F2;

    int i = int(floor(x + s));
    int j = int(floor(y + s));

    float G2 = (3.0 - sqrt(3.0)) / 6.0;
    float t = float(i + j) * G2;

    float X0 = float(i) - t;
    float Y0 = float(j) - t;
    float x0 = x - X0;
    float y0 = y - Y0;

    int i1, j1;
    if (x0 > y0) { i1 = 1; j1 = 0; }
    else { i1 = 0; j1 = 1; }

    float x1 = x0 - float(i1) + G2;
    float y1 = y0 - float(j1) + G2;
    float x2 = x0 - 1.0 + 2.0 * G2;
    float y2 = y0 - 1.0 + 2.0 * G2;

    int ii = i & 255;
    int jj = j & 255;

    int gi0 = perm[ii + perm[jj]] % 12;
    int gi1 = perm[ii + i1 + perm[jj + j1]] % 12;
    int gi2 = perm[ii + 1 + perm[jj + 1]] % 12;
    
    float n0, n1, n2;

    float t0 = 0.5 - x0 * x0 - y0 * y0;
    if (t0 < 0.0) n0 = 0.0;
    else {
        t0 *= t0;
        n0 = t0 * t0 * grad(gi0, x0, y0);
    }

    float t1 = 0.5 - x1 * x1 - y1 * y1;
    if (t1 < 0.0) n1 = 0.0;
    else {
        t1 *= t1;
        n1 = t1 * t1 * grad(gi1, x1, y1);
    }

    float t2 = 0.5 - x2 * x2 - y2 * y2;
    if (t2 < 0.0) n2 = 0.0;
    else {
        t2 *= t2;
        n2 = t2 * t2 * grad(gi2, x2, y2);
    }

    return 70.0 * (n0 + n1 + n2);
}

void main() {
    permute();

    ivec2 globalID = ivec2(gl_GlobalInvocationID.xy);
    vec2 coords = ((vec2(globalID) / size) + offset);

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