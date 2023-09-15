//
// Created by Radzivon Bartoshyk on 05/09/2023.
//

#include "Rgb1010102.h"
#include <vector>
#include "ThreadPool.hpp"

#if HAVE_NEON

#include <arm_neon.h>

void convertRGBA1010102ToU8_NEON(const uint8_t *src, int srcStride, uint8_t *dst, int dstStride,
                                 int width, int height) {
    uint32x4_t mask = vdupq_n_u32(0x3FF); // Create a vector with 10 bits set
    auto maxColors = vdupq_n_u32(255);
    auto minColors = vdupq_n_u32(0);

    uint32_t testValue = 0x01020304;
    auto testBytes = reinterpret_cast<uint8_t *>(&testValue);

    bool littleEndian = false;
    if (testBytes[0] == 0x04) {
        littleEndian = true;
    } else if (testBytes[0] == 0x01) {
        littleEndian = false;
    }

    auto dstPtr = reinterpret_cast<uint8_t *>(dst);

    auto m8Bit = vdupq_n_u32(255);
    const uint32_t scalarMask = (1u << 10u) - 1u;

    for (int y = 0; y < height; ++y) {
        const uint8_t *srcPointer = src;
        uint8_t *dstPointer = dstPtr;
        int x;
        for (x = 0; x + 4 < width; x += 4) {
            uint32x4_t rgba1010102 = vld1q_u32(reinterpret_cast<const uint32_t *>(srcPointer));

            auto originalR = vshrq_n_u32(
                    vmulq_u32(vandq_u32(vshrq_n_u32(rgba1010102, 20), mask), m8Bit), 10);
            uint32x4_t r = vmaxq_u32(vminq_u32(originalR, maxColors), minColors);
            auto originalG = vshrq_n_u32(
                    vmulq_u32(vandq_u32(vshrq_n_u32(rgba1010102, 10), mask), m8Bit), 10);
            uint32x4_t g = vmaxq_u32(vminq_u32(originalG, maxColors), minColors);
            auto originalB = vshrq_n_u32(vmulq_u32(vandq_u32(rgba1010102, mask), m8Bit), 10);
            uint32x4_t b = vmaxq_u32(vminq_u32(originalB, maxColors), minColors);

            uint32x4_t a1 = vshrq_n_u32(rgba1010102, 30);

            uint32x4_t aq = vorrq_u32(vshlq_n_u32(a1, 8), vorrq_u32(vshlq_n_u32(a1, 6),
                                                                    vorrq_u32(vshlq_n_u32(a1, 4),
                                                                              vorrq_u32(
                                                                                      vshlq_n_u32(
                                                                                              a1,
                                                                                              2),
                                                                                      a1))));
            uint32x4_t a = vshrq_n_u32(vmulq_u32(aq, m8Bit), 10);

            uint8x8_t rUInt8 = vqmovn_u16(vqmovn_high_u32(vqmovn_u32(r), r));
            uint8x8_t gUInt8 = vqmovn_u16(vqmovn_high_u32(vqmovn_u32(g), g));
            uint8x8_t bUInt8 = vqmovn_u16(vqmovn_high_u32(vqmovn_u32(b), b));
            uint8x8_t aUInt8 = vqmovn_u16(vqmovn_high_u32(vqmovn_u32(a), a));

            uint8x8x4_t resultRGBA;
            // Interleave the channels to get RGBA format
            resultRGBA.val[0] = vzip_u8(bUInt8, bUInt8).val[0];
            resultRGBA.val[1] = vzip_u8(gUInt8, gUInt8).val[0];
            resultRGBA.val[2] = vzip_u8(rUInt8, rUInt8).val[1];
            resultRGBA.val[3] = vzip_u8(aUInt8, aUInt8).val[1];

            vst4_u8(reinterpret_cast<uint8_t *>(dstPointer), resultRGBA);

            srcPointer += 16;
            dstPointer += 16;
        }

        for (; x < width; ++x) {
            uint32_t rgba1010102 = reinterpret_cast<const uint32_t *>(srcPointer)[0];

            uint32_t b = (rgba1010102) & scalarMask;
            uint32_t g = (rgba1010102 >> 10) & scalarMask;
            uint32_t r = (rgba1010102 >> 20) & scalarMask;

            uint32_t a1 = (rgba1010102 >> 30);
            uint32_t a = (a1 << 8) | (a1 << 6) | (a1 << 4) | (a1 << 2) | a1;

            // Convert each channel to floating-point values
            auto rUInt16 = static_cast<uint16_t>(r);
            auto gUInt16 = static_cast<uint16_t>(g);
            auto bUInt16 = static_cast<uint16_t>(b);
            auto aUInt16 = static_cast<uint16_t>(a);

            auto dstCast = reinterpret_cast<uint16_t *>(dstPointer);

            if (littleEndian) {
                dstCast[0] = bUInt16;
                dstCast[1] = gUInt16;
                dstCast[2] = rUInt16;
                dstCast[3] = aUInt16;
            } else {
                dstCast[0] = rUInt16;
                dstCast[1] = gUInt16;
                dstCast[2] = bUInt16;
                dstCast[3] = aUInt16;
            }

            srcPointer += 4;
            dstPointer += 4;
        }

        src += srcStride;
        dstPtr += dstStride;
    }
}

