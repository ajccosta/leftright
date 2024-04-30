#pragma once

#define FAKE_WORK(n) { std::atomic_uint_fast32_t __noop {0}; for(int i = 0; i < n; i++) { __noop++; } }
