#include "ReadTxt.h"

#include <fstream>
#include <cstring>
#include <cstdio>
#include <stdexcept>

int DataRead::ReadData(char* data) {
    Pattern pattern;
    NoteEvent note;
    Instrument instr;

    std::ifstream file(data);
    if (!file.is_open()) {
        throw std::runtime_error("ERROR: The txt file did not open.");
    }

    char buffer[256];
    while (file.getline(buffer, sizeof(buffer))) {

        char* p = buffer;
        while (*p == ' ' || *p == '\t') ++p;

        if (*p == '#' || *p == '\0') continue;

        if (strncmp(p, "bpm", 3) == 0) {
            int bpm_value{};
            if (sscanf(p, "bpm %d", &bpm_value) == 1) bmp = bpm_value;
            continue;
        }

        else if (strncmp(p, "pattern", 7) == 0) {

            char name_pattern_buffer[64];
            int resolution_value{};
            if (sscanf(p, "pattern %s resolution %d", name_pattern_buffer, &resolution_value) == 2) {
                pattern = Pattern{};
                pattern.name_pattern = name_pattern_buffer;
                pattern.resolution   = resolution_value;
            }

            while (file.getline(buffer, sizeof(buffer))) {

                char* q = buffer;
                while (*q == ' ' || *q == '\t') ++q;

                if (*q == '#' || *q == '\0') continue;

                if (strncmp(q, "end", 3) == 0) break;

                int call_start = 0;
                char call_name[64];

                if (sscanf(q, "%d @%63s", &call_start, call_name) == 2) {
                    CallEvent call_event;
                    call_event.start = call_start;
                    call_event.pattern_name = call_name;
                    pattern.calls.push_back(call_event);
                    continue;
                }

                int start_value, duration_value, velocity_value;
                char instrument_name_buffer[64];
                char pitch_value_buffer[64];

                if (sscanf(q, "%d %s %s %d %d",
                           &start_value,
                           instrument_name_buffer,
                           pitch_value_buffer,
                           &duration_value,
                           &velocity_value) == 5) {

                    note.start = start_value;
                    note.instrument_name = instrument_name_buffer;
                    note.pitch = pitch_value_buffer;
                    note.duration = duration_value;
                    note.velocity = velocity_value;

                    pattern.notes.push_back(note);
                }
            }

            patterns.push_back(pattern);

            continue;
        }

        else if (strncmp(p, "instrument", 10) == 0) {

            char inst_name_buffer[64];
            char type_type_buffer[64];

            if (sscanf(p, "instrument %63s %63s",
                       inst_name_buffer,
                       type_type_buffer) == 2) {

                instr = Instrument{};
                instr.name_instr = inst_name_buffer;
                instr.params.clear();

                if (strcmp(type_type_buffer, "sampler") == 0) {
                    instr.type = InstrumentType::SAMPLER;
                }
                else if (strcmp(type_type_buffer, "square") == 0) {
                    instr.type = InstrumentType::SQUARE;
                }
                else if (strcmp(type_type_buffer, "sine") == 0) {
                    instr.type = InstrumentType::SINE;
                }
                else if (strcmp(type_type_buffer, "triangle") == 0) {
                    instr.type = InstrumentType::TRIANGLE;
                }
            }

            while (file.getline(buffer, sizeof(buffer))) {

                char* q = buffer;
                while (*q == ' ' || *q == '\t') ++q;

                if (*q == '#' || *q == '\0') continue;

                if (strncmp(q, "end", 3) == 0) break;

                char sample_buf[128];
                if (sscanf(q, "sample=%127s", sample_buf) == 1) {
                    instr.sampler.sample = sample_buf;
                    continue;
                }

                char root_buf[32];
                if (sscanf(q, "root=%31s", root_buf) == 1) {
                    instr.sampler.root = root_buf;
                    continue;
                }

                int loop_start{};
                int loop_end{};
                if (sscanf(q, "loop=%d,%d", &loop_start, &loop_end) == 2) {
                    instr.sampler.loop_start = loop_start;
                    instr.sampler.loop_end = loop_end;
                    continue;
                }

                float attack_buff{};
                if (sscanf(q, "attack=%f", &attack_buff) == 1) {

                    instr.sampler.attack = attack_buff;
                    instr.square.attack = attack_buff;
                    instr.sine.attack = attack_buff;
                    instr.triangle.attack = attack_buff;
                    continue;
                }

                float release_buff{};
                if (sscanf(q, "release=%f", &release_buff) == 1) {
                    instr.sampler.release = release_buff;
                    instr.square.release = release_buff;
                    instr.sine.release = release_buff;
                    instr.triangle.release = release_buff;
                    continue;
                }

                int duty_buff{};
                if (sscanf(q, "duty=%d", &duty_buff) == 1) {
                    instr.square.duty = duty_buff;
                    continue;
                }

                char effect_type_buf[64];
                if (sscanf(q, "effect %63s", effect_type_buf) == 1) {
                    if (!instr.params.empty()) instr.params += "; ";
                    instr.params += q;
                    continue;
                }

                if (!instr.params.empty()) instr.params += "; ";
                instr.params += q;
            }

            instruments.push_back(instr);

            continue;
        }
    }

    return 0;
}