void convertRGBA1010102ToU16_NEON(const uint8_t *src, int srcStride, uint16_t *dst, int dstStride,
                                  int width, int height) {
    uint32x4_t mask = vdupq_n_u32(0x3FF); // Create a vector with 10 bits set
    auto maxColors = vdupq_n_u32(1023);
    auto minColors = vdupq_n_u32(0);

    uint32_t testValue = 0x01020304;
    auto testBytes = reinterpret_cast<uint8_t *>(&testValue);

    bool littleEndian = false;
    if (testBytes[0] == 0x04) {
        littleEndian = true;
    } else if (testBytes[0] == 0x01) {
        littleEndian = false;
    }

    const uint32_t scalarMask = (1u << 10u) - 1u;

    auto dstPtr = reinterpret_cast<uint8_t *>(dst);

    for (int y = 0; y < height; ++y) {
        const uint8_t *srcPointer = src;
        uint8_t *dstPointer = dstPtr;
        int x;
        for (x = 0; x + 2 < width; x += 2) {
            uint32x4_t rgba1010102 = vld1q_u32(reinterpret_cast<const uint32_t *>(srcPointer));

            uint32x4_t r = vmaxq_u32(
                    vminq_u32(vandq_u32(vshrq_n_u32(rgba1010102, 20), mask), maxColors),
                    minColors);
            uint32x4_t g = vmaxq_u32(
                    vminq_u32(vandq_u32(vshrq_n_u32(rgba1010102, 10), mask), maxColors),
                    minColors);
            uint32x4_t b = vmaxq_u32(vminq_u32(vandq_u32(rgba1010102, mask), maxColors),
                                     minColors);

            uint32x4_t a1 = vshrq_n_u32(rgba1010102, 30);

            uint32x4_t a = vorrq_u32(vshlq_n_u32(a1, 8), vorrq_u32(vshlq_n_u32(a1, 6),
                                                                   vorrq_u32(vshlq_n_u32(a1, 4),
                                                                             vorrq_u32(
                                                                                     vshlq_n_u32(a1,
                                                                                                 2),
                                                                                     a1))));

            uint16x4_t rUInt16 = vqmovn_u32(b);
            uint16x4_t gUInt16 = vqmovn_u32(g);
            uint16x4_t bUInt16 = vqmovn_u32(r);
            uint16x4_t aUInt16 = vqmovn_u32(vmaxq_u32(vminq_u32(a, maxColors), minColors));

            uint16x4x4_t interleavedRGBA;
            interleavedRGBA.val[0] = rUInt16;
            interleavedRGBA.val[1] = gUInt16;
            interleavedRGBA.val[2] = bUInt16;
            interleavedRGBA.val[3] = aUInt16;

            vst4_u16(reinterpret_cast<uint16_t *>(dstPointer), interleavedRGBA);

            srcPointer += 8;
            dstPointer += 16;
        }

        for (; x < width; ++x) {
            uint32_t rgba1010102 = reinterpret_cast<const uint32_t *>(srcPointer)[0];
            uint32_t b = (rgba1010102) & scalarMask;
            uint32_t g = (rgba1010102 >> 10) & scalarMask;
            uint32_t r = (rgba1010102 >> 20) & scalarMask;

            uint32_t a1 = (rgba1010102 >> 30);
            uint32_t a = (a1 << 8) | (a1 << 6) | (a1 << 4) | (a1 << 2) | a1;

            // Convert each channel to floating-point values
            auto rFloat = std::clamp(static_cast<uint8_t>((r * 255) / 1023), (uint8_t) 255,
                                     (uint8_t) 0);
            auto gFloat = std::clamp(static_cast<uint8_t>((g * 255) / 1023), (uint8_t) 255,
                                     (uint8_t) 0);
            auto bFloat = std::clamp(static_cast<uint8_t>((b * 255) / 1023), (uint8_t) 255,
                                     (uint8_t) 0);
            auto aFloat = std::clamp(static_cast<uint8_t>((a * 255) / 1023), (uint8_t) 255,
                                     (uint8_t) 0);

            auto dstCast = reinterpret_cast<uint8_t *>(dstPointer);
            if (littleEndian) {
                dstCast[0] = bFloat;
                dstCast[1] = gFloat;
                dstCast[2] = rFloat;
                dstCast[3] = aFloat;
            } else {
                dstCast[0] = rFloat;
                dstCast[1] = gFloat;
                dstCast[2] = bFloat;
                dstCast[3] = aFloat;
            }

            srcPointer += 4;
            dstPointer += 4;
        }


        src += srcStride;
        dstPtr += dstStride;
    }
}

