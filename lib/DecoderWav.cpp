#include "DecoderWav.h"
#include "Pitch.h"
#include "Envelope.h"
#include "SampleLoader.h"
#include "ReadTxt.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <utility>
#include <cstddef>

namespace {
const int kHz = 44100;

struct ResolvedNote {
    double start_beats{};
    double duration_beats{};
    std::string instrument_name;
    std::string pitch;
    int velocity{};
};

Instrument* FindInstr(DataRead& r, const std::string& name) {
    for (auto& i : r.instruments) {
        if (i.name_instr == name) {
            return &i;
        }
    }
    return nullptr;
}

Pattern* FindPat(DataRead& r, const std::string& name) {
    for (auto& p : r.patterns) {
        if (p.name_pattern == name) {
            return &p;
        }
    }
    return nullptr;
}

void PatternConnect(DataRead& r, Pattern* pat, double off_beats, std::vector<ResolvedNote>& out) {
    if (pat == nullptr) {
        throw std::runtime_error("ERROR: Pattern was not found");
    }
    if (pat->resolution <= 0) {
        throw std::runtime_error("ERROR: Pattern resolution must be positive");
    }

    for (const auto& n : pat->notes) {
        ResolvedNote rn;
        rn.start_beats = off_beats + static_cast<double>(n.start) / pat->resolution;
        rn.duration_beats = static_cast<double>(n.duration) / pat->resolution;
        rn.instrument_name = n.instrument_name;
        rn.pitch = n.pitch;
        rn.velocity = n.velocity;
        out.push_back(rn);
    }

    for (const auto& c : pat->calls) {
        const double call_off_beats = off_beats + static_cast<double>(c.start) / pat->resolution;
        PatternConnect(r, FindPat(r, c.pattern_name), call_off_beats, out);
    }
}

float ParamValue(const std::string& text, const std::string& key, float def) {
    const std::string needle = key + "=";
    const size_t p = text.find(needle);
    if (p == std::string::npos) return def;

    const size_t start = p + needle.size();
    const size_t end = text.find_first_of(" \t;\r\n", start);
    try {
        return std::stof(text.substr(start, end == std::string::npos ? end : end - start));
    } catch (...) {
        return def;
    }
}

std::vector<std::string> EffectLines(const std::string& params) {
    std::vector<std::string> lines;
    std::stringstream ss(params);
    std::string item;
    while (std::getline(ss, item, ';')) {
        const size_t first = item.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        const size_t last = item.find_last_not_of(" \t\r\n");
        item = item.substr(first, last - first + 1);
        if (item.rfind("effect ", 0) == 0) lines.push_back(item);
    }
    return lines;
}

inline float Wave(InstrumentType t, double ph, int duty) {
    if (t == InstrumentType::SINE) return std::sin(2.0 * M_PI * ph);
    if (t == InstrumentType::SQUARE) return (ph < duty / 100.0) ? 1.f : -1.f;
    return 4.0 * std::abs(ph - 0.5) - 1.0;
}

void RenderOsc(const ResolvedNote& n, const Instrument& i, int start, int len, std::vector<float>& buf) {
    float vel = std::clamp(n.velocity / 100.f, 0.f, 1.f);
    double f = Pitch(n.pitch);

    float attack = 0, release = 0;
    int duty = 50;
    if (i.type == InstrumentType::SINE) {
        attack = i.sine.attack; release = i.sine.release;
    }
    if (i.type == InstrumentType::SQUARE) {
        attack = i.square.attack;
        release = i.square.release;
        duty = std::clamp(i.square.duty, 1, 99);
    }
    if (i.type == InstrumentType::TRIANGLE) {
        attack = i.triangle.attack;
        release = i.triangle.release;
    }

    const int safe_len = std::min(len, static_cast<int>(buf.size()) - start);
    for (int s = 0; s < safe_len; s++) {
        float env = EnvelopeAR(s, safe_len, attack, release);
        double ph = std::fmod(s * f / kHz, 1.0);
        buf[start + s] += vel * env * Wave(i.type, ph, duty);
    }
}

const std::vector<float>& Sample(const std::string& path) {
    static std::vector<std::pair<std::string, std::vector<float>>> c;
    for (auto& ptr : c) {
        if (ptr.first == path) {
            return ptr.second;
        }
    }
    c.push_back({path, {}});
    LoadWav(path, c.back().second);
    return c.back().second;
}

float SamplerValue(const std::vector<float>& x, double pos, int loop_start, int loop_end) {
    if (x.empty() || pos < 0.0) return 0.0f;

    const bool has_loop = loop_end > loop_start &&
                          loop_start >= 0 &&
                          loop_end <= static_cast<int>(x.size());

    if (has_loop && pos >= loop_end) {
        const double loop_len = static_cast<double>(loop_end - loop_start);
        pos = loop_start + std::fmod(pos - loop_start, loop_len);
    } else if (!has_loop && pos >= static_cast<double>(x.size())) {
        return 0.0f;
    }

    return SampleAt(x, pos);
}

void RenderSampler(const ResolvedNote& n, const Instrument& i, int start, int len, std::vector<float>& buf) {
    const auto& x = Sample(i.sampler.sample);
    if (x.empty()) return;

    float vel = std::clamp(n.velocity / 100.f, 0.f, 1.f);
    float attack = i.sampler.attack;
    float release = i.sampler.release;

    double step = Pitch(n.pitch) / Pitch(i.sampler.root);
    double pos = 0.0;

    const int safe_len = std::min(len, static_cast<int>(buf.size()) - start);
    for (int s = 0; s < safe_len; ++s) {
        float env = EnvelopeAR(s, safe_len, attack, release);
        buf[start + s] += vel * env * SamplerValue(x, pos, i.sampler.loop_start, i.sampler.loop_end);
        pos += step;
    }
}

void ApplyGain(std::vector<float>& buf, float gain) {
    for (float& x : buf) x *= gain;
}

void ApplyEcho(std::vector<float>& buf, float delay_s, float decay) {
    const int delay = std::max(0, static_cast<int>(std::lround(delay_s * kHz)));
    if (delay <= 0 || decay == 0.0f || buf.empty()) return;

    const std::vector<float> dry = buf;
    buf.resize(buf.size() + delay, 0.0f);
    for (size_t i = 0; i < dry.size(); ++i) {
        buf[i + delay] += dry[i] * decay;
    }
}

void ApplyTremolo(std::vector<float>& buf, float freq, float depth) {
    if (freq <= 0.0f || depth <= 0.0f) return;
    depth = std::clamp(depth, 0.0f, 1.0f);
    for (size_t s = 0; s < buf.size(); ++s) {
        const double t = static_cast<double>(s) / kHz;
        const float mod = 1.0f - depth + depth * std::sin(2.0 * M_PI * freq * t);
        buf[s] *= mod;
    }
}

void ApplyEffects(const Instrument& instr, std::vector<float>& track) {
    for (const auto& e : EffectLines(instr.params)) {
        if (e.find("effect gain") == 0) {
            ApplyGain(track, ParamValue(e, "gain", 1.0f));
        } else if (e.find("effect echo") == 0) {
            ApplyEcho(track, ParamValue(e, "delay", 0.0f), ParamValue(e, "decay", 0.0f));
        } else if (e.find("effect tremolo") == 0) {
            ApplyTremolo(track, ParamValue(e, "freq", 0.0f), ParamValue(e, "depth", 0.0f));
        }
    }
}

void AddTrack(std::vector<float>& mix, const std::vector<float>& track) {
    if (mix.size() < track.size()) mix.resize(track.size(), 0.0f);
    for (size_t i = 0; i < track.size(); ++i) mix[i] += track[i];
}

} // namespace

