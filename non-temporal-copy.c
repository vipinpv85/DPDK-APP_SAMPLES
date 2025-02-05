#include <immintrin.h>
#include <string.h>

static inline void non_temporal_avx512_memcpy(void *dst, const void *src, unsigned int size)
{
        const unsigned int temp_simd_size = (size / (64 * 4)) * 64;
        const unsigned int temp_remn_size = size - temp_simd_size;
        int temp_size = size;

        unsigned int index = 0;
        while (temp_size >= (64 * 4)) {
                __m512i datareg[4];
                const __m512i* m_src_address = (const __m512i*) src + 4 * index;
                __m512i* m_dst_address = (__m512i*) dst + 4 * index;

                datareg[0] = _mm512_stream_load_si512(m_src_address + 0 );
                datareg[1] = _mm512_stream_load_si512(m_src_address + 1);
                datareg[2] = _mm512_stream_load_si512(m_src_address + 2);
                datareg[3] = _mm512_stream_load_si512(m_src_address + 3);

                _mm512_store_si512(m_dst_address + 0, datareg[0]);
                _mm512_store_si512(m_dst_address + 1, datareg[1]);
                _mm512_store_si512(m_dst_address + 2, datareg[2]);
                _mm512_store_si512(m_dst_address + 3, datareg[3]);

                index += 1;
                temp_size -= 64 * 4;
        }

        memcpy((void *)((char *)dst+temp_simd_size), (const void *)((const char *)src+temp_simd_size), temp_remn_size);
}
