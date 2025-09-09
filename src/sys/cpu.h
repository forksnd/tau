#if defined(_WIN64) || defined(_WIN32)
  #define SYS_WIN
#elif defined(__CYGWIN__) && !defined(_WIN32)
  #define SYS_WIN_CYG
#elif defined(__ANDROID__)
  #define SYS_ANDROID
#elif defined(__linux__)
  #define SYS_LINUX
#elif defined(__unix__) || !defined(__APPLE__) && defined(__MACH__)
  #include <sys/param.h>
  #if defined(BSD)
    #define SYS_BSD
  #endif
#elif defined(__APPLE__) && defined(__MACH__) // Apple OSX and iOS (Darwin)
  #include <TargetConditionals.h>
  #if TARGET_IPHONE_SIMULATOR == 1
    #define SYS_IOS
  #elif TARGET_OS_IPHONE == 1
    #define SYS_IOS
  #elif TARGET_OS_MAC == 1
    #define SYS_MAC
  #endif
#elif defined(__sun) && defined(__SVR4)
    #define SYS_SOLARIS
#else
  #error Unkown operating system
#endif

/* compiler specific intrinisics */
#ifdef _MSC_VER

#define alignto(x) __declspec(align(x))
#define cpu_bit_cnt(u) __popcnt(u)
#define cpu_bit_cnt64(u) __popcnt64(u)

static inline int
cpu_bit_ffs32(unsigned u) {
  unsigned long idx = 0;
  unsigned char ret = _BitScanForward(&idx, u);
  if (ret == 0) {
    return 32;
  }
  return casti(idx);
}
static inline int
cpu_bit_ffs64(unsigned long long u) {
  unsigned long idx = 0;
  unsigned char ret = _BitScanForward64(&idx, u);
  if (ret == 0) {
    return 64;
  }
  return casti(idx);

}
#define lfence() _ReadBarrier()
#define sfence() _WriteBarrier()

#define atom_cmp_xchg(val, new_val, exp) _InterlockedCompareExchange64((long long volatile *)val, new_val, exp)
#define atom_xchg(val, new_val) _InterlockedExchange64((long long volatile *)val, new_val)
#define atom_add(val, add) _InterlockedExchangeAdd64((long long volatile *)val, add)
#define atom_sub(val, add) _InterlockedExchangeSub64((long long volatile *)val, add)

#define force_inline __forceinline
#define assume(x) __assume(x)
#define likely(x)
#define unlikely(x)
#define not_null

#define no_sanitize_int
#define no_sanitize_addr
#define no_sanitize_undef
#define no_sanitize_leak

#else

#define alignto(x)        __attribute__((aligned(x)))
#define cpu_bit_cnt(u)    __builtin_popcount(u)
#define cpu_bit_cnt64(u)  __builtin_popcountll(u)
#define cpu_bit_ffs32(u)  __builtin_ctz(u)
#define cpu_bit_ffs64(u)  __builtin_ctzll(u)

#define lfence() asm volatile("" ::: "memory")
#define sfence() asm volatile("" ::: "memory")

#define atom_cmp_xchg(val, new_val, exp) __sync_val_compare_and_swap(val, exp, new_val)
#define atom_xchg(val, new_val) __sync_lock_test_and_set(val, new_val)
#define atom_add(val, add) __sync_fetch_and_add(val, add)
#define atom_sub(val, sub) __sync_fetch_and_sub(val, sub)

#define force_inline      __attribute__((always_inline))
#define purist            __attribute__((pure))

#define assume(x)         __attribute__((__assume__(x)))
#define likely(x)         __builtin_expect(!!(x), 1)
#define unlikely(x)       __builtin_expect(!!(x), 0)
#define not_null          __nonnull

#define no_sanitize_int   __attribute__((no_sanitize("integer")))
#define no_sanitize_addr  __attribute__((no_sanitize("address")))
#define no_sanitize_undef __attribute__((no_sanitize("undefined")))
#define no_sanitize_leak  __attribute__((no_sanitize("leak")))

#endif

#define requires(x) assert(x)
#define ensures(x) assert(x)
#define maybe(x)
#define loop_invariant(x) assert(x)