#endif

void convertRGBA1010102ToU16_C(const uint8_t *src, int srcStride, uint16_t *dst, int dstStride,
                               int width, int height) {
    auto mDstPointer = reinterpret_cast<uint8_t *>(dst);
    auto mSrcPointer = reinterpret_cast<const uint8_t *>(src);

    uint32_t testValue = 0x01020304;
    auto testBytes = reinterpret_cast<uint8_t *>(&testValue);

    bool littleEndian = false;
    if (testBytes[0] == 0x04) {
        littleEndian = true;
    } else if (testBytes[0] == 0x01) {
        littleEndian = false;
    }

    for (int y = 0; y < height; ++y) {

        auto dstPointer = reinterpret_cast<uint8_t *>(mDstPointer);
        auto srcPointer = reinterpret_cast<const uint8_t *>(mSrcPointer);

        for (int x = 0; x < width; ++x) {
            uint32_t rgba1010102 = reinterpret_cast<const uint32_t *>(srcPointer)[0];

            const uint32_t mask = (1u << 10u) - 1u;
            uint32_t b = (rgba1010102) & mask;
            uint32_t g = (rgba1010102 >> 10) & mask;
            uint32_t r = (rgba1010102 >> 20) & mask;

            uint32_t a1 = (rgba1010102 >> 30);
            uint32_t a = (a1 << 8) | (a1 << 6) | (a1 << 4) | (a1 << 2) | a1;

            // Convert each channel to floating-point values
            auto rUInt16 = static_cast<uint16_t>(r);
            auto gUInt16 = static_cast<uint16_t>(g);
            auto bUInt16 = static_cast<uint16_t>(b);
            auto aUInt16 = static_cast<uint16_t>(a);

            auto dstCast = reinterpret_cast<uint16_t *>(dstPointer);

            if (littleEndian) {
                dstCast[0] = bUInt16;
                dstCast[1] = gUInt16;
                dstCast[2] = rUInt16;
                dstCast[3] = aUInt16;
            } else {
                dstCast[0] = rUInt16;
                dstCast[1] = gUInt16;
                dstCast[2] = bUInt16;
                dstCast[3] = aUInt16;
            }

            srcPointer += 4;
            dstPointer += 8;
        }

        mSrcPointer += srcStride;
        mDstPointer += dstStride;
    }
}

