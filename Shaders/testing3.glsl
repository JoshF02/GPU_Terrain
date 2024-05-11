// testing2 but with the options from testing1 + output buffer
#version 450

layout (local_size_x = 16, local_size_y = 16) in;

//uniform float scale;
//uniform float dims;
uniform float amplitude;
uniform float frequency;
uniform int octaves;
uniform float persistence;
uniform float lacunarity;
uniform vec2 offset;

//uniform int perm[512];

layout(binding = 0, rgba8) uniform image2D resultImage; // changed from rgba32f


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
    /*perm = [151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
                          140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
                          247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
                          57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
                          74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
                          60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
                          65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
                          200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3,
                          64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82,
                          85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223,
                          183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155,
                          167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224,
                          232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238,
                          210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239,
                          107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115,
                          121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29,
                          24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,
            151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
                          140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
                          247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
                          57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
                          74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
                          60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
                          65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
                          200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3,
                          64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82,
                          85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223,
                          183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155,
                          167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224,
                          232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238,
                          210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239,
                          107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115,
                          121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29,
                          24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180 ];*/
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
    vec2 p = ((vec2(globalID) / 256) + offset);// / scale;

    float height = 0.0;
    //float amplitude = 1.0;
    float amp = amplitude;
    float freq = frequency;
    
    for (int i = 0; i <= octaves; i++) {
        //vec2 pp = p * pow(lacunarity, float(i));
        vec2 pp = p * freq;
        height += noise(pp.x, pp.y) * amp;
        amp *= persistence;
        freq *= lacunarity;
    }

    //imageStore(outputTexture, globalID, vec4(vec3(height), 1.0));

    ivec2 imgSize = imageSize(resultImage);
    ivec2 flippedCoord = ivec2(globalID.x, imgSize.y - globalID.y - 1); // Flip Y coordinate

    imageStore(resultImage, globalID, vec4(globalID.x, height, globalID.y, 1.0));

    //imageStore(resultImage, globalID, vec4(0.687,0.687,0.687,0.449));
}