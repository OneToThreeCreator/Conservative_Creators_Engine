#ifndef ENDIANESS_H
#define ENDIANESS_H

#include <stdint.h>
#include "engine_common.h"

typedef uint8_t cce_endianess;

#define CCE_BIG_ENDIAN 0
#define CCE_LITTLE_ENDIAN 1

CCE_PUBLIC_OPTIONS extern const cce_endianess *const g_endianess;

CCE_PUBLIC_OPTIONS extern uint16_t (*cceLittleEndianConversionInt16) (uint16_t);
CCE_PUBLIC_OPTIONS extern uint32_t (*cceLittleEndianConversionInt32) (uint32_t);
CCE_PUBLIC_OPTIONS extern uint64_t (*cceLittleEndianConversionInt64) (uint64_t);
CCE_PUBLIC_OPTIONS extern void* (*cceLittleEndianConversionArrayIntN)    (void*, size_t, size_t);
CCE_PUBLIC_OPTIONS extern void* (*cceLittleEndianConversionNewArrayIntN) (void*, const void*, size_t, size_t);
CCE_PUBLIC_OPTIONS extern uint16_t (*cceBigEndianConversionInt16) (uint16_t);
CCE_PUBLIC_OPTIONS extern uint32_t (*cceBigEndianConversionInt32) (uint32_t);
CCE_PUBLIC_OPTIONS extern uint64_t (*cceBigEndianConversionInt64) (uint64_t);
CCE_PUBLIC_OPTIONS extern void* (*cceBigEndianConversionArrayIntN)    (void*, size_t, size_t);
CCE_PUBLIC_OPTIONS extern void* (*cceBigEndianConversionNewArrayIntN) (void*, const void*, size_t, size_t);

#define cceBigEndianToHostEndianInt16(value) cceBigEndianConversionInt16(value)
#define cceHostEndianToBigEndianInt16(value) cceBigEndianConversionInt16(value)
#define cceBigEndianToHostEndianInt32(value) cceBigEndianConversionInt32(value)
#define cceHostEndianToBigEndianInt32(value) cceBigEndianConversionInt32(value)
#define cceBigEndianToHostEndianInt64(value) cceBigEndianConversionInt64(value)
#define cceHostEndianToBigEndianInt64(value) cceBigEndianConversionInt64(value)

#define cceBigEndianToHostEndianArrayInt16(array, size) (uint16_t*) cceBigEndianConversionArrayIntN(array, size, 2)
#define cceHostEndianToBigEndianArrayInt16(array, size) (uint16_t*) cceBigEndianConversionArrayIntN(array, size, 2)
#define cceBigEndianToHostEndianArrayInt32(array, size) (uint32_t*) cceBigEndianConversionArrayIntN(array, size, 4)
#define cceHostEndianToBigEndianArrayInt32(array, size) (uint32_t*) cceBigEndianConversionArrayIntN(array, size, 4)
#define cceBigEndianToHostEndianArrayInt64(array, size) (uint64_t*) cceBigEndianConversionArrayIntN(array, size, 8)
#define cceHostEndianToBigEndianArrayInt64(array, size) (uint64_t*) cceBigEndianConversionArrayIntN(array, size, 8)
#define cceBigEndianToHostEndianArrayIntN(array, size, n) cceBigEndianConversionArrayIntN(array, size, n)
#define cceHostEndianToBigEndianArrayIntN(array, size, n) cceBigEndianConversionArrayIntN(array, size, n)

