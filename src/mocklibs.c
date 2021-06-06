#ifndef WITH_ZLIB
#include <zlib.h>
ZEXTERN int ZEXPORT
uncompress OF((Bytef *dest,   uLongf *destLen,
               const Bytef *source, uLong sourceLen))
{
  return Z_DATA_ERROR;
}
#endif

#ifndef WITH_LZ4
#include <lz4.h>
LZ4LIB_API int
LZ4_decompress_safe (const char* src, char* dst, int compressedSize, int dstCapacity)
{
  return -1;
}
#endif

#ifndef WITH_LZMA
#include <lzma.h>
LZMA_API(lzma_ret)
lzma_stream_buffer_decode (uint64_t *memlimit, uint32_t flags,
                           const lzma_allocator *allocator,
                           const uint8_t *in, size_t *in_pos, size_t in_size,
                           uint8_t *out, size_t *out_pos, size_t out_size) // lzma_nothrow lzma_attr_warn_unused_result;
{
  return LZMA_FORMAT_ERROR;
}
#endif

#ifndef WITH_LZO
#include <lzo/lzo1x.h>
LZO_EXTERN(int)
lzo1x_decompress_safe (const lzo_bytep src, lzo_uint  src_len,
                       lzo_bytep dst, lzo_uintp dst_len,
                       lzo_voidp wrkmem /* NOT USED */ )
{
  return LZO_E_NOT_YET_IMPLEMENTED;
}
#endif

#ifndef WITH_ZSTD
#include <zstd.h>
ZSTDLIB_API size_t
ZSTD_decompress (void* dst, size_t dstCapacity,
                 const void* src, size_t compressedSize)
{
  return ZSTD_CONTENTSIZE_ERROR;
}
ZSTDLIB_API unsigned
ZSTD_isError (size_t code)
{
  return 1;
}
#endif
