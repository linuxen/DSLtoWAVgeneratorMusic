#include "Envelope.h"

#include <algorithm>
#include <cmath>

float EnvelopeAR(int i, int len, float attack_s, float release_s) {
    const int khz = 44100;
    const int attack_len = (int)std::lround(attack_s * khz);
    const int release_len = (int)std::lround(release_s * khz);

    float attack  = (attack_len > 0 && i < attack_len) ? (float)i / (float)attack_len : 1.0f;

    const int rel_start = len - release_len;
    float release = (release_len > 0 && i >= rel_start) ? 1.0f - (float)(i - rel_start) / (float)release_len : 1.0f;

    float env = std::min(attack, release);
    return std::clamp(env, 0.0f, 1.0f);
}