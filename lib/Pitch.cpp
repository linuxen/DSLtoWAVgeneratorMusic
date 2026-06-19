#include "Pitch.h"

#include <string>
#include <cmath>

double Pitch(const std::string& note) {
    int note_idx = 0;
    switch (note[0]) {
        case 'C': note_idx = 0; break;
        case 'D': note_idx = 2; break;
        case 'E': note_idx = 4; break;
        case 'F': note_idx = 5; break;
        case 'G': note_idx = 7; break;
        case 'A': note_idx = 9; break;
        case 'B': note_idx = 11; break;
    }

    int pos = 1;
    if (pos < (int)note.size() && note[pos] == '#') { 
        note_idx += 1; 
        pos++; 
    }
    else if (pos < (int)note.size() && note[pos] == 'b') { 
        note_idx -= 1; 
        pos++; 
    }

    int oct = (pos < (int)note.size() ? note[pos] - '0' : 4);

    int midi = (oct + 1) * 12 + note_idx;
    return 440.0 * std::pow(2.0, (midi - 69) / 12.0);
}
