#include "SampleLoader.h"

#include <fstream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <string>
#include <cstring>

namespace {
template <typename T>
static bool Read(std::ifstream& in, T& v) {
    return static_cast<bool>(in.read(reinterpret_cast<char*>(&v), sizeof(T)));
}

static bool Read4(std::ifstream& in, char b[4]) {
    return static_cast<bool>(in.read(b, 4));
}

static bool FourCC(const char b[4], const char* s) {
    return std::memcmp(b, s, 4) == 0;
}
} // namespace

bool LoadWav(const std::string& path, std::vector<float>& out) {
    out.clear();
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;

    char id[4];
    uint32_t riff_size = 0;
    char wave[4];
    if (!Read4(in, id) || !Read(in, riff_size) || !Read4(in, wave)) return false;
    if (!FourCC(id, "RIFF") || !FourCC(wave, "WAVE")) return false;

    uint16_t audio_format = 0;
    uint16_t channels = 0;
    uint32_t sample_rate = 0;
    uint16_t bits_per_sample = 0;
    std::vector<char> data;

    while (in && (!audio_format || data.empty())) {
        char chunk_id[4];
        uint32_t chunk_size = 0;
        if (!Read4(in, chunk_id) || !Read(in, chunk_size)) break;

        if (FourCC(chunk_id, "fmt ")) {
            uint32_t byte_rate = 0;
            uint16_t block_align = 0;
            Read(in, audio_format);
            Read(in, channels);
            Read(in, sample_rate);
            Read(in, byte_rate);
            Read(in, block_align);
            Read(in, bits_per_sample);
            if (chunk_size > 16) in.seekg(chunk_size - 16, std::ios::cur);
        } else if (FourCC(chunk_id, "data")) {
            data.resize(chunk_size);
            in.read(data.data(), chunk_size);
        } else {
            in.seekg(chunk_size, std::ios::cur);
        }

        if (chunk_size % 2 == 1) in.seekg(1, std::ios::cur);
    }

    if (audio_format != 1 || channels == 0 || bits_per_sample != 16 || data.empty()) {
        return false;
    }

    const int bytes_per_frame = channels * 2;
    const int frames = static_cast<int>(data.size()) / bytes_per_frame;
    out.resize(frames);

    const auto* p = reinterpret_cast<const uint8_t*>(data.data());
    for (int i = 0; i < frames; ++i) {
        int sum = 0;
        for (int ch = 0; ch < channels; ++ch) {
            const int idx = i * bytes_per_frame + ch * 2;
            const int16_t s = static_cast<int16_t>(p[idx] | (p[idx + 1] << 8));
            sum += s;
        }
        out[i] = static_cast<float>(sum) / (32768.0f * channels);
    }

    return sample_rate == 44100;
}

float SampleAt(const std::vector<float>& x, double pos) {
    if (x.empty()) return 0.0f;
    pos = std::clamp(pos, 0.0, static_cast<double>(x.size() - 1));
    int i = static_cast<int>(pos);
    int j = std::min(i + 1, static_cast<int>(x.size() - 1));
    float f = static_cast<float>(pos - i);
    return x[i] + (x[j] - x[i]) * f;
}
