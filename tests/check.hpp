#pragma once
#include <cstdlib>
#include <iostream>
#define CHECK(expr) do { if(!(expr)) {std::cerr << __FILE__ << ':' << __LINE__ << ": CHECK failed: " #expr << '\n'; std::abort();} } while(false)
