#include "lib.h"

#include <fstream>
#include <cstdint>
#include <cmath>
#include <vector>
#include <algorithm>
#include <string>
#include <stdexcept>

static void W32(std::ofstream& out, uint32_t write) { out.write(reinterpret_cast<char*>(&write), 4); }
static void W16(std::ofstream& out, uint16_t write) { out.write(reinterpret_cast<char*>(&write), 2); }

void WriteWavMono16(const std::string& path, const std::vector<float>& mix) {
    std::ofstream out(path, std::ios::binary);
    if (!out) throw std::runtime_error("WAV open failed");

    const uint32_t data_size = static_cast<uint32_t>(mix.size() * 2);
    const int khz = 44100;

    out.write("RIFF", 4);
    W32(out, 36 + data_size);
    out.write("WAVE", 4);
    out.write("fmt ", 4);
    W32(out, 16);
    W16(out, 1);
    W16(out, 1);
    W32(out, khz);
    W32(out, khz * 2);
    W16(out, 2);
    W16(out, 16);

    out.write("data", 4);
    W32(out, data_size);

    float peak = 0.0f;
    for (float x : mix) peak = std::max(peak, std::abs(x));
    const float scale = (peak > 1.0f) ? (0.98f / peak) : 1.0f;

    for (float x : mix) {
        const float y = std::clamp(x * scale, -1.0f, 1.0f);
        int16_t s = static_cast<int16_t>(std::lround(y * 32767.f));
        out.write(reinterpret_cast<char*>(&s), 2);
    }
}
