
/*-
 * $Copyright$
 */

#include <gtest/gtest.h>
#include <ctime>
#include <cstdlib>
#include <cstdint>
#include <array>
#include <vector>

#include "fifocopy.hpp"

/******************************************************************************/
struct ScatteredMemory {
    union {
        uint16_t    m_data16;
        uint8_t     m_data8[2];
    }           m_data;
    uint16_t    m_reserved;

    /*
     * This conversion operator is used when a std::array<ScatteredMemory, ...>::iterator
     * is dereferenced and used in bit-shift operation. Notably, this happens in the
     * stm32::copy() function.
     */
    operator int() const { return m_data.m_data16; }

    ScatteredMemory(void) = default;
    constexpr ScatteredMemory(uint16_t p_data) : m_data { .m_data16 = p_data }, m_reserved(0) { };
} __attribute__((packed));
static_assert(4 == sizeof(ScatteredMemory));
static_assert(2 == sizeof(ScatteredMemory::m_data));
/******************************************************************************/

/******************************************************************************/
template<
    size_t  nBytes
>
struct ScatteredArray : public std::array<ScatteredMemory, nBytes/sizeof(uint16_t)> {
    static constexpr size_t fifo_width = sizeof(uint16_t);

    static_assert(nBytes >= sizeof(uint16_t));
    static_assert(0 == (nBytes % sizeof(uint16_t)));
};
/******************************************************************************/

/******************************************************************************/
template<
    size_t nBytes,
    typename T
>
struct FifoArray : public std::array<T, nBytes/sizeof(T)> {
    static constexpr size_t fifo_width = sizeof(T);

    static_assert(nBytes >= sizeof(T));
    static_assert(0 == (nBytes % sizeof(T)));
};
/******************************************************************************/

/******************************************************************************/
class FifoCopyTest : public ::testing::TestWithParam<size_t> {
protected:
    static constexpr unsigned m_bufferSz = 64u;

    std::array<uint8_t, m_bufferSz> m_sourceData {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
        0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    };
    static_assert(m_bufferSz == sizeof(m_sourceData));

    template<typename T>
    void
    copyData(size_t p_byteCount) {
        std::vector<uint8_t> targetBuffer(p_byteCount);
        T packet;

        stm32::copy_to_fifo(m_sourceData, packet, p_byteCount);
        stm32::copy_from_fifo(packet, targetBuffer, p_byteCount);
        EXPECT_TRUE(std::equal(targetBuffer.cbegin(), targetBuffer.cend(), m_sourceData.begin()));
    }
};
/******************************************************************************/

/******************************************************************************/
namespace FifoCopyByte {
/******************************************************************************/
struct FifoCopyByteTest : public FifoCopyTest {
    void
    copyData(void) {
        FifoCopyTest::copyData< FifoArray<m_bufferSz, uint16_t> >(GetParam());
    }
};

TEST_P(FifoCopyByteTest, Copy) {
    copyData();
}

INSTANTIATE_TEST_SUITE_P(
    FifoCopyByteTests,
    FifoCopyByteTest,
    ::testing::Values(
        1, 2, 4, 8, 64
    )
);
/******************************************************************************/
} /* namespace FifoCopyByte */
/******************************************************************************/

/******************************************************************************/
namespace FifoCopyHalfWord {
/******************************************************************************/
struct FifoCopyHalfWordTest : public FifoCopyTest {
    void
    copyData(void) {
        FifoCopyTest::copyData< FifoArray<m_bufferSz, uint16_t> >(GetParam());
    }
};

TEST_P(FifoCopyHalfWordTest, Copy) {
    copyData();
}

INSTANTIATE_TEST_SUITE_P(
    FifoCopyHalfWordTests,
    FifoCopyHalfWordTest,
    ::testing::Values(
            1, 2, 4, 8, 64
    )
);

/******************************************************************************/
} /* namespace FifoCopyHalfWord */
/******************************************************************************/

/******************************************************************************/
namespace FifoCopyWord {
/******************************************************************************/
struct FifoCopyWordTest : public FifoCopyTest {
    void
    copyData(void) {
        FifoCopyTest::copyData< FifoArray<m_bufferSz, uint32_t> >(GetParam());
    }
};

TEST_P(FifoCopyWordTest, Copy) {
    copyData();
}

INSTANTIATE_TEST_SUITE_P(
    FifoCopyWordTests,
    FifoCopyWordTest,
    ::testing::Values(
            1, 2, 4, 8, 64
    )
);

TEST_F(FifoCopyWordTest, PacketLargerThanBuffer) {
    constexpr unsigned byteCount = m_bufferSz;
    std::array<uint8_t, byteCount - 1> targetBuffer;
    FifoArray<m_bufferSz, uint16_t> packet;

    stm32::copy_to_fifo(m_sourceData, packet, byteCount);
    stm32::copy_from_fifo(packet, targetBuffer, byteCount);
    EXPECT_TRUE(std::equal(targetBuffer.cbegin(), targetBuffer.cend(), m_sourceData.begin()));
}

TEST_F(FifoCopyWordTest, MoreThanPacket) {
    constexpr unsigned byteCount = m_bufferSz + 1;
    std::array<uint8_t, byteCount> targetBuffer;
    FifoArray<m_bufferSz, uint16_t> packet;

    stm32::copy_to_fifo(m_sourceData, packet, byteCount);
    stm32::copy_from_fifo(packet, targetBuffer, byteCount);
    EXPECT_TRUE(std::equal(targetBuffer.cbegin(), targetBuffer.cend() - 1, m_sourceData.begin()));
}

/******************************************************************************/
} /* namespace FifoCopyWord */
/******************************************************************************/

/******************************************************************************/
namespace FifoCopyScattered {
/******************************************************************************/

struct FifoCopyScatteredTest : public FifoCopyTest {
    void
    copyData(void) {
        FifoCopyTest::copyData< ScatteredArray<m_bufferSz> >(GetParam());
    }
};

TEST_P(FifoCopyScatteredTest, Copy) {
    copyData();
}

INSTANTIATE_TEST_SUITE_P(
    FifoCopyScatteredTests,
    FifoCopyScatteredTest,
    ::testing::Values(
            1, 2, 4, 8, 64
    )
);

TEST_F(FifoCopyScatteredTest, PacketLargerThanBuffer) {
    constexpr unsigned byteCount = m_bufferSz;
    std::array<uint8_t, byteCount - 1> targetBuffer;
    ScatteredArray<m_bufferSz> packet;

    stm32::copy_to_fifo(m_sourceData, packet, byteCount);
    stm32::copy_from_fifo(packet, targetBuffer, byteCount);
    EXPECT_TRUE(std::equal(targetBuffer.cbegin(), targetBuffer.cend(), m_sourceData.begin()));
}

TEST_F(FifoCopyScatteredTest, MoreThanPacket) {
    constexpr unsigned byteCount = m_bufferSz + 1;
    std::array<uint8_t, byteCount> targetBuffer;
    ScatteredArray<m_bufferSz> packet;

    stm32::copy_to_fifo(m_sourceData, packet, byteCount);
    stm32::copy_from_fifo(packet, targetBuffer, byteCount);
    EXPECT_TRUE(std::equal(targetBuffer.cbegin(), targetBuffer.cend() - 1, m_sourceData.begin()));
}

/******************************************************************************/
} /* namespace FifoCopyScattered */
/******************************************************************************/