std::vector<float> Render(DataRead& read) {
    Pattern* main = FindPat(read, "main");
    if (main == nullptr) {
        throw std::runtime_error("ERROR: main pattern was not found");
    }
    if (read.bmp <= 0) {
        throw std::runtime_error("ERROR: BPM must be positive");
    }

    const double beat_s = 60.0 / read.bmp;

    std::vector<ResolvedNote> notes;
    PatternConnect(read, main, 0.0, notes);

    double last_beat = 0.0;
    for (const auto& n : notes) {
        last_beat = std::max(last_beat, n.start_beats + n.duration_beats);
    }

    int total = std::max(1, static_cast<int>(std::lround(last_beat * beat_s * kHz)));
    std::vector<float> mix(total, 0.f);

    for (const auto& instr : read.instruments) {
        std::vector<float> track(total, 0.f);

        for (const auto& n : notes) {
            if (n.instrument_name != instr.name_instr) continue;

            int start = static_cast<int>(std::lround(n.start_beats * beat_s * kHz));
            int len = static_cast<int>(std::lround(n.duration_beats * beat_s * kHz));
            if (start < 0 || len <= 0 || start >= static_cast<int>(track.size())) continue;

            if (instr.type == InstrumentType::SAMPLER) {
                RenderSampler(n, instr, start, len, track);
            } else {
                RenderOsc(n, instr, start, len, track);
            }
        }

        ApplyEffects(instr, track);
        AddTrack(mix, track);
    }

    for (const auto& n : notes) {
        if (FindInstr(read, n.instrument_name) == nullptr) {
            throw std::runtime_error("ERROR: Instrument was not found: " + n.instrument_name);
        }
    }

    return mix;
}