#define sse_align alignto(16)
#define avx_align alignto(32)

/* os specific definitions */
#if defined(SYS_WIN) || defined(SYS_WIN_CYG)

#define COL_R 0u
#define COL_G 8u
#define COL_B 16u
#define COL_A 24u

#define col_rgb_hex(c) \
  (((((c) >> 16u) & 0xff) << COL_R) |\
  (((((c) >>  8u) & 0xff) << COL_G)) |\
  (((((c) >>  0u) & 0xff) << COL_B)) | (0xffu << COL_A))

#define col_rgba_hex(c) \
  (((((c) >> 16u) & 0xff) << COL_R) |\
  ((((c) >>  8u) & 0xff) << COL_G) |\
  ((((c) >>  0u) & 0xff) << COL_B) |\
  ((((c) >> 24u) & 0xff) << COL_A))

#elif defined(SYS_MAC) || defined(SYS_IOS)

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define COL_R 0u
#define COL_G 8u
#define COL_B 16u
#define COL_A 24u

#define col_rgb_hex(c) \
  (((((c) >> 16u) & 0xff) << COL_R) |\
   ((((c) >>  8u) & 0xff) << COL_G) |\
   ((((c) >>  0u) & 0xff) << COL_B) | (0xffu << COL_A))

#define col_rgba_hex(c) \
  (((((c) >> 16u) & 0xff) << COL_R) |\
  ((((c) >>  8u) & 0xff) << COL_G) |\
  ((((c) >>  0u) & 0xff) << COL_B) |\
  ((((c) >> 24u) & 0xff) << COL_A))

#elif defined(__EMSCRIPTEN__)

#define COL_R 0
#define COL_G 8
#define COL_B 16
#define COL_A 24

#define col_rgb_hex(c) \
  (((((c) >> 16u) & 0xff) << COL_R) |\
   ((((c) >>  8u) & 0xff) << COL_G) |\
   ((((c) >>  0u) & 0xff) << COL_B) | (0xffu << COL_A))

#define col_rgba_hex(c) \
  (((((c) >> 16u) & 0xff) << COL_R) |\
  ((((c) >>  8u) & 0xff) << COL_G) |\
  ((((c) >>  0u) & 0xff) << COL_B) |\
  ((((c) >> 24u) & 0xff) << COL_A))

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define COL_R 16
#define COL_G 8
#define COL_B 0
#define COL_A 24

#define col_rgb_hex(c) ((c) | (0xffu << COL_A))
#define col_rgba_hex(c) (c)

#endif

static const unsigned char sse_align aes_seed[16] = {
  178, 201, 95, 240, 40, 41, 143, 216,
  2, 209, 178, 114, 232, 4, 176, 188
};

enum cpu_vendor {
  CPU_UNKNOWN,
  CPU_INTEL,
  CPU_AMD,
  CPU_APPLE,
  CPU_SAMSUNG,
  CPU_EMSCRIPT
};
struct cpu_info {
  enum cpu_vendor vendor;
  unsigned has_simd_128;
  unsigned has_simd_256;

#ifdef __x86_64__
  unsigned has_fpu:1;
  unsigned has_mmx:1;
  unsigned has_sse:1;
  unsigned has_sse2:1;
  unsigned has_sse3:1;
  unsigned has_ssse3:1;
  unsigned has_sse41:1;
  unsigned has_sse42:1;
  unsigned has_aes:1;
  unsigned has_sha:1;
  unsigned has_avx:1;
  unsigned has_avx2:1;
  unsigned has_avx512_f:1;
  unsigned has_avx512_dq:1;
  unsigned has_avx512_ifma:1;
  unsigned has_avx512_pf:1;
  unsigned has_avx512_er:1;
  unsigned has_avx512_cd:1;
  unsigned has_avx512_bw:1;
  unsigned has_avx512_vl:1;
  unsigned has_avx512_vbmi:1;
  unsigned has_avx512_vbmi2:1;
  unsigned has_avx512_vnni:1;
  unsigned has_avx512_bitalg:1;
  unsigned has_avx512_vpopcntdq:1;
  unsigned has_avx512_4vnniw:1;
  unsigned has_avx512_4fmaps:1;
  unsigned has_avx512_vp2intersect:1;
#else

#endif
};

