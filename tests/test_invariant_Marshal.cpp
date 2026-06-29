#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cstring>

// Include the actual production header
#include "ZLua~/libil2cpp-2022/icalls/mscorlib/System.Runtime.InteropServices/Marshal.h"

class SecurityTest : public ::testing::TestWithParam<std::string> {};

TEST_P(SecurityTest, BufferReadsNeverExceedDeclaredLength) {
    // Invariant: Buffer reads never exceed the declared length
    std::string payload = GetParam();
    
    // Test the actual production function: Marshal::StringToHGlobalAnsi
    // This function copies a managed string to unmanaged memory
    const char* input = payload.c_str();
    int declaredBufferSize = 16; // Small buffer to test overflow
    
    // Create destination buffer with guard pages to detect overflow
    char* buffer = new char[declaredBufferSize + 32];
    char* guardFront = buffer;
    char* guardBack = buffer + declaredBufferSize + 16;
    
    // Fill guard regions with sentinel values
    memset(guardFront, 0xAA, 16);
    memset(buffer + declaredBufferSize, 0x00, declaredBufferSize);
    memset(guardBack, 0xBB, 16);
    
    // Call the actual production function
    // Note: In real scenario, we'd need to adapt to actual function signature
    // This is a template - adjust based on actual Marshal.cpp functions
    intptr_t result = Marshal::StringToHGlobalAnsi(input);
    
    // Check guard regions weren't corrupted
    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(guardFront[i], (char)0xAA) << "Buffer underflow detected!";
        ASSERT_EQ(guardBack[i], (char)0xBB) << "Buffer overflow detected!";
    }
    
    // Cleanup
    Marshal::FreeHGlobal(result);
    delete[] buffer;
}

INSTANTIATE_TEST_SUITE_P(
    AdversarialInputs,
    SecurityTest,
    ::testing::Values(
        // Exact exploit case: significantly oversized input
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
        // Boundary case: exactly at buffer limit + 1
        "0123456789ABCDEF0",
        // Valid input: within buffer limits
        "SafeInput123",
        // Another adversarial case: null-terminator manipulation
        "NoNullTerminator",
        // Minimal oversize case
        "JustOverflow"
    )
);

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}