#define cceBigEndianToHostEndianNewArrayInt16(dest, src, size) (uint16_t*) cceBigEndianConversionNewArrayIntN(dest, src, size, 2)
#define cceHostEndianToBigEndianNewArrayInt16(dest, src, size) (uint16_t*) cceBigEndianConversionNewArrayIntN(dest, src, size, 2)
#define cceBigEndianToHostEndianNewArrayInt32(dest, src, size) (uint32_t*) cceBigEndianConversionNewArrayIntN(dest, src, size, 4)
#define cceHostEndianToBigEndianNewArrayInt32(dest, src, size) (uint32_t*) cceBigEndianConversionNewArrayIntN(dest, src, size, 4)
#define cceBigEndianToHostEndianNewArrayInt64(dest, src, size) (uint64_t*) cceBigEndianConversionNewArrayIntN(dest, src, size, 8)
#define cceHostEndianToBigEndianNewArrayInt64(dest, src, size) (uint64_t*) cceBigEndianConversionNewArrayIntN(dest, src, size, 8)
#define cceBigEndianToHostEndianNewArrayIntN(dest, src, size, n) cceBigEndianConversionNewArrayIntN(dest, src, size, n)
#define cceHostEndianToBigEndianNewArrayIntN(dest, src, size, n) cceBigEndianConversionNewArrayIntN(dest, src, size, n)

#define cceLittleEndianToHostEndianInt16(value) cceLittleEndianConversionInt16(value)
#define cceHostEndianToLittleEndianInt16(value) cceLittleEndianConversionInt16(value)
#define cceLittleEndianToHostEndianInt32(value) cceLittleEndianConversionInt32(value)
#define cceHostEndianToLittleEndianInt32(value) cceLittleEndianConversionInt32(value)
#define cceLittleEndianToHostEndianInt64(value) cceLittleEndianConversionInt64(value)
#define cceHostEndianToLittleEndianInt64(value) cceLittleEndianConversionInt64(value)

#define cceHostEndianToLittleEndianArrayInt16(array, size) (uint16_t*) cceLittleEndianConversionArrayIntN(array, size, 2)
#define cceLittleEndianToHostEndianArrayInt16(array, size) (uint16_t*) cceLittleEndianConversionArrayIntN(array, size, 2)
#define cceHostEndianToLittleEndianArrayInt32(array, size) (uint32_t*) cceLittleEndianConversionArrayIntN(array, size, 4)
#define cceLittleEndianToHostEndianArrayInt32(array, size) (uint32_t*) cceLittleEndianConversionArrayIntN(array, size, 4)
#define cceHostEndianToLittleEndianArrayInt64(array, size) (uint64_t*) cceLittleEndianConversionArrayIntN(array, size, 8)
#define cceLittleEndianToHostEndianArrayInt64(array, size) (uint64_t*) cceLittleEndianConversionArrayIntN(array, size, 8)
#define cceHostEndianToLittleEndianArrayIntN(array, size, n) cceLittleEndianConversionArrayIntN(array, size, n)
#define cceLittleEndianToHostEndianArrayIntN(array, size, n) cceLittleEndianConversionArrayIntN(array, size, n)

#define cceHostEndianToLittleEndianNewArrayInt16(dest, src, size) (uint16_t*) cceLittleEndianConversionNewArrayIntN(dest, src, size, 2)
#define cceLittleEndianToHostEndianNewArrayInt16(dest, src, size) (uint16_t*) cceLittleEndianConversionNewArrayIntN(dest, src, size, 2)
#define cceHostEndianToLittleEndianNewArrayInt32(dest, src, size) (uint32_t*) cceLittleEndianConversionNewArrayIntN(dest, src, size, 4)
#define cceLittleEndianToHostEndianNewArrayInt32(dest, src, size) (uint32_t*) cceLittleEndianConversionNewArrayIntN(dest, src, size, 4)
#define cceHostEndianToLittleEndianNewArrayInt64(dest, src, size) (uint64_t*) cceLittleEndianConversionNewArrayIntN(dest, src, size, 8)
#define cceLittleEndianToHostEndianNewArrayInt64(dest, src, size) (uint64_t*) cceLittleEndianConversionNewArrayIntN(dest, src, size, 8)
#define cceHostEndianToLittleEndianNewArrayIntN(dest, src, size, n) cceLittleEndianConversionNewArrayIntN(dest, src, size, n)
#define cceLittleEndianToHostEndianNewArrayIntN(dest, src, size, n) cceLittleEndianConversionNewArrayIntN(dest, src, size, n)