/* x86/x64 */
#ifdef __x86_64__

#define SYS_X86_64 1

#include <emmintrin.h>
#include <smmintrin.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <wmmintrin.h>

/* simd 128-bit */
#ifdef USE_SIMD_128
#define CPU_SIMD_128
#endif

/* misc */
#define yield() _mm_pause()

static force_inline void
cpu__id(unsigned *eax, unsigned *ebx, unsigned *ecx, unsigned *edx) {
  __asm__ volatile("cpuid" : "=a" (*eax), "=b" (*ebx), "=c" (*ecx),
      "=d" (*edx) : "0" (*eax), "2" (*ecx));
}
static void
cpu_info(struct cpu_info *cpu) {
  unsigned eax, ebx, ecx, edx;

  /* vendor  */
  eax = 0;
  ecx = 0;
  cpu__id(&eax, &ebx, &ecx, &edx);
  char vendor[13] = {0};
  memcpy(vendor + 0, &ebx, sizeof(ebx));
  memcpy(vendor + 4, &edx, sizeof(edx));
  memcpy(vendor + 8, &ecx, sizeof(ecx));
  if (!strcmp(vendor, "GenuineIntel")) {
    cpu->vendor = CPU_INTEL;
  } else if(!strcmp(vendor, "AuthenticAMD")) {
    cpu->vendor = CPU_AMD;
  } else {
    cpu->vendor = CPU_UNKNOWN;
  }
  /* check features */
  eax = 1;
  ecx = 0;
  cpu__id(&eax, &ebx, &ecx, &edx);
  cpu->has_fpu = (edx & (1u << 0)) != 0;
  cpu->has_mmx = (edx & (1u << 23)) != 0;
  cpu->has_sse = (edx & (1u << 25)) != 0;
  cpu->has_sse2 = (ecx & (1u << 26)) != 0;
  cpu->has_sse3 = (ecx & (1u << 0)) != 0;
  cpu->has_ssse3 = (ecx & (1u << 9)) != 0;
  cpu->has_sse41 = (ecx & (1u << 19)) != 0;
  cpu->has_sse42 = (ecx & (1u << 20)) != 0;
  cpu->has_aes = (ecx & (1u << 25)) != 0;
  cpu->has_avx = (ecx & (1u << 28)) != 0;
  cpu->has_simd_128 = cpu->has_sse42 && cpu->has_sse41 && cpu->has_sse2 && cpu->has_sse;
  cpu->has_simd_256 = cpu->has_avx;

  /* check extended features */
  eax = 7;
  ecx = 0;
  cpu__id(&eax, &ebx, &ecx, &edx);
  cpu->has_avx2 = (ebx & (1u << 5)) != 0;
  cpu->has_sha = (ebx & (1u << 29)) != 0;
  cpu->has_avx512_f = (ebx & (1u << 16)) != 0;
  cpu->has_avx512_dq = (ebx & (1u << 17)) != 0;
  cpu->has_avx512_ifma = (ebx & (1u << 21)) != 0;
  cpu->has_avx512_pf = (ebx & (1u << 26)) != 0;
  cpu->has_avx512_er = (ebx & (1u << 27)) != 0;
  cpu->has_avx512_cd = (ebx & (1u << 28)) != 0;
  cpu->has_avx512_bw = (ebx & (1u << 30)) != 0;
  cpu->has_avx512_vl = (ebx & (1u << 31)) != 0;
  cpu->has_avx512_vbmi = (ecx & (1u << 1)) != 0;
  cpu->has_avx512_vbmi2 = (ecx & (1u << 6)) != 0;
  cpu->has_avx512_vnni = (ecx & (1u << 11)) != 0;
  cpu->has_avx512_bitalg = (ecx & (1u << 12)) != 0;
  cpu->has_avx512_vpopcntdq = (ecx & (1u << 14)) != 0;
  cpu->has_avx512_4vnniw = (edx & (1u << 2)) != 0;
  cpu->has_avx512_4fmaps = (edx & (1u << 3)) != 0;
  cpu->has_avx512_vp2intersect = (edx & (1u << 8)) != 0;
}
static force_inline float
cpu_fma(float x, float y, float z) {
  __asm__ ("vfmaddss %3, %2, %1, %0" : "=x" (x) : "x" (x), "x" (y), "x" (z));
  return x;
}
static force_inline float
cpu_rintf(float x) {
  __asm__ ("frndint" : "+t"(x));
  return x;
}

