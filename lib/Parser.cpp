#include "Parser.h"

#include <stdexcept>

bool Parser(int argc, char* argv[], Data& data) {
    if (argc < 3) { 
        throw std::runtime_error("ERROR: Not enough arguments");
    }

    data.kFileTxt = argv[1];
    data.kFileWav = argv[2];

    
    return 0;
}