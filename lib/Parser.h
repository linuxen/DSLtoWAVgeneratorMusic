#pragma once

struct Data {
    char* kFileTxt = nullptr;
    char* kFileWav = nullptr;
};

bool Parser(int argc, char* argv[], Data& data);