#define flt4                __m128
#define flt4_flt(a)         _mm_set_ps1(a)
#define flt4_str(d,r)       _mm_storeu_ps(((float*)(d)),r)
#define flt4_max(a,b)       _mm_max_ps(a,b)
#define flt4_mul(a,b)       _mm_mul_ps(a,b)
#define flt4_add(a,b)       _mm_add_ps(a,b)
#define flt4_cmp_gt(a,b)    _mm_castps_si128(_mm_cmpgt_ps(a,b))
#define flt4_and(a,b)       _mm_and_ps(a,b)
#define flt4_xor(a,b)       _mm_xor_ps(a,b)
#define flt4_zip_lo32(a,b)  _mm_unpacklo_ps(a,b)
#define flt4_zip_hi32(a,b)  _mm_unpackhi_ps(a,b)
#define flt4_zip_lo64(a,b)  _mm_castpd_ps(_mm_unpacklo_pd(_mm_castps_pd(a),_mm_castps_pd(b)))
#define flt4_zip_hi64(a,b)  _mm_castpd_ps(_mm_unpackhi_pd(_mm_castps_pd(a),_mm_castps_pd(b)))
#define flt4_int4(a)        _mm_castsi128_ps(a)
#define flt4_cmpu(a,b)      _mm_cmpunord_ps(a,b)
#define flt4_rsqrt(a)       _mm_rsqrt_ps(a)
#define flt4_sqrt(a)        _mm_sqrt_ps(a)
#define flt4_strs(d,a)      _mm_store_ss(d,a)

static inline float
cpu_rsqrt(float n) {
  flt4_strs(&n, flt4_rsqrt(flt4_flt(n)));
  return n;
}
static inline float
cpu_sqrt(float n) {
  flt4_strs(&n, flt4_sqrt(flt4_flt(n)));
  return n;
}

#define int4                __m128i
#define int4_ld(p)          _mm_loadu_si128((const __m128i*)(const void*)p)
#define int4_set(x,y,z,w)   _mm_setr_epi32(x,y,z,w)
#define int4_str(p,i)       _mm_storeu_si128((__m128i*)(void*)p, i)
#define int4_char(i)        _mm_set1_epi8(i)
#define int4_int(i)         _mm_set1_epi32(i)
#define int4_uint(i)        _mm_set1_epi32(i)
#define int4_sll(v,i)       _mm_slli_epi32(v,i)
#define int4_srl(v,i)       _mm_srli_epi32(v,i)
#define int4_sra(v,i)       _mm_srai_epi32(v,i)
#define int4_and(a,b)       _mm_and_si128(a, b)
#define int4_andnot(a,b)    _mm_andnot_si128(a, b)
#define int4_or(a,b)        _mm_or_si128(a, b)
#define int4_add(a,b)       _mm_add_epi32(a, b)
#define int4_sub(a,b)       _mm_sub_epi32(a, b)
#define int4_mul(a,b)       _mm_mullo_epi32(a, b)
#define int4_blend(a,b,m)   _mm_blendv_epi8(a,b,m)
#define int4_flt4(a)        _mm_castps_si128(a)
#define int4_cmp_gt(a,b)    _mm_cmpgt_epi32(a,b)

/* simd 256-bit */
#ifdef USE_SIMD_256
#define CPU_SIMD_256
#endif

