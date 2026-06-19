#include "Parser.h"
#include "ReadTxt.h"
#include "DecoderWav.h"
#include "lib.h"

int main(int argc, char** argv) {
    Data data;
    Parser(argc, argv, data);

    DataRead read;

    read.ReadData(data.kFileTxt);

    auto mix = Render(read);

     WriteWavMono16(data.kFileWav, mix);

    return 0;
}