void convertRGBA1010102ToU8_C(const uint8_t *src, int srcStride, uint8_t *dst, int dstStride,
                              int width, int height) {
    auto mDstPointer = reinterpret_cast<uint8_t *>(dst);
    auto mSrcPointer = reinterpret_cast<const uint8_t *>(src);

    uint32_t testValue = 0x01020304;
    auto testBytes = reinterpret_cast<uint8_t *>(&testValue);

    bool littleEndian = false;
    if (testBytes[0] == 0x04) {
        littleEndian = true;
    } else if (testBytes[0] == 0x01) {
        littleEndian = false;
    }

    const uint32_t mask = (1u << 10u) - 1u;

    for (int y = 0; y < height; ++y) {

        auto dstPointer = reinterpret_cast<uint8_t *>(mDstPointer);
        auto srcPointer = reinterpret_cast<const uint8_t *>(mSrcPointer);

        for (int x = 0; x < width; ++x) {
            uint32_t rgba1010102 = reinterpret_cast<const uint32_t *>(srcPointer)[0];

            uint32_t b = (rgba1010102) & mask;
            uint32_t g = (rgba1010102 >> 10) & mask;
            uint32_t r = (rgba1010102 >> 20) & mask;

            uint32_t a1 = (rgba1010102 >> 30);
            uint32_t a = (a1 << 8) | (a1 << 6) | (a1 << 4) | (a1 << 2) | a1;

            // Convert each channel to floating-point values
            auto rFloat = std::clamp(static_cast<uint8_t>((r * 255) / 1023), (uint8_t) 255,
                                     (uint8_t) 0);
            auto gFloat = std::clamp(static_cast<uint8_t>((g * 255) / 1023), (uint8_t) 255,
                                     (uint8_t) 0);
            auto bFloat = std::clamp(static_cast<uint8_t>((b * 255) / 1023), (uint8_t) 255,
                                     (uint8_t) 0);
            auto aFloat = std::clamp(static_cast<uint8_t>((a * 255) / 1023), (uint8_t) 255,
                                     (uint8_t) 0);

            auto dstCast = reinterpret_cast<uint8_t *>(dstPointer);
            if (littleEndian) {
                dstCast[0] = bFloat;
                dstCast[1] = gFloat;
                dstCast[2] = rFloat;
                dstCast[3] = aFloat;
            } else {
                dstCast[0] = rFloat;
                dstCast[1] = gFloat;
                dstCast[2] = bFloat;
                dstCast[3] = aFloat;
            }

            srcPointer += 4;
            dstPointer += 4;
        }

        mSrcPointer += srcStride;
        mDstPointer += dstStride;
    }
}

void RGBA1010102ToU8(const uint8_t *src, int srcStride, uint8_t *dst, int dstStride,
                     int width, int height) {
#if HAVE_NEON
    convertRGBA1010102ToU8_NEON(src, srcStride, dst, dstStride, width, height);
#else
    convertRGBA1010102ToU8_C(src, srcStride, dst, dstStride, width, height);
#endif
}

void RGBA1010102ToU16(const uint8_t *src, int srcStride, uint16_t *dst, int dstStride,
                      int width, int height) {
#if HAVE_NEON
    convertRGBA1010102ToU16_NEON(src, srcStride, dst, dstStride, width, height);
#else
    convertRGBA1010102ToU16_C(src, srcStride, dst, dstStride, width, height);
#endif
}