#pragma once

#pragma message("Don't worry, this is just a pragma message in fake1.cpp")

#pragma GCC optimize("O3")

#pragma pack(1)

#pragma warning(disable : 4996)

#pragma clang diagnostic ignored "-Wunused-variable"

int main() {
    int x = 42; // This variable is intentionally unused
    return 0;
}

