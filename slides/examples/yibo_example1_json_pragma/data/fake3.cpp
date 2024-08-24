#pragma once

#pragma message("Don't worry, this is just a pragma message in fake3.cpp")

#pragma GCC optimize("Os")

#pragma pack(8)

#pragma warning(disable : 4100)

#pragma clang diagnostic ignored "-Wreturn-type"

#pragma region TestRegion
void testFunction() {
    // Function intentionally left empty
}
#pragma endregion

int main() {
    testFunction();
    return 0;
}

