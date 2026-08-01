#pragma once

namespace logging {

void init();

void printf(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

[[noreturn]] void fatal(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

}
