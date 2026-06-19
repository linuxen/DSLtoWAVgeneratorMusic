#pragma once

#include <string>
#include <vector>

bool LoadWav(const std::string& path, std::vector<float>& out);
float SampleAt(const std::vector<float>& x, double pos);