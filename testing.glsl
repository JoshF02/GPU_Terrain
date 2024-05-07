#version 430

layout (local_size_x = 16, local_size_y = 16) in;

uniform float scale;
uniform int octaves;
uniform float persistence;
uniform float lacunarity;
uniform vec2 offset;

float perlin(vec2 p) {
    
    const int hashSize = 256;                   // CREATES AND SHUFFLES HASH TABLE
    int hash[hashSize];
    for (int j = 0; j < hashSize; ++j) {
        hash[j] = j;
    }

    for (int j = 0; j < hashSize; ++j) {
        int k = int(mod(float(j) * 13.0, float(hashSize)));
        int temp = hash[j];
        hash[j] = hash[k];
        hash[k] = temp;
    }




    // Integer coordinates of the grid cell
    ivec2 i = ivec2(floor(p));
    
    // Fractional coordinates within the grid cell
    vec2 f = fract(p);

    // Smoothstep function for interpolation
    vec2 u = f * f * (3.0 - 2.0 * f);               // FADE

    vec2 uTest = f * f * f * (f * (f * 6 - vec2(15,15)) + vec2(10,10));



    // Gradient function for hash lookup
    vec2 gradient(ivec2 p) {
        return normalize(vec2(float(hash[hash[p.x & (hashSize - 1)] + p.y]), 
                              float(hash[hash[p.x & (hashSize - 1)] + p.y + 1])));
    }

    // Hash coordinates
    vec2 gi0 = gradient(i);
    vec2 gi1 = gradient(i + ivec2(1, 0));
    vec2 gi2 = gradient(i + ivec2(0, 1));
    vec2 gi3 = gradient(i + ivec2(1, 1));

    // Bilinear interpolation of gradients
    float a = dot(gi0, f);
    float b = dot(gi1, f - vec2(1, 0));
    float c = dot(gi2, f - vec2(0, 1));
    float d = dot(gi3, f - vec2(1, 1));

    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);    // LERP
}

void main() {
    // Get the global position within the compute grid
    ivec2 globalID = ivec2(gl_GlobalInvocationID.xy);
    
    // Calculate the corresponding position in the heightmap
    vec2 p = (vec2(globalID) + offset) / scale;

    // Initialize the heightmap value
    float height = 0.0;
    float amplitude = 1.0;
    
    // Generate Perlin noise
    for (int i = 0; i < octaves; i++) {
        height += perlin(p * pow(lacunarity, float(i))) * amplitude;
        amplitude *= persistence;
    }

    // Write the heightmap value to the output buffer
    imageStore(outputTexture, globalID, vec4(vec3(height), 1.0));
}