#define flt8                __m256
#define int8                __m256i
#define int8_ld(p)          _mm256_loadu_si256((const __m256i*)(const void*)p)
#define int8_store(p,i)     _mm256_storeu_si256((__m256i*)(void*)p, i)
#define int8_char(i)        _mm256_set1_epi8(i)
#define int8_int(i)         _mm256_set1_epi32(i)
#define int8_sll(v,i)       _mm256_slli_epi32(v,i)
#define int8_srl(v,i)       _mm256_srli_epi32(v,i)
#define int8_and(a,b)       _mm256_and_si256(a, b)
#define int8_andnot(a,b)    _mm256_andnot_si256(a, b)
#define int8_or(a,b)        _mm256_or_si256(a, b)
#define int8_add(a,b)       _mm256_add_epi32(a, b)
#define int8_sub(a,b)       _mm256_sub_epi32(a, b)
#define int8_mul(a,b)       _mm256_mullo_epi32(a, b)

/* string */
#ifdef CPU_SIMD_256

static inline no_sanitize_addr const char*
cpu_str_chr(const char *s, int n, int chr) {
  static const char unsigned ovr_msk[64] = {
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  };
  const char *e = s + n;
  __m256i m = _mm256_set1_epi8(chr & 0xff);
  for (; s < e; s += 32) {
    int r = (int)(e - s); r = r > 32 ? 32 : r;
    __m256i o = _mm256_loadu_si256((const __m256i*)(ovr_msk + 32 - r));
    __m256i d = _mm256_loadu_si256((const __m256i*)(const void*)s);
    __m256i v = _mm256_and_si128(d, o);
    unsigned msk = _mm256_movemask_epi8(_mm256_cmpeq_epi8(v,m));
    if (msk) {
      return s + cpu_bit_ffs32(msk);
    }
  }
  return e;
}
static inline no_sanitize_addr int
cpu_str_fnd(const char *s, int n, const char *needle, int k) {
  const __m256i first = _mm256_set1_epi8(needle[0]);
  const __m256i last  = _mm256_set1_epi8(needle[k-1]);
  for (size_t i = 0; i < n; i += 32) {
    const __m256i block_first = _mm256_loadu_si256((const __m256i*)(s + i));
    const __m256i block_last  = _mm256_loadu_si256((const __m256i*)(s + i + k - 1));
    const __m256i eq_first = _mm256_cmpeq_epi8(first, block_first);
    const __m256i eq_last  = _mm256_cmpeq_epi8(last, block_last);
    unsigned mask = _mm256_movemask_epi8(_mm256_and_si256(eq_first, eq_last));
    while (mask != 0) {
      int bitpos = __builtin_ctz(mask);
      if (memcmp(s + i + bitpos + 1, needle + 1, k - 2) == 0) {
          return i + bitpos;
      }
      mask = mask & (mask - 1);
    }
  }
  return n;
}
#else

static inline const char*
cpu_str_chr(const char *str, int cnt, int chr) {
  static const char unsigned ovr_msk[32] = {
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  };
  const char *end = str + cnt;
  __m128i ndl = _mm_set1_epi8((char)(chr & 0xff));
  for (; str < end; str += 16) {
    int rst = (int)(end - str);
    rst = rst > 16 ? 16 : rst;

    __m128i o = _mm_loadu_si128((const __m128i *)(const void*)(ovr_msk + 16 - rst));
    __m128i d = _mm_loadu_si128((const __m128i *)(const void*)(const void*)str);
    __m128i v = _mm_and_si128(d, o);
    unsigned msk = (unsigned)_mm_movemask_epi8(_mm_cmpeq_epi8(v,ndl));
    if (msk) {
      return str + cpu_bit_ffs32(msk);
    }
  }
  return end;
}
static inline int
cpu_str_fnd(const char *str, int cnt, const char *ndl, size_t len) {
  const __m128i first = _mm_set1_epi8(ndl[0]);
  const __m128i last  = _mm_set1_epi8(ndl[len-1]);
  for (int i = 0; i < cnt; i += 16) {
    const __m128i blk_beg = _mm_loadu_si128((const __m128i*)(const void*)(str + i));
    const __m128i blk_end = _mm_loadu_si128((const __m128i*)(const void*)(str + i + len - 1));
    const __m128i eql_beg = _mm_cmpeq_epi8(first, blk_beg);
    const __m128i eql_end = _mm_cmpeq_epi8(last, blk_end);
    unsigned msk = (unsigned)_mm_movemask_epi8(_mm_and_si128(eql_beg, eql_end));
    while (msk != 0) {
      unsigned bitpos = (unsigned)__builtin_ctz(msk);
      if (memcmp(str + i + bitpos, ndl + 1, (size_t)(len - 1)) == 0) {
        return i + (int)bitpos;
      }
      msk = msk & (msk - 1);
    }
  }
  return cnt;
}
#endif

