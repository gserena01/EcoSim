#include "Texture.h"
#include <fstream>
#include "vec.h"
#include <cmath>



void Texture::loadFile(const std::string& fileName)
{
}

float fract(float x) {
    return x - floor(x);
}

vec3 fract(vec3 v) {
    return vec3(fract(v[0]), fract(v[1]), fract(v[2]));
}

float dot(vec3 u, vec3 v) {
    return (u[0] * v[0] + u[1] * v[1] + u[2] * v[2]);
}

vec3 sin(vec3 v) {
    return vec3(sin(v[0]), sin(v[1]), sin(v[2]));
}

vec3 random3(vec3 p) {
    return fract(sin(vec3(dot(p, vec3(127.1, 311.7, 457.3)), dot(p, vec3(269.5, 183.3, 271.5)), dot(p, vec3(119.3, 257.1, 361.7)))) * 43758.5453);
}

vec3 random32(vec3 p) {
    return fract(sin(vec3(dot(p, vec3(69.1, 311.7, 457.3)), dot(p, vec3(269.5, 420.69, 271.5)), dot(p, vec3(119.3, 257.1, 369.7)))) * 43758.5453);
}

float WorleyNoise(vec3 uv)
{
    // Tile the space
    vec3 uvInt = vec3(floor(uv[0]), floor(uv[1]), floor(uv[2]));
    vec3 uvFract = fract(uv);

    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for (int z = -1; z <= 1; z++) {
        for (int y = -1; y <= 1; y++)
        {
            for (int x = -1; x <= 1; x++)
            {
                vec3 neighbor = vec3(float(x), float(y), float(z));

                // Random point inside current neighboring cell
                vec3 point = random3(uvInt + neighbor);

                // Animate the point
                //point = 0.5 + 0.5 * sin(iTime + 6.2831 * point); // 0 to 1 range

                // Compute the distance b/t the point and the fragment
                // Store the min dist thus far
                vec3 diff = neighbor + point - uvFract;
                float dist = sqrt(diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2]);
                minDist = min(minDist, dist);
            }
        }
    }
    return minDist;
}

float WorleyNoise2(vec3 uv)
{
    // Tile the space
    vec3 uvInt = vec3(floor(uv[0]), floor(uv[1]), floor(uv[2]));
    vec3 uvFract = fract(uv);

    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for (int z = -1; z <= 1; z++) {
        for (int y = -1; y <= 1; y++)
        {
            for (int x = -1; x <= 1; x++)
            {
                vec3 neighbor = vec3(float(x), float(y), float(z));

                // Random point inside current neighboring cell
                vec3 point = random32(uvInt + neighbor);

                // Animate the point
                //point = 0.5 + 0.5 * sin(iTime + 6.2831 * point); // 0 to 1 range

                // Compute the distance b/t the point and the fragment
                // Store the min dist thus far
                vec3 diff = neighbor + point - uvFract;
                float dist = sqrt(diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2]);
                minDist = min(minDist, dist);
            }
        }
    }
    return minDist;
}

vec3 fbm(vec3 uv) {
    float amp = 0.5;
    float freq = 1.0;
    vec3 sum = vec3(0.0, 0.0, 0.0);
    float maxSum = 0.0;
    for (int i = 0; i < 4; i++) {
        sum[0] += WorleyNoise(uv * freq) * amp;
        sum[1] += WorleyNoise(uv * freq) * amp;
        sum[2] += WorleyNoise(uv * freq) * amp;
        maxSum += amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum / maxSum;
}

vec3 fbm2(vec3 uv) {
    float amp = 0.5;
    float freq = 10.0;
    vec3 sum = vec3(0.0, 0.0, 0.0);
    float maxSum = 0.0;
    for (int i = 0; i < 4; i++) {
        sum[0] += WorleyNoise2(uv * freq) * amp;
        sum[1] += WorleyNoise2(uv * freq) * amp;
        sum[2] += WorleyNoise2(uv * freq) * amp;
        maxSum += amp;
        amp *= 0.0512;
        freq *= 2.88888;
    }
    return sum / maxSum;
}

float grayscale(vec3 v) {
    return .3 * v[0] + .6 * v[1] + .1 * v[2];
}

float make_it_moist(vec3 v) {
    return v[0] + v[1] + v[2];
}

std::array<std::array<std::array<float, 4>, 32>, 32> makeVaporMap(int resolution, int depth) {
    std::array<std::array<std::array<float, 4>, 32>, 32> tex_3d = { {{0.0}} };
    for (int i = 0; i < resolution; ++i) {
        for (int j = 0; j < resolution; ++j) {
            for (int k = 0; k < depth; ++k) {
                tex_3d[i][j][k] = grayscale(fbm(vec3(1.0 / i, 1.0 / j, 1.0 / k)));
            }
        }
    }
    return tex_3d;
}

std::array<std::array<float, 32>, 32> makeSoilMap(int resolution) {
    std::array<std::array<float, 32>, 32> tex_3d = { {{0.0}} };
    for (int i = 0; i < resolution; ++i) {
        for (int j = 0; j < resolution; ++j) {
            tex_3d[i][j] = make_it_moist(fbm2(vec3(i, j, 1.0)));
        }
    }
    return tex_3d;
}

Texture::Texture(int resolution, int depth) {
    this->tex_arr_3d = makeVaporMap(resolution, depth);
    this->tex_arr_2d = makeSoilMap(resolution);
}

Texture::Texture() {
    this->tex_arr_3d = makeVaporMap(32, 4);
    this->tex_arr_2d = makeSoilMap(32);
}