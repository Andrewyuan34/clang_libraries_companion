#pragma once

#pragma message("Don't worry, this is just a pragma message in fake2.cpp")

#pragma GCC optimize("O2")

#pragma pack(4)

#pragma warning(disable : 4201)

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

int main() {
    int y = 100; // This variable is intentionally unused
    return 0;
}