#elif defined(__arm__) || defined(__aarch64__)

#include <arm_neon.h>

#define SYS_ARM 1

/* simd 128-bit */
#ifdef USE_SIMD_128
#define CPU_SIMD_128
#endif

/* misc */
#define yield() __asm__ __volatile__("isb\n")

static void
cpu_info(struct cpu_info *cpu) {
#if defined(SYS_IOS) || defined(SYS_MAC)
  cpu->vendor = CPU_APPLE;
#elif defined(SYS_ANDROID)
  cpu->vendor = CPU_SAMSUNG;
#endif
  cpu->has_simd_128 = 1;
  cpu->has_simd_256 = 1;
}
static inline float
cpu_fma(float x, float y, float z) {
  __asm__("fmadd %s0, %s1, %s2, %s3" : "=w"(x) : "w"(x), "w"(y), "w"(z));
  return x;
}
static inline float
cpu_rintf(float x) {
  __asm__("frintx %s0, %s1" : "=w"(x) : "w"(x));
  return x;
}

/* float4 */
#define flt4                float32x4_t
#define flt4_flt(a)         vdupq_n_f32(a)
#define flt4_str(d,r)       vst1q_f32((float*)d, r)
#define flt4_max(a,b)       vmaxnmq_f32(a,b)
#define flt4_mul(a,b)       vmulq_f32(a,b)
#define flt4_add(a,b)       vaddq_f32(a,b)
#define flt4_cmp_gt(a,b)    vcgtq_f32(a,b)
#define flt4_and(a,b)       vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(a),vreinterpretq_u32_f32(b)))
#define flt4_xor(a,b)       vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(a),vreinterpretq_u32_f32(b)))
#define flt4_zip_lo32(a,b)  vzip1q_f32(a,b)
#define flt4_zip_hi32(a,b)  vzip2q_f32(a,b)
#define flt4_zip_lo64(a,b)  vreinterpretq_f32_f64(vzip1q_f64(vreinterpretq_f64_f32(a),vreinterpretq_f64_f32(b)))
#define flt4_zip_hi64(a,b)  vreinterpretq_f32_f64(vzip2q_f64(vreinterpretq_f64_f32(a),vreinterpretq_f64_f32(b)))
#define flt4_int4(a)        vreinterpretq_f32_s32(a)
#define flt4_rsqrt(a)       vrsqrteq_f32(a)
#define flt4_sqrt(a)        vsqrtq_f32(a)
#define flt4_strs(d,a)      vst1q_lane_f32(d,a,0)

static inline flt4
flt4_cmpu(flt4 a, flt4 b) {
  uint32x4_t f32a = vceqq_f32(a, a);
  uint32x4_t f32b = vceqq_f32(b, b);
  return vreinterpretq_f32_u32(vmvnq_u32(vandq_u32(f32a, f32b)));
}
static inline float
cpu_rsqrt(float n) {
  flt4_strs(&n, flt4_rsqrt(flt4_flt(n)));
  return n;
}
static inline float
cpu_sqrt(float n) {
  flt4_strs(&n, flt4_sqrt(flt4_flt(n)));
  return n;
}

#define int4                int32x4_t
#define int4_ld(p)          vld1q_s32((const int*)(const void*)p)
#define int4_str(p,i)       vst1q_s32((int*)(void*)p, i)
#define int4_char(i)        vreinterpretq_s32_s8(vdupq_n_s8(i))
#define int4_int(i)         vdupq_n_s32(i)
#define int4_uint(i)        vreinterpretq_s32_u32(vdupq_n_u32(i))
#define int4_and(a,b)       vandq_s32(a,b)
#define int4_andnot(a,b)    vbicq_s32(b,a)
#define int4_or(a,b)        vorrq_s32(a,b)
#define int4_add(a,b)       vaddq_s32(a,b)
#define int4_sub(a,b)       vsubq_s32(a,b)
#define int4_mul(a,b)       vmulq_s32(a,b)
#define int4_sll(a,i)       vshlq_s32(a, vdupq_n_s32(i))
#define int4_srl(a,i)       vshlq_s32(a, vdupq_n_s32(-i))
#define int4_sra(v,i)       vshlq_s32(v, vdupq_n_s32(-i))
#define int4_blend(a,b,msk) vbslq_s32(msk, b, a)
#define int4_flt4(a)        vreinterpretq_s32_f32(a)
#define int4_cmp_gt(a,b)    vreinterpretq_s32_u32(vcgtq_s32(a,b))