CCE_PUBLIC_OPTIONS uint16_t cceSwapEndianInt16 (uint16_t value);
CCE_PUBLIC_OPTIONS uint32_t cceSwapEndianInt32 (uint32_t value);
CCE_PUBLIC_OPTIONS uint64_t cceSwapEndianInt64 (uint64_t value);
CCE_PUBLIC_OPTIONS void* cceSwapEndianArrayIntN (void *array, size_t size, size_t n);
CCE_PUBLIC_OPTIONS void* cceSwapEndianNewArrayIntN (void *newArray, const void *array, size_t size, size_t n);

#define cceBigEndianToLittleEndianInt16(value) cceSwapEndianInt16(value)
#define cceLittleEndianToBigEndianInt16(value) cceSwapEndianInt16(value)
#define cceBigEndianToLittleEndianInt32(value) cceSwapEndianInt32(value)
#define cceLittleEndianToBigEndianInt32(value) cceSwapEndianInt32(value)
#define cceBigEndianToLittleEndianInt64(value) cceSwapEndianInt64(value)
#define cceLittleEndianToBigEndianInt64(value) cceSwapEndianInt64(value)

#define cceBigEndianToLittleEndianArrayInt16(array, size) (uint16_t*) cceSwapEndianArrayIntN(array, size, 2)
#define cceLittleEndianToBigEndianArrayInt16(array, size) (uint16_t*) cceSwapEndianArrayIntN(array, size, 2)
#define cceBigEndianToLittleEndianArrayInt32(array, size) (uint32_t*) cceSwapEndianArrayIntN(array, size, 4)
#define cceLittleEndianToBigEndianArrayInt32(array, size) (uint32_t*) cceSwapEndianArrayIntN(array, size, 4)
#define cceBigEndianToLittleEndianArrayInt64(array, size) (uint64_t*) cceSwapEndianArrayIntN(array, size, 8)
#define cceLittleEndianToBigEndianArrayInt64(array, size) (uint64_t*) cceSwapEndianArrayIntN(array, size, 8)
#define cceBigEndianToLittleEndianArrayIntN(array, size, n) cceSwapEndianArrayIntN(array, size, n)
#define cceLittleEndianToBigEndianArrayIntN(array, size, n) cceSwapEndianArrayIntN(array, size, n)

#define cceBigEndianToLittleEndianNewArrayInt16(dest, src, size) (uint16_t*) cceSwapEndianNewArrayIntN(dest, src, size, 2)
#define cceLittleEndianToBigEndianNewArrayInt16(dest, src, size) (uint16_t*) cceSwapEndianNewArrayIntN(dest, src, size, 2)
#define cceBigEndianToLittleEndianNewArrayInt32(dest, src, size) (uint32_t*) cceSwapEndianNewArrayIntN(dest, src, size, 4)
#define cceLittleEndianToBigEndianNewArrayInt32(dest, src, size) (uint32_t*) cceSwapEndianNewArrayIntN(dest, src, size, 4)
#define cceBigEndianToLittleEndianNewArrayInt64(dest, src, size) (uint64_t*) cceSwapEndianNewArrayIntN(dest, src, size, 8)
#define cceLittleEndianToBigEndianNewArrayInt64(dest, src, size) (uint64_t*) cceSwapEndianNewArrayIntN(dest, src, size, 8)
#define cceBigEndianToLittleEndianNewArrayIntN(dest, src, size, n) cceSwapEndianNewArrayIntN(dest, src, size, n)
#define cceLittleEndianToBigEndianNewArrayIntN(dest, src, size, n) cceSwapEndianNewArrayIntN(dest, src, size, n)

#endif // ENDIANESS_H
