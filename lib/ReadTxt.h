#pragma once

#include <string>
#include <vector>

struct NoteEvent {
    int start{};
    int duration{};
    std::string instrument_name;
    std::string pitch;
    int velocity{};
};

struct CallEvent {
    int start{};
    std::string pattern_name;
};

struct Pattern {
    std::string name_pattern;
    int resolution{};
    std::vector<NoteEvent> notes;
    std::vector<CallEvent> calls;
};

enum class InstrumentType {
    SAMPLER,
    SQUARE,
    SINE,
    TRIANGLE
};

struct Effect {
    std::string name_effect;
};

struct Sampler {
    std::string sample;
    std::string root;
    int loop_start{};
    int loop_end{};
    float attack{};
    float release{};
};

struct Square {
    int duty{};
    float attack{};
    float release{};
};

struct SineTriangle {
    float attack{};
    float release{};
};

struct Instrument {
    std::string name_instr;
    InstrumentType type;
    std::string params;
    Sampler sampler;
    Square square;
    SineTriangle sine;
    SineTriangle triangle;
};

class DataRead {
public:
    int bmp = 120;
    int ReadData(char* data);
    std::vector<Instrument> instruments;
    std::vector<Pattern> patterns;
};