static inline int4
int4_set(int i3, int i2, int i1, int i0) {
  int sse_align v[4] = {i3, i2, i1, i0};
  return vld1q_s32(v);
}

/* string */
static inline int
chr16_tst_all_ones(uint8x16_t a) {
  unsigned long long lo = vgetq_lane_u64(vreinterpretq_u64_u8(a), 0);
  unsigned long long hi = vgetq_lane_u64(vreinterpretq_u64_u8(a), 1);
  return (lo&hi) == (unsigned long long)-1;
}
static inline int
chr16_tst_all_zero(uint8x16_t a) {
  unsigned long long lo = vgetq_lane_u64(vreinterpretq_u64_u8(a), 0);
  unsigned long long hi = vgetq_lane_u64(vreinterpretq_u64_u8(a), 1);
  return (lo|hi) == 0u;
}
static inline no_sanitize_addr const char*
cpu_str_chr(const char *s, int n, int chr) {
  static const char unsigned ovr_msk[32] = {
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  };
  if (!s) {
    return s;
  }
  const char *e = s + n;
  if (n < 16) {
    while (s < e) {
      if (*s == chr) {
        return s;
      }
      s++;
    }
    return e;
  }
  uint8x16_t m = vdupq_n_u8(chr & 0xff);
  for (; s < e; s += 16) {
    int r = (int)(e - s); r = r > 16 ? 16 : r;
    uint8x16_t o = vld1q_u8(ovr_msk + 16 - r);
    uint8x16_t d = vld1q_u8((const unsigned char*)s);
    uint8x16_t v = vandq_u8(d, o);
    uint8x16_t c = vceqq_u8(v, m);
    uint64x2_t p = vreinterpretq_u64_u8(c);
    uint64_t vlo = vgetq_lane_u64(p, 0);
    if (vlo) {
      return s + ((cpu_bit_ffs64(vlo)) >> 3);
    }
    uint64_t vhi = vgetq_lane_u64(p, 1);
    if (vhi) {
      return s + 8 + ((cpu_bit_ffs64(vhi)) >> 3);
    }
  }
  return e;
}
static inline no_sanitize_addr int
cpu_str_fnd(const char *s, size_t n, const char *needle, size_t k) {
  assert(k > 0);
  assert(n > 0);

  const uint8x16_t first = vdupq_n_u8((unsigned char)needle[0]);
  const uint8x16_t last  = vdupq_n_u8((unsigned char)needle[k - 1]);
  const unsigned char *ptr = (const unsigned char*)s;
  for (size_t i = 0; i < n; i += 16) {
    const uint8x16_t blk_first = vld1q_u8(ptr + i);
    const uint8x16_t blk_last  = vld1q_u8(ptr + i + k - 1);
    const uint8x16_t eq_first = vceqq_u8(first, blk_first);
    const uint8x16_t eq_last  = vceqq_u8(last, blk_last);
    const uint8x16_t pred_16  = vandq_u8(eq_first, eq_last);
    unsigned long long mask = vgetq_lane_u64(vreinterpretq_u64_u8(pred_16), 0);
    if (mask) {
      for (size_t j=0; j < 8; j++) {
        if ((mask & 0xff) && (memcmp(s + i + j + 1, needle + 1, k - 2) == 0)) {
          return (int)(i + j);
        }
        mask >>= 8;
      }
    }
    mask = vgetq_lane_u64(vreinterpretq_u64_u8(pred_16), 1);
    if (mask) {
      for (size_t j=0; j < 8; j++) {
        if ((mask & 0xff) && (memcmp(s + i + j + 8 + 1, needle + 1, k - 2) == 0)) {
            return (int)(i + j + 8);
        }
        mask >>= 8;
      }
    }
  }
  return (int)n;
}

