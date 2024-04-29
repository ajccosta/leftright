#pragma once

#include <map>
#define WRITE(m, k, v) (m[k] = v)
#define READ(m, k) (m.find(k)->second)
#define TYPE std::map<int, int>
