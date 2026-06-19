set(CMAKE_CXX_STANDARD 20)

add_library(music lib.cpp DecoderWav.cpp Parser.cpp ReadTxt.cpp Envelope.cpp Pitch.cpp SampleLoader.cpp)
target_include_directories(music PUBLIC .)