#elif defined(__EMSCRIPTEN__)

#define SYS_EMSCRIPT 1

#include <wasm_simd128.h>

/* simd 128-bit */

/* misc */
#define yield()

static void
cpu_info(struct cpu_info *cpu) {
  cpu->vendor = CPU_EMSCRIPT;
  cpu->has_simd_128 = 0;
  cpu->has_simd_256 = 0;
}
static inline float
cpu_fma(float x, float y, float z) {
  return x * y + z;
}
static inline float
cpu_rintf(float x) {
  static const float toint = 1/FLT_EPSILON;
  union {float f; uint32_t i;} u = {x};
  int e = u.i>>23 & 0xff;
  int s = u.i>>31;
  float y;
  if (e >= 0x7f+23) {
    return x;
  }
  if (s) {
    y = x - toint + toint;
  } else {
    y = x + toint - toint;
  }
  if (y == 0) {
    return s ? -0.0f : 0.0f;
  }
  return y;
}

static inline float
cpu_rsqrt(float x) {
  /* Exact bits: 13.71 */
  /* δ+max = 7.459289×10^−5 */
  /* δ−max = −7.450387×10^−5 */
  union { float f; int i; } v = {x};
  int k = v.i & 0x00800000;
  float y = 0.0f;
  if (k != 0) {
    v.i = 0x5ed9e91f - (v.i >> 1);
    y = v.f;
    y = 2.33124256f*y*cpu_fma(-x,y*y, 1.0749737f);
  } else {
    v.i = 0x5f19e8fc - (v.i >> 1);
    y = v.f;
    y = 0.824218631f*y*cpu_fma(-x, y*y, 2.1499474f);
  }
  return y;
}
static inline float
cpu_sqrt(float x) {
  /* Exact bits: 13.71 */
  /* δ+max = 7.450372×10^−5 */
  /* δ−max = −7.451108×10^−5 */
  union { float f; int i; } v = {x};
  int k = v.i & 0x00800000;
  float y = 0.0f, c;
  if (k != 0) {
    v.i = 0x5ed9e893 - (v.i >> 1);
    y = v.f;
    c = x*y;
    y = 2.33130789f*c*cpu_fma(y, -c, 1.07495356f);
  } else {
    v.i = 0x5f19e8fd - (v.i >> 1);
    y = v.f;
    c = x*y;
    y = 0.82421863f*c*cpu_fma(y, -c, 2.1499474f);
  }
  return y;
}

/* string */
static const char*
cpu_str_chr(const char *str, int n, int chr) {
  const char *s = str;
  const char *e = str + n;

  int c = chr & 0xFF;
  unsigned long long m4 = (c << 24)|(c << 16)|(c << 8)|c;
  unsigned long long m = m4 << 32LLU | m4;
  for (;s < e;) {
    while ((((uintptr_t)s) & 7) && s < e) {
    chk1: if (s[0] == c) return s;
    chk2: if (s[0] == 0) return s;
      ++s;
    }
    for (;s < e; s += 8) {
      unsigned long long v = *(unsigned long long*)s;
      unsigned long long c = (~v) & 0x8080808080808080LLU;
      if (((v ^ m) - 0x0101010101010101) & c) goto chk1;
      if ((v - 0x0101010101010101) & c) goto chk2;
    }
  }
  return e;
}
static inline int
cpu_str_fnd(const char *hay, int hay_len, const char *needle, int needle_len) {
  #define CPU_STR_FND_LIMIT 8
  static const char unsigned ovr_msk[16] = {
    255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0
  };
  unsigned long long ndl = *((unsigned long long*)needle);
  unsigned long long ovr = *((unsigned long long*)(ovr_msk + 8 - (needle_len & 7)));
  for (int i = 0; i + needle_len <= hay_len; i++) {
    unsigned long long txt = *((unsigned long long*)(hay + i));
    if (((ndl ^ txt) & ovr) == 0) {
      return i;
    }
  }
  return hay_len;
}

#endif

