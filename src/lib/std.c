#if __has_include(<stdckdint.h>)
  #include <stdckdint.h>
#else
  #define ckd_add(R, A, B) __builtin_add_overflow ((A), (B), (R))
  #define ckd_sub(R, A, B) __builtin_sub_overflow ((A), (B), (R))
  #define ckd_mul(R, A, B) __builtin_mul_overflow ((A), (B), (R))
#endif

static purist inline int
chk_add(int a, int b, unsigned *ovf) {
  int ret = 0;
  *ovf |= ckd_add(&ret, a, b);
  return ret;
}
static purist inline int
chk_sub(int a, int b, unsigned *ovf) {
  int ret = 0;
  *ovf |= ckd_sub(&ret, a, b);
  return ret;
}
static purist inline int
chk_mul(int a, int b, unsigned *ovf) {
  int ret = 0;
  *ovf |= ckd_mul(&ret, a, b);
  return ret;
}
static int
ilog2(int n) {
  requires(n >= 0);
  if (!n) return 0;
  int ret = 0;
#ifdef _MSC_VER
  unsigned long msbp = 0;
  _BitScanReverse(&msbp, (unsigned long)n);
  ret = (int)msbp;
#else
  ret = (int)sizeof(unsigned long) * CHAR_BIT - 1 - __builtin_clzl((unsigned long)n);
#endif
  ensures(ret >= 0);
  ensures(ret <= 31);
  return ret;
}
static purist inline int
npow2(int input) {
  requires(input > 0);
  unsigned int val = castu(input);
  val--;
  val |= val >> 1;
  val |= val >> 2;
  val |= val >> 4;
  val |= val >> 8;
  val |= val >> 16;
  val++;

  int ret = casti(val);
  ensures(ret >= input);
  ensures(ret <= 2 * input);
  ensures(ret == 1 << casti(ilog2(input)));
  ensures(ispow2(ret));
  ensures((ret / 2) < input || ret == 1);
  ensures(ret <= 2 * input);
  return ret;
}
/* ---------------------------------------------------------------------------
 *                                Foreach
 * ---------------------------------------------------------------------------
 */
#define forever while(1)
#define loopr(i,r) (int i = (r).lo; i != (r).hi; i += 1)
#define looprn(i,r,n) (int i = (r).lo; i != min((r).lo + n, (r).hi); i += 1)
#define loopi(i,j,r) (int i = (r).lo, j = 0; i != (r).hi; i += 1, ++j)
#define loop(i,n) (int i = 0; i < (n); ++i)
#define loopn(i,n,m) (int i = 0; i < min(n,m); ++i)

/* ---------------------------------------------------------------------------
 *                                Memory
 * ---------------------------------------------------------------------------
 */
static inline void*
mcpy(void* restrict dst, void const *restrict src, int cnt) {
  unsigned char *restrict dst8 = dst;
  const unsigned char *restrict src8 = src;
  for loop(i, cnt) {
    dst8[i] = src8[i];
  }
  return dst;
}
static inline void*
mset(void *addr, int val, int cnt) {
  unsigned char *dst = addr;
  for loop(i, cnt) {
    dst[i] = castb(val);
  }
  return addr;
}
#define swap(x,y) do {\
  unsigned char uniqid(t)[szof(x) == szof(y) ? szof(x) : -1]; \
  mcpy(uniqid(t),&y,szof(x)); \
  mcpy(&y,&x,szof(x)); \
  mcpy(&x,uniqid(t),szof(x)); \
} while(0)

/* ---------------------------------------------------------------------------
 *                                Range
 * ---------------------------------------------------------------------------
 */
#define rng(b,e,n) rng__mk(rng__bnd(b,n), rng__bnd(e,n), n)
#define intvl(b,n) rng(b,n,n)
#define rngn(n) rng(0,(n),(n))
#define rng_inv (struct rng){-1,-1,-1,-1}
#define rng_nil (struct rng){0}

#define slc(b,e) rng((b),(e),(e)-(b))
#define slc_beg(p,r) ((p)+(r).lo)
#define slc_end(p,r) ((p)+(r).hi)
#define slc_at(p,r,i) (slc_beg(p,r)[i])
#define slc_ptr(p,r,i) ((p)+(r).lo+(i))

#define rng_has_incl(a,b) ((a)->lo <= (b)->lo && (a)->hi >= (b)->hi)
#define rng_has_inclv(a,v) ((v) >= (a)->lo && (v) <= (a)->hi)
#define rng_has_excl(a,b) ((a)->lo < (b)->lo && (a)->hi > (b)->hi)
#define rng_has_exclv(a,v) ((v) > (a)->lo && (v) < (a)->hi)
#define rng_overlaps(a,b) (max((a)->lo, (b)->lo) <= min((a)->hi, (b)->hi))
#define rng_is_inv(a) (a.cnt < 0 || a.total < 0)

#define rng_clamp(a,v) clamp((a)->lo, v, (a)->hi)
#define rng_cnt(r) ((r)->cnt)
#define rng_shft(r, d) (r)->lo += (d), (r)->hi += (d)
#define rng_norm(r,v) (castf(castd(rng_clamp(r,v) - (r)->lo) / castd(rng_cnt(r))))

#define rng_rhs(r,n) rng_sub(r,n,(r)->cnt)
#define rng_lhs(r,n) rng_sub(r,0,n)
#define rng_cut_lhs(r,n) *(r) = rng_rhs(s,n)
#define rng_cut_rhs(r,n) *(r) = rng_lhs(s,n)

static inline int
rng__is_val(const struct rng *rng) {
  assert(rng);
  assert(rng->lo <= rng->hi);
  assert(rng->cnt <= abs(rng->hi - rng->lo));
  assert(rng->cnt >= 0);
  assert(rng->total >= 0);
  return 1;
}
static purist force_inline int
rng__bnd(int idx, int cnt) {
  int lft = max(cnt, 1) - 1;
  int val = (idx < 0) ? (cnt - idx) : idx;
  return clamp(val, 0, lft);
}
static purist force_inline struct rng
rng__mk(int low, int high, int cnt) {
  requires(low <= high);
  struct rng ret = {.lo = low, .hi = high};
  ret.cnt = abs(ret.hi - ret.lo);
  ret.total = cnt;

  ensures(rng__is_val(&ret));
  return ret;
}
static purist force_inline struct rng
rng_sub(const struct rng *rng, int beg, int end) {
  requires(rng__is_val(rng));
  struct rng ret = rng(beg, end, rng->total);
  rng_shft(&ret, rng->lo);
  ensures(rng__is_val(&ret));
  return ret;
}
static purist force_inline struct rng
rng_put(const struct rng *rng, const struct rng *val) {
  requires(rng__is_val(rng));
  requires(val);

  struct rng ret = *val;
  rng_shft(&ret, rng->lo);
  ensures(rng__is_val(&ret));
  return ret;
}

/* ---------------------------------------------------------------------------
 *                                  Hash
 * ---------------------------------------------------------------------------
 */
#define FNV1A32_HASH_INITIAL 2166136261U
#define FNV1A64_HASH_INITIAL 14695981039346656037llu

static purist inline no_sanitize_int unsigned
fnv1a32(const void *ptr, int cnt, unsigned hash) {
  requires(ptr);
  requires(cnt >= 0);
  requires(hash >= 0);
  const unsigned char *ptr8 = ptr;
  if (!ptr || !cnt) {
    return FNV1A32_HASH_INITIAL;
  }
  for loop(i,cnt) {
    hash = (hash ^ ptr8[i]) * 16777619u;
  }
  return hash;
}
static purist inline no_sanitize_int unsigned long long
fnv1a64(const void *ptr, int len, unsigned long long hash) {
  requires(ptr);
  requires(len >= 0);
  requires(hash >= 0);

  const unsigned char *ptr8 = ptr;
  if (!ptr || !len) {
    return FNV1A64_HASH_INITIAL;
  }
  for loop(i,len) {
    hash ^= (unsigned long long)ptr8[i];
    hash *= 1099511628211LLU;
  }
  return hash;
}
static purist unsigned
fnv1au32(unsigned hash, unsigned uid) {
  return fnv1a32(&uid, sizeof(uid), hash);
}
static purist unsigned long long
fnv1au64(unsigned long long uid, unsigned long long hash) {
  return fnv1a64(&uid, sizeof(uid), hash);
}
static purist unsigned long long
hash_ptr(const void *ptr) {
  return fnv1a64(&ptr, szof(void*), FNV1A64_HASH_INITIAL);
}
static purist unsigned long long
hash_int(long long lld) {
  return fnv1a64(&lld, szof(lld), FNV1A64_HASH_INITIAL);
}
static purist unsigned long long
hash_lld(long long lld) {
  return fnv1a64(&lld, szof(lld), FNV1A64_HASH_INITIAL);
}

/* ---------------------------------------------------------------------------
 *                                Random
 * ---------------------------------------------------------------------------
 */
static purist inline unsigned long long
rnd_gen(unsigned long long val, int idx) {
  return val + castull(idx) * 0x9E3779B97F4A7C15llu;
}
static purist inline unsigned long long
rnd_mix(unsigned long long val) {
  val = (val ^ (val >> 30)) * 0xBF58476D1CE4E5B9llu;
  val = (val ^ (val >> 27)) * 0x94D049BB133111EBllu;
  return val ^ (val >> 31llu);
}
static inline unsigned long long
rnd_split_mix(unsigned long long *val, int idx) {
  requires(val);
  *val = rnd_gen(*val, idx);
  return rnd_mix(*val);
}
static inline unsigned long long
rnd(unsigned long long *val) {
  requires(val);
  return rnd_split_mix(val, 1);
}
static inline unsigned
rndu(unsigned long long *val) {
  requires(val);
  unsigned long long ret = rnd(val);
  return castu(ret & 0xffffffffu);
}
static inline int
rndi(unsigned long long *val) {
  requires(val);
  unsigned rnd = rndu(val);
  long long norm = castll(rnd) - (UINT_MAX/2);
  assert(norm >= INT_MIN && norm <= INT_MAX);
  return casti(norm);
}
static inline double
rndn(unsigned long long *val) {
  requires(val);
  unsigned rnd = rndu(val);
  return castd(rnd) / castd(UINT_MAX);
}
static unsigned
rnduu(unsigned long long *x, unsigned mini, unsigned maxi) {
  requires(x);
  unsigned lo = min(mini, maxi);
  unsigned hi = max(mini, maxi);
  unsigned rng = castu(-1);
  unsigned n = hi - lo + 1U;
  if (n == 1U)  {
    return mini;
  } else if(n == 0u) {
    return rndu(x);
  } else {
    unsigned v = 0;
    unsigned remainder = rng % n;
    do {v = rndu(x);}
    while(v >= rng - remainder);
    return mini + v % n;
  }
}
static inline float
rndf01(unsigned long long *x) {
  requires(x);
  unsigned u = rndu(x);
  double du = castd(u);
  double div = castd((unsigned)-1);
  return castf(du/div);
}
static float
rnduf(unsigned long long *x, float mini, float maxi) {
  requires(x);
  unsigned u = rndu(x);
  float lo = min(mini, maxi);
  float hi = max(mini, maxi);

  float rng = hi - lo;
  double du = castd(u);
  double div = castd((unsigned)-1);
  return lo + rng * castf(du/div);
}

/* ---------------------------------------------------------------------------
 *                                  Array
 * ---------------------------------------------------------------------------
 */
#define arrv(b) (b),cntof(b)
#define arr_shfl(a,n,p) do {                              \
  for (int uniqid(i) = 0; uniqid(i) < (n); ++uniqid(i)) { \
    if ((p)[uniqid(i)] >= 0) {                            \
      int uniqid(j) = uniqid(i);                          \
      while ((p)[uniqid(j)] != uniqid(i)) {               \
        int uniqid(d) = (p)[uniqid(j)];                   \
        swap((a)[uniqid(j)],(a)[uniqid(d)]);              \
        (p)[uniqid(j)] = -1 - uniqid(d);                  \
        uniqid(j) = uniqid(d);                            \
      } (p)[uniqid(j)] = -1 - (p)[uniqid(j)];             \
    }                                                     \
}} while (0)

#define arr_eachp(it,a,e) ((it) = (a); (it) < (e); ++(it))
#define arr_each(it,a,n) arr_eachp(it,a,(a)+(n))
#define arr_eachv(it,a) arr_eachp(it,(a),(a)+cntof(a))
#define arr_loopn(i,a,n) (int i = 0; i < min(n,cntof(a)); ++i)
#define arr_loopv(i,a) (int i = 0; i < cntof(a); ++i)
#define arr_loop(i,r) (int (i) = (r).lo; (i) != (r).hi; (i) += 1)
#define arr_rm(a,i,n) memmove(&(a)[i], &a[i+1], (size_t)(casti(n) - 1 - i) * sizeof((a)[0]))

/* ---------------------------------------------------------------------------
 *                                Sequence
 * ---------------------------------------------------------------------------
 */
static inline void
seq_rng(int *seq, struct rng rng) {
  requires(seq);
  for loopi(i,k,rng) {
    seq[k] = i;
  }
}
static inline void
seq_rngu(unsigned *seq, struct rng rng) {
  requires(seq);
  for loopi(i,k,rng) {
    seq[k] = castu(i);
  }
}
static inline void
seq_rnd(int *seq, int n, unsigned long long *r) {
  requires(r);
  requires(seq);
  for (int i = n - 1; i > 0; --i) {
    unsigned at = rndu(r) % castu(i + 1);
    iswap(seq[i], seq[at]);
  }
}
static inline void
seq_fix(int *p, int n) {
  requires(p);
  for (int i = 0; i < n; ++i) {
    p[i] = -1 - p[i];
  }
}

/* ---------------------------------------------------------------------------
 *                                  Bits
 * ---------------------------------------------------------------------------
 */
static purist inline unsigned
bit_rev32(unsigned x){
  x = ((x & 0x55555555) << 1) | ((x >> 1) & 0x55555555);
  x = ((x & 0x33333333) << 2) | ((x >> 2) & 0x33333333);
  x = ((x & 0x0F0F0F0F) << 4) | ((x >> 4) & 0x0F0F0F0F);
  x = (x << 24) | ((x & 0xFF00) << 8) | ((x >> 8) & 0xFF00) | (x >> 24);
  return x;
}
static purist inline unsigned long long
bit_rev64(unsigned long long x){
  unsigned x1 = castu(x & 0xffffffffu);
  unsigned x2 = castu(x >> 32);
  return (castull(bit_rev32(x1)) << 32)|bit_rev32(x2);
}
static purist inline unsigned
bit_eqv(unsigned x, unsigned y) {
  /* Calculates bitwise equivalence.
   * Bitwise equivalence is the opposite of xor. It sets all bits that
   * are the same to 1, whereas xor sets all bits that are different to 1.
   * Thus, you can simply use ~ to get bitwise equivalence from xor.
   * Example:
   *    11001100, 11110000 -> 11000011
   */
  return castu(~(x ^ y));
}

/* ---------------------------------------------------------------------------
 *                                  Bitset
 * ---------------------------------------------------------------------------
 */
#define bit_loop(i,x,s,n) \
  (int i = bit_ffs(s,n,0), x = 0; i < n; i = bit_ffs(s,n,i+1), x = x + 1)
static int bit_xor(unsigned long *addr, int nr);

static purist inline int
bit_tst(const unsigned long *addr, int nr) {
  requires(addr);
  unsigned long msk = (unsigned long)nr & (BITS_PER_LONG - 1);
  return (1ul & (addr[bit_word(nr)] >> msk)) != 0;
}
static inline int
bit_tst_clr(unsigned long *addr, int nr) {
  requires(addr);
  requires(nr >= 0);
  if (bit_tst(addr, nr)) {
    bit_xor(addr, nr);
    return 1;
  }
  return 0;
}
static inline int
bit_set(unsigned long *addr, int nr) {
  requires(addr);
  requires(nr >= 0);

  unsigned long m = bit_mask(nr);
  unsigned long *p = addr + bit_word(nr);
  unsigned long old_p = *p;
  int ret = casti(!!(*p & m));
  *p |= m;

  ensures(ret == casti(!!(old_p & m)));
  ensures(*p == (old_p | m));
  return ret;
}
static inline void
bit_set_on(unsigned long *addr, int nr, int cond) {
  requires(addr);
  requires(nr >= 0);
  if (cond) {
    bit_set(addr, nr);
  }
}
static inline int
bit_clr(unsigned long *addr, int nr) {
  requires(addr);
  requires(nr >= 0);

  unsigned long m = bit_mask(nr);
  unsigned long *p = addr + bit_word(nr);
  unsigned long old_p = *p;
  int ret = casti(!!(*p & m));
  *p &= ~m;

  ensures(ret == casti(!!(old_p & m)));
  ensures(*p == (old_p & ~m));
  return ret;
}
static inline void
bit_clr_on(unsigned long *addr, int nr, int cond) {
  requires(addr);
  if (cond) {
    bit_clr(addr, nr);
  }
}
static inline int
bit_xor(unsigned long *addr, int nr) {
  requires(addr);
  unsigned long m = bit_mask(nr);
  unsigned long *p = addr + bit_word(nr);
  *p ^= m;
  return (*p & m) ? 1 : 0;
}
static inline void
bit_fill(unsigned long *addr, int byte, int nbits) {
  requires(addr);
  int n = bits_to_long(nbits);
  mset(addr, byte, n * szof(long));
}
static int
bit_ffs(const unsigned long *addr, int nbits, int idx) {
  requires(addr);
  int ret = nbits;
  unsigned long off = bit_word_idx(idx);
  unsigned long long n = (unsigned long long)bits_to_long(nbits);
  for (unsigned long i = bit_word(idx); i < n; ++i) {
    unsigned long cmsk = bit_mask(off) - 1U;
    unsigned long c = addr[i] & (unsigned long)~cmsk;
    if (!c) {
      off = bit_word_nxt(off);
      continue;
    }
    int pos = cpu_bit_ffs64(c);
    ret = min((int)(i * BITS_PER_LONG) + pos, nbits);
    break;
  }
  ensures(idx <= ret);
  ensures(ret <= nbits);
  return ret;
}
static int
bit_ffz(const unsigned long *addr, int nbits, int idx) {
  requires(addr);
  requires(nbits >= 0);

  int ret = nbits;
  unsigned long off = bit_word_idx(idx);
  unsigned long long n = (unsigned long long)bits_to_long(nbits);
  for (unsigned long i = bit_word(idx); i < n; ++i) {
    unsigned long cmsk = bit_mask(off) - 1U;
    unsigned long c = (~addr[i]) & (unsigned long)~cmsk;
    if (!c) {
      off = bit_word_nxt(off);
      continue;
    }
    int pos = cpu_bit_ffs64(c);
    ret = min((int)(i * BITS_PER_LONG) + pos, nbits);
    break;
  }
  ensures(idx <= ret);
  ensures(ret <= nbits);
  return ret;
}
static purist int
bit_cnt_set(const unsigned long *addr, int nbits, int idx) {
  requires(addr);
  requires(nbits >= 0);
  requires(idx < nbits);

  unsigned long widx = bit_word(idx);
  unsigned long cmsk = max(1U, bit_mask(idx)) - 1U;
  if ((unsigned long)nbits < BITS_PER_LONG) {
    cmsk |= ~(max(1U, bit_mask(nbits)) - 1U);
  }
  unsigned long w = addr[widx] & ~cmsk;
  unsigned long long n = (unsigned long long)bits_to_long(nbits);
  int cnt = cpu_bit_cnt64(w);

  for (unsigned long i = widx + 1; i < n; ++i) {
    w = addr[i];
    if ((unsigned long)nbits - BITS_PER_LONG * i < BITS_PER_LONG) {
      cmsk |= ~(bit_mask(nbits) - 1U);
      w = w & ~cmsk;
    }
    cnt += cpu_bit_cnt64(w);
  }
  ensures(cnt >= 0);
  ensures(cnt <= nbits);
  return cnt;
}
static purist int
bit_cnt_zero(const unsigned long *addr, int nbits, int idx) {
  requires(addr);
  requires(nbits >= 0);
  requires(idx < nbits);

  unsigned long widx = bit_word(idx);
  unsigned long cmsk = max(1U, bit_mask(idx)) - 1U;
  if ((unsigned long)nbits < BITS_PER_LONG) {
    cmsk |= ~(max(1U, bit_mask(nbits)) - 1U);
  }
  unsigned long w = addr[widx]|cmsk;
  unsigned long long n = (unsigned long long)bits_to_long(nbits);
  int cnt = cpu_bit_cnt64(~w);

  for (unsigned long i = widx + 1; i < n; ++i) {
    w = addr[i];
    if ((unsigned long)nbits - BITS_PER_LONG * i < BITS_PER_LONG) {
      cmsk |= ~(bit_mask(nbits) - 1U);
      w = w|cmsk;
    }
    cnt += cpu_bit_cnt64(~w);
  }
  ensures(cnt >= 0);
  ensures(cnt <= nbits);
  return cnt;
}
static purist int
bit_set_at(const unsigned long *addr, int nbits, int off, int idx) {
  requires(addr);
  requires(nbits >= 0);
  requires(off < nbits);
  if (!idx && !off) {
    return bit_ffs(addr, nbits, idx);
  }
  for (int i = 0; i <= idx && off < nbits; ++off) {
    if (bit_tst(addr, off) && i++ == idx) {
      break;
    }
  }
  return off;
}
static purist int
bit_zero_at(const unsigned long *addr, int nbits, int off, int idx) {
  requires(addr);
  requires(nbits >= 0);
  requires(off < nbits);
  if (!idx && !off) {
    return bit_ffz(addr, nbits, idx);
  }
  for (int i = 0; i <= idx && off < nbits; ++off) {
    if (!bit_tst(addr, off) && i++ == idx) {
      break;
    }
  }
  return off;
}
static void
ut_bit(void) {
  {
    unsigned long data[2] = {0}; // Initialize test data.  Adjust size as needed.

    /* test setting and then checking a bit */
    data[0] = 0;
    data[0] |= bit_mask(5);
    assert(bit_tst(data, 5) == 1);
    assert(bit_tst(data, 6) == 0);

    data[1] = 0;
    data[1] |= bit_mask(BITS_PER_LONG + 10); // Set a bit in the next word
    assert(bit_tst(data, BITS_PER_LONG + 10) == 1);
    assert(bit_tst(data, BITS_PER_LONG + 11) == 0);

    data[0] = 0;
    assert(bit_tst(data, 0) == 0); // Test bit 0
    data[0] |= bit_mask(0);
    assert(bit_tst(data, 0) == 1);

    assert(bit_tst(data, BITS_PER_LONG - 1) == 0); // Last bit of a word
    data[0] |= bit_mask(BITS_PER_LONG - 1);
    assert(bit_tst(data, BITS_PER_LONG - 1) == 1);

    data[0] = ~0ul;
    for (int i = 0; i < BITS_PER_LONG; ++i) {
      assert(bit_tst(data, i) == 1);
    }
    data[1] = ~0ul;
    for (int i = BITS_PER_LONG; i < 2 * BITS_PER_LONG; ++i) {
      assert(bit_tst(data, i) == 1);
    }
  }
  {
    unsigned long data[2] = {0};

    // 1. Normal expected inputs (bit is set)
    data[0] |= bit_mask(5);
    assert(bit_tst_clr(data, 5) == 1); // Should clear and return 1
    assert(bit_tst(data, 5) == 0);    // Verify that the bit is cleared

    data[1] |= bit_mask(BITS_PER_LONG + 10);
    assert(bit_tst_clr(data, BITS_PER_LONG + 10) == 1);
    assert(bit_tst(data, BITS_PER_LONG + 10) == 0);

    // 2. Normal expected inputs (bit is not set)
    assert(bit_tst_clr(data, 7) == 0); // Should return 0 (bit not set)
    assert(bit_tst(data, 7) == 0);    // Bit should still be 0

    // 3. Edge cases
    data[0] |= bit_mask(0);
    assert(bit_tst_clr(data, 0) == 1);
    assert(bit_tst(data, 0) == 0);

    data[0] |= bit_mask(BITS_PER_LONG - 1);
    assert(bit_tst_clr(data, BITS_PER_LONG - 1) == 1);
    assert(bit_tst(data, BITS_PER_LONG - 1) == 0);

    // Test with all bits set
    data[0] = ~0ul;
    for (int i = 0; i < BITS_PER_LONG; ++i) {
      assert(bit_tst_clr(data, i) == 1);
      assert(bit_tst(data, i) == 0);
    }
    data[1] = ~0ul;
    for (int i = BITS_PER_LONG; i < 2 * BITS_PER_LONG; ++i) {
      assert(bit_tst_clr(data, i) == 1);
      assert(bit_tst(data, i) == 0);
    }
  }
  {
    unsigned long data[2] = {0};

    // 1. Normal expected inputs (bit is initially clear)
    assert(bit_set(data, 5) == 0); // Should set and return 0
    assert(bit_tst(data, 5) == 1);    // Verify that the bit is set

    assert(bit_set(data, BITS_PER_LONG + 10) == 0);
    assert(bit_tst(data, BITS_PER_LONG + 10) == 1);

    // 2. Normal expected inputs (bit is already set)
    assert(bit_set(data, 5) == 1); // Should return 1 (bit was already set)
    assert(bit_tst(data, 5) == 1);    // Bit should still be set

    // 3. Edge cases
    assert(bit_set(data, 0) == 0);
    assert(bit_tst(data, 0) == 1);

    assert(bit_set(data, BITS_PER_LONG - 1) == 0);
    assert(bit_tst(data, BITS_PER_LONG - 1) == 1);

    // Test with all bits initially set
    data[0] = ~0ul;
    for (int i = 0; i < BITS_PER_LONG; ++i) {
      assert(bit_set(data, i) == 1);  // All bits should already be set
      assert(bit_tst(data, i) == 1);
    }
    data[1] = ~0ul;
    for (int i = BITS_PER_LONG; i < 2 * BITS_PER_LONG; ++i) {
      assert(bit_set(data, i) == 1);  // All bits should already be set
      assert(bit_tst(data, i) == 1);
    }
  }
  {
    unsigned long data[2] = {0};

    // 1. Normal expected inputs (bit is initially set)
    data[0] |= bit_mask(5);  // Set the bit
    assert(bit_clr(data, 5) == 1); // Should clear and return 1 (was set)
    assert(bit_tst(data, 5) == 0);    // Verify that the bit is cleared

    data[1] |= bit_mask(BITS_PER_LONG + 10);
    assert(bit_clr(data, BITS_PER_LONG + 10) == 1);
    assert(bit_tst(data, BITS_PER_LONG + 10) == 0);

    // 2. Normal expected inputs (bit is initially clear)
    assert(bit_clr(data, 7) == 0); // Should return 0 (was already clear)
    assert(bit_tst(data, 7) == 0);    // Bit should still be clear

    // 3. Edge cases
    data[0] |= bit_mask(0);
    assert(bit_clr(data, 0) == 1);
    assert(bit_tst(data, 0) == 0);

    data[0] |= bit_mask(BITS_PER_LONG - 1);
    assert(bit_clr(data, BITS_PER_LONG - 1) == 1);
    assert(bit_tst(data, BITS_PER_LONG - 1) == 0);

     // Test with all bits initially set
    data[0] = ~0ul;
    for (int i = 0; i < BITS_PER_LONG; ++i) {
      assert(bit_clr(data, i) == 1);  // All bits should have been set
      assert(bit_tst(data, i) == 0);
    }
    data[1] = ~0ul;
    for (int i = BITS_PER_LONG; i < 2 * BITS_PER_LONG; ++i) {
      assert(bit_clr(data, i) == 1);  // All bits should have been set
      assert(bit_tst(data, i) == 0);
    }
  }
  {
    unsigned long data[2] = {0};

    // 1. Normal expected inputs (bit initially clear)
    assert(bit_xor(data, 5) == 1); // Should set and return 1
    assert(bit_tst(data, 5) == 1);

    assert(bit_xor(data, BITS_PER_LONG + 10) == 1);
    assert(bit_tst(data, BITS_PER_LONG + 10) == 1);

    // 2. Normal expected inputs (bit already set)
    assert(bit_xor(data, 5) == 0); // Should clear and return 0
    assert(bit_tst(data, 5) == 0);

    assert(bit_xor(data, BITS_PER_LONG + 10) == 0);
    assert(bit_tst(data, BITS_PER_LONG + 10) == 0);

    // 3. Edge cases
    assert(bit_xor(data, 0) == 1);
    assert(bit_tst(data, 0) == 1);
    assert(bit_xor(data, 0) == 0);
    assert(bit_tst(data, 0) == 0);

    assert(bit_xor(data, BITS_PER_LONG - 1) == 1);
    assert(bit_tst(data, BITS_PER_LONG - 1) == 1);
    assert(bit_xor(data, BITS_PER_LONG - 1) == 0);
    assert(bit_tst(data, BITS_PER_LONG - 1) == 0);

    // Test with all bits initially set
    data[0] = ~0ul;
    for (int i = 0; i < BITS_PER_LONG; ++i) {
      assert(bit_xor(data, i) == 0);  // All bits should have been set, so xor clears them
      assert(bit_tst(data, i) == 0);
    }
    data[1] = ~0ul;
    for (int i = BITS_PER_LONG; i < 2 * BITS_PER_LONG; ++i) {
      assert(bit_xor(data, i) == 0);  // All bits should have been set, so xor clears them
      assert(bit_tst(data, i) == 0);
    }
  }
  {
    unsigned long data[2] = {0};

    // 1. Normal expected inputs (one bit set)
    data[0] |= bit_mask(5);
    assert(bit_ffs(data, 64, 0) == 5);
    assert(bit_ffs(data, 64, 5) == 5); // Start at the set bit
    assert(bit_ffs(data, 64, 6) == 64); // Start after the set bit

    data[1] |= bit_mask(BITS_PER_LONG + 10);
    assert(bit_ffs(data, 128, 0) == 5);
    assert(bit_ffs(data, 128, BITS_PER_LONG + 10) == BITS_PER_LONG + 10);
    assert(bit_ffs(data, 128, BITS_PER_LONG + 11) == 128);

    // 2. No bits set
    data[0] = data[1] = 0;
    assert(bit_ffs(data, 64, 0) == 64); // Or whatever nbits should return.
    assert(bit_ffs(data, 128, 0) == 128);

    // 3. Multiple bits set in the same word
    data[0] |= bit_mask(2) | bit_mask(7);
    assert(bit_ffs(data, 64, 0) == 2); // Finds the lowest set bit
    assert(bit_ffs(data, 64, 3) == 7);

    // 4. Multiple bits set across words
    data[0] = data[1] = 0;
    data[0] |= bit_mask(3);
    data[1] |= bit_mask(BITS_PER_LONG + 1);
    assert(bit_ffs(data, 128, 0) == 3);
    assert(bit_ffs(data, 128, 4) == BITS_PER_LONG + 1);

    // 5. Start index within a word
    data[0] = data[1] = 0;
    data[0] |= bit_mask(5);
    assert(bit_ffs(data, 64, 3) == 5); // Start in middle of first word
    assert(bit_ffs(data, 64, 7) == 64); // Start past the set bit

    // 6. nbits smaller than actual data size.
    assert(bit_ffs(data, 5, 0) == 5); // nbits is smaller than the set bit.

    // 7. Edge Cases
    data[0] = 0;
    data[0] |= bit_mask(0);
    assert(bit_ffs(data, 64, 0) == 0);

    data[0] = 0;
    data[0] |= bit_mask(BITS_PER_LONG - 1);
    assert(bit_ffs(data, 64, 0) == BITS_PER_LONG - 1);

    // 8. All bits set
    data[0] = ~0ul;
    assert(bit_ffs(data, 64, 0) == 0); // Find first set bit.
    data[1] = ~0ul;
    assert(bit_ffs(data, 128, 0) == 0);
  }
  {
    unsigned long data[2] = {~0ul, ~0ul}; // Initialize with all bits set

    // 1. Normal expected inputs (one bit clear)
    data[0] &= ~bit_mask(5);
    assert(bit_ffz(data, 64, 0) == 5);
    assert(bit_ffz(data, 64, 5) == 5); // Start at the cleared bit
    assert(bit_ffz(data, 64, 6) == 64); // Start after the cleared bit

    data[1] &= ~bit_mask(BITS_PER_LONG + 10);
    assert(bit_ffz(data, 128, 0) == 5);
    assert(bit_ffz(data, 128, BITS_PER_LONG + 10) == BITS_PER_LONG + 10);
    assert(bit_ffz(data, 128, BITS_PER_LONG + 11) == 128);

    // 2. No bits clear (all bits set)
    data[0] = ~0ul;
    data[1] = ~0ul;
    assert(bit_ffz(data, 64, 0) == 64); // Or however nbits should return.
    assert(bit_ffz(data, 128, 0) == 128);

    // 3. Multiple bits clear in the same word
    data[0] = ~0ul;
    data[1] = ~0ul;
    data[0] &= ~(bit_mask(2) | bit_mask(7));
    assert(bit_ffz(data, 64, 0) == 2); // Finds the lowest cleared bit
    assert(bit_ffz(data, 64, 3) == 7);

    // 4. Multiple bits clear across words
    data[0] = ~0ul;
    data[1] = ~0ul;
    data[0] &= ~bit_mask(3);
    data[1] &= ~bit_mask(BITS_PER_LONG + 1);
    assert(bit_ffz(data, 128, 0) == 3);
    assert(bit_ffz(data, 128, 4) == BITS_PER_LONG + 1);

    // 5. Start index within a word
    data[0] = ~0ul;
    data[1] = ~0ul;
    data[0] &= ~bit_mask(5);
    assert(bit_ffz(data, 64, 3) == 5); // Start in middle of first word
    assert(bit_ffz(data, 64, 7) == 64); // Start past the cleared bit

    // 6. nbits smaller than actual data size.
    data[0] = ~0ul;
    data[1] = ~0ul;
    data[0] &= ~bit_mask(5);
    assert(bit_ffz(data, 5, 0) == 5); // nbits is smaller than the cleared bit.

    // 7. Edge Cases
    data[0] = ~0ul;
    data[0] &= ~bit_mask(0);
    assert(bit_ffz(data, 64, 0) == 0);

    data[0] = ~0ul;
    data[0] &= ~bit_mask(BITS_PER_LONG - 1);
    assert(bit_ffz(data, 64, 0) == BITS_PER_LONG - 1);

    // 8. All bits clear
    data[0] = 0;
    assert(bit_ffz(data, 64, 0) == 0); // Find first clear bit.
    data[1] = 0;
    assert(bit_ffz(data, 128, 0) == 0);
  }
  {
    unsigned long data[2] = {0};

    // 1. Normal expected inputs (a few bits set)
    data[0] |= bit_mask(2) | bit_mask(5) | bit_mask(7);
    assert(bit_cnt_set(data, 64, 0) == 3);
    assert(bit_cnt_set(data, 64, 3) == 2); // Start at bit 2
    assert(bit_cnt_set(data, 64, 7) == 1); // Start at bit 5
    assert(bit_cnt_set(data, 64, 8) == 0); // Start at bit 7

    data[0] = 0;
    data[1] |= bit_mask(BITS_PER_LONG + 1) | bit_mask(BITS_PER_LONG + 9);
    assert(bit_cnt_set(data, 128, 0) == 2);
    assert(bit_cnt_set(data, 128, BITS_PER_LONG + 2) == 1);
    assert(bit_cnt_set(data, 128, BITS_PER_LONG + 10) == 0);

    // 2. No bits set
    data[0] = data[1] = 0;
    assert(bit_cnt_set(data, 64, 0) == 0);
    assert(bit_cnt_set(data, 128, 0) == 0);

    // 3. All bits set
    data[0] = ~0ul;
    data[1] = ~0ul;
    assert(bit_cnt_set(data, 64, 0) == BITS_PER_LONG);
    assert(bit_cnt_set(data, 128, 0) == 2 * BITS_PER_LONG);

     // 4. nbits smaller than BITS_PER_LONG
    data[0] = 0;
    data[0] |= bit_mask(2) | bit_mask(5);
    assert(bit_cnt_set(data, 7, 0) == 2);
    assert(bit_cnt_set(data, 4, 0) == 1); // Test nbits smaller than first set bit.
    assert(bit_cnt_set(data, 2, 0) == 0); // Test nbits exactly equal to the first bit.
    assert(bit_cnt_set(data, 1, 0) == 0); // Test nbits smaller than the first bit.

    // 5. Edge cases
    data[0] = 0;
    data[0] |= bit_mask(0);
    assert(bit_cnt_set(data, 64, 0) == 1);

    data[0] = 0;
    data[0] |= bit_mask(BITS_PER_LONG - 1);
    assert(bit_cnt_set(data, 64, 0) == 1);

    // 6. idx near nbits
    data[0] = 0;
    data[0] |= bit_mask(62);
    assert(bit_cnt_set(data, 64, 60) == 1);
    assert(bit_cnt_set(data, 64, 62) == 1);
    assert(bit_cnt_set(data, 64, 63) == 0);

    // 7. idx equals nbits -1
    data[0] = 0;
    data[0] |= bit_mask(62);
    assert(bit_cnt_set(data, 63, 62) == 1); // nbits is 63 and idx is 62.
  }
  {
    unsigned long data[2] = {~0ul, ~0ul}; // Initialize with all bits set

    // 1. Normal expected inputs (a few bits cleared)
    data[0] &= ~(bit_mask(2) | bit_mask(5) | bit_mask(7));
    assert(bit_cnt_zero(data, 64, 0) == 3);
    assert(bit_cnt_zero(data, 64, 3) == 2); // Start at bit 2
    assert(bit_cnt_zero(data, 64, 6) == 1); // Start at bit 5
    assert(bit_cnt_zero(data, 64, 8) == 0); // Start at bit 7

    data[1] &= ~(bit_mask(BITS_PER_LONG + 1) | bit_mask(BITS_PER_LONG + 9));
    assert(bit_cnt_zero(data, 128, 0) == 5);
    assert(bit_cnt_zero(data, 128, BITS_PER_LONG + 1) == 2);
    assert(bit_cnt_zero(data, 128, BITS_PER_LONG + 9) == 1);
    assert(bit_cnt_zero(data, 128, BITS_PER_LONG + 10) == 0);

    // 2. No bits cleared (all bits set)
    data[0] = ~0ul;
    data[1] = ~0ul;
    assert(bit_cnt_zero(data, 64, 0) == 0);
    assert(bit_cnt_zero(data, 128, 0) == 0);

    // 3. All bits cleared
    data[0] = 0;
    data[1] = 0;
    assert(bit_cnt_zero(data, 64, 0) == BITS_PER_LONG);
    assert(bit_cnt_zero(data, 128, 0) == 2 * BITS_PER_LONG);

    // 4. nbits smaller than BITS_PER_LONG
    data[0] = ~0ul;
    data[0] &= ~(bit_mask(2) | bit_mask(5));
    assert(bit_cnt_zero(data, 7, 0) == 2);
    assert(bit_cnt_zero(data, 4, 0) == 1); // Test nbits smaller than first cleared bit.
    assert(bit_cnt_zero(data, 2, 0) == 0); // Test nbits exactly equal to the first bit.
    assert(bit_cnt_zero(data, 1, 0) == 0); // Test nbits smaller than the first bit.

    // 5. Edge cases
    data[0] = ~0ul;
    data[0] &= ~bit_mask(0);
    assert(bit_cnt_zero(data, 64, 0) == 1);

    data[0] = ~0ul;
    data[0] &= ~bit_mask(BITS_PER_LONG - 1);
    assert(bit_cnt_zero(data, 64, 0) == 1);

    // 6. idx near nbits
    data[0] = ~0ul;
    data[0] &= ~bit_mask(62);
    assert(bit_cnt_zero(data, 64, 60) == 1);
    assert(bit_cnt_zero(data, 64, 62) == 1);
    assert(bit_cnt_zero(data, 64, 63) == 0);

    // 7. idx equals nbits - 1
    data[0] = ~0ul;
    data[0] &= ~bit_mask(62);
    assert(bit_cnt_zero(data, 63, 62) == 1); // nbits is 63 and idx is 62.

    //8. nbits less than BITS_PER_LONG
    data[0] = ~0ul;
    data[0] &= ~(bit_mask(2) | bit_mask(5));
    assert(bit_cnt_zero(data, 6, 0) == 2);
    assert(bit_cnt_zero(data, 3, 0) == 1);
    assert(bit_cnt_zero(data, 2, 0) == 0);
    assert(bit_cnt_zero(data, 1, 0) == 0);
  }
  {
    unsigned long data[2] = {0};

    // 1. Normal expected inputs (bits set)
    data[0] |= bit_mask(2) | bit_mask(5) | bit_mask(7);
    assert(bit_set_at(data, 64, 0, 0) == 2); // First set bit
    assert(bit_set_at(data, 64, 0, 1) == 5); // Second set bit
    assert(bit_set_at(data, 64, 0, 2) == 7); // Third set bit
    assert(bit_set_at(data, 64, 0, 3) == 64); // No fourth set bit

    data[0] = 0ul;
    data[1] |= bit_mask(BITS_PER_LONG + 1) | bit_mask(BITS_PER_LONG + 9);
    assert(bit_set_at(data, 128, 0, 0) == BITS_PER_LONG + 1);
    assert(bit_set_at(data, 128, 0, 1) == BITS_PER_LONG + 9);
    assert(bit_set_at(data, 128, 0, 2) == 128);

    // 2. No bits set
    data[0] = data[1] = 0ul;
    assert(bit_set_at(data, 64, 0, 0) == 64);
    assert(bit_set_at(data, 128, 0, 0) == 128);

    // 3. Start offset in the middle of a word
    data[0] |= bit_mask(5) | bit_mask(7);
    assert(bit_set_at(data, 64, 3, 0) == 5);
    assert(bit_set_at(data, 64, 6, 0) == 7);
    assert(bit_set_at(data, 64, 8, 0) == 64);

    // 4. Start offset in the middle of a word, multiple words
    data[0] = data[1] = 0ul;
    data[0] |= bit_mask(5);
    data[1] |= bit_mask(BITS_PER_LONG + 2);
    assert(bit_set_at(data, 128, 3, 0) == 5);
    assert(bit_set_at(data, 128, 6, 0) == BITS_PER_LONG + 2);
    assert(bit_set_at(data, 128, BITS_PER_LONG + 3, 0) == 128);

    // 5. idx = 0
    data[0] |= bit_mask(5) | bit_mask(7);
    data[0] |= bit_mask(2);
    assert(bit_set_at(data, 64, 0, 0) == 2);

    // 6. Edge Cases
    data[0] = 0;
    data[0] |= bit_mask(0);
    assert(bit_set_at(data, 64, 0, 0) == 0);

    data[0] = 0;
    data[0] |= bit_mask(BITS_PER_LONG - 1);
    assert(bit_set_at(data, 64, 0, 0) == BITS_PER_LONG - 1);

    // 7. nbits smaller than data size
    data[0] = data[1] = 0;
    data[0] |= bit_mask(2);
    assert(bit_set_at(data, 5, 0, 0) == 2);
    assert(bit_set_at(data, 1, 0, 0) == 1); // No set bits within the range
  }
  {
    unsigned long data[2] = {~0ul, ~0ul}; // Initialize with all bits set

    // 1. Normal expected inputs (bits cleared)
    data[0] &= ~(bit_mask(2) | bit_mask(5) | bit_mask(7));
    assert(bit_zero_at(data, 64, 0, 0) == 2); // First cleared bit
    assert(bit_zero_at(data, 64, 0, 1) == 5); // Second cleared bit
    assert(bit_zero_at(data, 64, 0, 2) == 7); // Third cleared bit
    assert(bit_zero_at(data, 64, 0, 3) == 64); // No fourth cleared bit

    data[0] = ~0ul;
    data[1] = ~0ul;
    data[1] &= ~(bit_mask(BITS_PER_LONG + 1) | bit_mask(BITS_PER_LONG + 9));
    assert(bit_zero_at(data, 128, 0, 0) == BITS_PER_LONG + 1);
    assert(bit_zero_at(data, 128, 0, 1) == BITS_PER_LONG + 9);
    assert(bit_zero_at(data, 128, 0, 2) == 128);

    // 2. No bits cleared (all bits set)
    data[0] = ~0ul;
    data[1] = ~0ul;
    assert(bit_zero_at(data, 64, 0, 0) == 64);
    assert(bit_zero_at(data, 128, 0, 0) == 128);

    // 3. Start offset in the middle of a word
    data[0] &= ~(bit_mask(5) | bit_mask(7));
    assert(bit_zero_at(data, 64, 3, 0) == 5);
    assert(bit_zero_at(data, 64, 6, 0) == 7);
    assert(bit_zero_at(data, 64, 8, 0) == 64);

    // 4. Start offset in the middle of a word, multiple words
    data[0] = ~0ul;
    data[1] = ~0ul;
    data[0] &= ~bit_mask(5);
    data[1] &= ~bit_mask(BITS_PER_LONG + 2);
    assert(bit_zero_at(data, 128, 3, 0) == 5);
    assert(bit_zero_at(data, 128, 6, 0) == BITS_PER_LONG + 2);
    assert(bit_zero_at(data, 128, BITS_PER_LONG + 3, 0) == 128);

    // 5. idx = 0
    data[0] = ~0ul;
    data[0] &= ~bit_mask(2);
    assert(bit_zero_at(data, 64, 0, 0) == 2);

    // 6. Edge Cases
    data[0] = ~0ul;
    data[0] &= ~bit_mask(0);
    assert(bit_zero_at(data, 64, 0, 0) == 0);

    data[0] = ~0ul;
    data[0] &= ~bit_mask(BITS_PER_LONG - 1);
    assert(bit_zero_at(data, 64, 0, 0) == BITS_PER_LONG - 1);

    // 7. nbits smaller than data size
    data[0] = ~0ul;
    data[0] &= ~bit_mask(2);
    assert(bit_zero_at(data, 5, 0, 0) == 2);
    assert(bit_zero_at(data, 1, 0, 0) == 1); // No cleared bits within the range
  }
}

/* ---------------------------------------------------------------------------
 *                                  Unicode
 * ---------------------------------------------------------------------------
 */
// clang-format off
#define is_upper(c) (((unsigned)c - 'A') < 26)
#define is_lower(c) (((unsigned)c - 'a') < 26)
#define to_lower(c) is_upper(c) ? (c) | 32 : c
#define to_upper(c) is_lower(c) ? ((c) & ~32) : c
#define is_digit(c) (((c) >= '0') && ((c) <= '9'))
#define is_hex(c) (is_digit(c) || (((c) >= 'a') && ((c) <= 'f')) || (((c) >= 'A') && ((c) <= 'F')))
#define is_alpha(c) (is_lower(c) || is_upper(c))
#define is_printable(c) ((c) >= 32 && (c) <= 126)
// clang-format on

static purist int
is_space(long c) {
  switch (c) {
  default: return 0;
  case 0x0020:
  case 0x0009:
  case 0x000a:
  case 0x000b:
  case 0x000c:
  case 0x000d:
  case 0x00A0:
  case 0x1680:
  case 0x2000:
  case 0x2001:
  case 0x2002:
  case 0x2003:
  case 0x2004:
  case 0x2005:
  case 0x2006:
  case 0x2007:
  case 0x2008:
  case 0x2009:
  case 0x200A:
  case 0x202F:
  case 0x205F:
  case 0x3000:
    return 1;
  }
}
static purist int
is_quote(long c) {
  switch (c) {
  default: return 0;
  case '\"':
  case '`':
  case '\'':
  case 0x00AB:
  case 0x00BB:
  case 0x2018:
  case 0x2019:
  case 0x201A:
  case 0x201C:
  case 0x201D:
  case 0x201E:
  case 0x2039:
  case 0x203A:
    return 1;
  }
}
static purist int
is_punct(long c) {
  switch (c) {
  default: return is_quote(c);
  case ',':
  case '.':
  case ';':
  case '(':
  case ')':
  case '{':
  case '}':
  case '[':
  case ']':
  case '<':
  case '>':
  case '|':
  case '/':
  case '?':
  case '#':
  case '~':
  case '@':
  case '=':
  case '+':
  case '-':
  case '_':
  case '*':
  case '&':
  case '^':
  case '%':
  case '$':
  case '!':
  case '\\':
  case ':':
  case 0x0964:
  case 0x0589:
  case 0x3002:
  case 0x06D4:
  case 0x2CF9:
  case 0x0701:
  case 0x1362:
  case 0x166E:
  case 0x1803:
  case 0x2FCE:
  case 0xA4FF:
  case 0xA60E:
  case 0xA6F3:
  case 0x083D:
  case 0x1B5F:
  case 0x060C:
  case 0x3001:
  case 0x055D:
  case 0x07F8:
  case 0x1363:
  case 0x1808:
  case 0xA4FE:
  case 0xA60D:
  case 0xA6F5:
  case 0x1B5E:
  case 0x2047:
  case 0x2048:
  case 0x2049:
  case 0x203D:
  case 0x2757:
  case 0x203C:
  case 0x2E18:
  case 0x00BF:
  case 0x061F:
  case 0x055E:
  case 0x0706:
  case 0x1367:
  case 0x2CFA:
  case 0x2CFB:
  case 0xA60F:
  case 0xA6F7:
  case 0x11143:
  case 0xAAF1:
  case 0x00A1:
  case 0x07F9:
  case 0x1944:
  case 0x00B7:
  case 0x1039F:
  case 0x103D0:
  case 0x12470:
  case 0x1361:
  case 0x1091:
  case 0x0830:
  case 0x058A:
  case 0x1806:
  case 0x0387:
  case 0x061B:
  case 0x1364:
  case 0x2024:
  case 0x1365:
  case 0xA6F6:
  case 0x1B5D:
  case 0x2026:
  case 0xFE19:
  case 0x0EAF:
  case 0x00AB:
  case 0x2039:
  case 0x00BB:
  case 0x203A:
  case 0x00AF:
  case 0x00B2:
  case 0x00B3:
  case 0x00B4:
  case 0x00B5:
  case 0x00B6:
  case 0x00B8:
  case 0x00B9:
  case 0x00BA:
  case 0x2010:
  case 0x2013:
  case 0x2014:
  case 0x2015:
  case 0x2016:
  case 0x2020:
  case 0x2021:
  case 0x2022:
  case 0x2025:
  case 0x2030:
  case 0x2031:
  case 0x2032:
  case 0x2033:
  case 0x2034:
  case 0x2035:
  case 0x203E:
  case 0x2041:
  case 0x2043:
  case 0x2044:
  case 0x204F:
  case 0x2057:
    return 1;
  }
}
/* ---------------------------------------------------------------------------
 *                                  String
 * ---------------------------------------------------------------------------
 */
// clang-format off
#define cstrn(s) casti(strlen(s))
#define str(p,r) (struct str){.ptr = (p), .rng = (r)}
#define strn(s,n) str(s,rngn(n))
#define str0(s) str(s,rngn(cstrn(s)))
#define strc(c) str(&c,rngn(1))
#define strf(s) (s).rng.cnt, str_beg(s)
#define str_nil (struct str){0,rngn(0)}
#define str_inv (struct str){0,rng_inv}
#define str_len(s) rng_cnt(&(s).rng)
#define str_is_empty(s) (str_len(s) == 0)
#define str_is_inv(s) ((s).ptr == 0 || rng_is_inv((s).rng))
#define str_is_val(s) (!rng_is_inv((s).rng))
#define str_eq(a,b) (str_cmp(a,b) == 0)
#define str_neq(a,b) (str_cmp(a,b) != 0)
#define str_sub(s,b,e) str((s).ptr, rng_sub(&(s).rng, b, e))
#define str_rhs(s,n) str_sub(s, min(str_len(s), n), str_len(s))
#define str_lhs(s,n) str_sub(s, 0, min(str_len(s), n))
#define str_cut_lhs(s,n) *(s) = str_rhs(*(s), n)
#define str_cut_rhs(s,n) *(s) = str_lhs(*(s), n)
#define str_beg(s) (((s).ptr) ? (slc_beg((s).ptr, (s).rng)) : 0)
#define str_end(s) (((s).ptr) ? (slc_end((s).ptr, (s).rng)) : 0)
#define str_at(s,i) slc_at((s).ptr, (s).rng, i)
#define str_ptr(s,i) slc_ptr((s).ptr, (s).rng, i)

#define str_each(it,c) (const char *it = slc_beg((c).ptr, (c).rng); it < slc_end((c).ptr, (c).rng); it += 1)
#define str_eachr(it,s,r) (const char *it = (s).str + (s).rng.lo + (r).lo; (it) != (s).str + (s).rng.hi - (r).hi; (it) += 1)
#define str_loop(i,s) (int i = 0; i < str_len(s); ++i)
#define str_tok(it, rest, src, delim)                       \
  (struct str rest = src, it = str_split_cut(&rest, delim); \
   str_len(it); it = str_split_cut(&rest, delim))
// clang-format on

static inline int
str__is_val(const struct str *str) {
  assert(str);
  assert(rng__is_val(&str->rng));
  assert(str->rng.lo >= 0);
  assert(str->rng.hi >= str->rng.lo);
  assert(str->rng.cnt == str->rng.hi - str->rng.lo);
  assert(str->rng.cnt <= str->rng.total);
  return 1;
}
static purist inline struct str
strptr(const char *ptr, const char *begin, const char *end, int total) {
  if (ptr == 0 || begin == 0 || end == 0) {
    return str_nil;
  }
  struct str str = {.ptr = ptr};
  int low = max(0, casti(begin - ptr));
  int hih = max(0, casti(end - ptr));
  int cnt = max(0, casti(hih - low));

  str.rng = (struct rng){low, hih, cnt, total};
  ensures(str__is_val(&str));
  return str;
}
static purist inline struct str
strp(const char *begin, const char *end) {
  if (begin == 0 || end == 0) {
    return str_nil;
  }
  int cnt = casti(end - begin);
  struct str ret = strptr(begin, begin, end, cnt);
  ensures(str__is_val(&ret));
  return ret;
}
static purist unsigned long long
str__hash(struct str str, unsigned long long id) {
  if (str_is_inv(str)) {
    return id;
  }
  return fnv1a64(str_beg(str), str_len(str), id);
}
static purist unsigned long long
str_hash(struct str str) {
  return str__hash(str, FNV1A64_HASH_INITIAL);
}
static purist int
str_cmp(struct str lhs, struct str rhs) {
  requires(str__is_val(&lhs));
  requires(str__is_val(&rhs));

  int cnt = min(str_len(lhs), str_len(rhs));
  for loop(i,cnt) {
    if (str_at(lhs,i) < str_at(rhs,i)) {
      return -1;
    } else if (str_at(lhs,i) > str_at(rhs,i)) {
      return +1;
    }
  }
  if (str_len(lhs) > str_len(rhs)) {
    return +1;
  } else if (str_len(lhs) < str_len(rhs)) {
    return -1;
  }
  return 0;
}
static purist int
str_fnd(struct str hay, struct str needle) {
  requires(str__is_val(&hay));
  requires(str__is_val(&needle));
  int ret = str_len(hay);
  if (str_len(hay) == 0 ||
      str_len(needle) == 0) {
    return ret;
  }
  if (str_len(needle) == 1) {
    const char *atp = cpu_str_chr(str_beg(hay), str_len(hay), str_at(needle,0));
    if (atp) {
      ret = casti(atp - str_beg(hay));
    }
  } else {
    ret = cpu_str_fnd(str_beg(hay), castsz(str_len(hay)), str_beg(needle), castsz(str_len(needle)));
  }
  ensures(ret >= 0);
  ensures(ret <= str_len(hay));
  return ret;
}
static purist inline int
str_has(struct str hay, struct str needle) {
  requires(str__is_val(&hay));
  requires(str__is_val(&needle));
  return str_fnd(hay, needle) < str_len(hay);
}
static struct str
str_split_cut(struct str *str, struct str delim) {
  requires(str__is_val(str));
  requires(str__is_val(&delim));

  struct str ret;
  int pos = str_fnd(*str, delim);
  if (pos < str_len(*str)) {
    ret = str_lhs(*str, pos);
    str_cut_lhs(str, pos + 1);
  } else {
    ret = *str;
    *str = str_nil;
  }
  requires(str__is_val(&ret));
  return ret;
}
static void
ut_str(void) {
  {
    char test_str[] = "abcde";
    const char *begin = test_str + 1; // Start of the substring
    const char *end = test_str + 4; // End of the substring (not inclusive)
    struct str expected_str = {.ptr = test_str, .rng = {.lo = 1, .hi = 4, .cnt = 3, .total = 5}};
    struct str actual_str = strptr(test_str, begin, end, cstrn(test_str));
    assert(memcmp(&expected_str, &actual_str, sizeof(struct str)) == 0);
  }
  {
    char test_str[] = "abcde";

    // Test case 1: ptr at the beginning
    const char *ptr = test_str;
    const char *begin = test_str;
    const char *end = test_str + 2;
    struct str expected_str = {.ptr = ptr, .rng = {.lo = 0, .hi = 2, .cnt = 2, .total = 5}};
    struct str actual_str = strptr(ptr, begin, end, cstrn(test_str));
    assert(memcmp(&expected_str, &actual_str, sizeof(struct str)) == 0);
  }
  {
    char test_str[] = "abcde";
    // Test case 2: ptr at the end
    const char *ptr = test_str + cstrn(test_str) - 1;
    const char *begin = test_str + cstrn(test_str) - 2;
    const char *end = test_str + cstrn(test_str);
    struct str expected_str = {.ptr = ptr, .rng = {.lo = 4, .hi = 5, .cnt = 1, .total = 5}};
    struct str actual_str = strptr(ptr, begin, end, cstrn(test_str));
    assert(memcmp(&expected_str, &actual_str, 1) == 0);
  }
  {
    char test_str[] = "abcde";
    const char *begin = test_str + 1; // Start of the substring
    const char *end = test_str + 4; // End of the substring (not inclusive)
    struct str expected_str = {.ptr = begin, .rng = {.lo = 0, .hi = 3, .cnt = 3, .total = 3}};
    struct str actual_str = strp(begin, end);
    assert(memcmp(&expected_str, &actual_str, sizeof(struct str)) == 0);
  }
  {
    // Test case 2: Empty string
    char empty_str[] = "";
    const char *begin = empty_str;
    const char *end = empty_str;
    struct str expected_str = {.ptr = begin, .rng = {.lo = 0, .hi = 0, .cnt = 0, .total = 0}};
    struct str actual_str = strp(begin, end);
    assert(memcmp(&expected_str, &actual_str, sizeof(struct str)) == 0);
  }
  {
    char test_str[] = "abcde";

    // Test case 1: begin and end at the same position
    const char *begin = test_str + 2;
    const char *end = test_str + 2;
    struct str expected_str = {.ptr = begin, .rng = {.lo = 0, .hi = 0, .cnt = 0, .total = 0}};
    struct str actual_str = strp(begin, end);
    assert(memcmp(&expected_str, &actual_str, sizeof(struct str)) == 0);
  }
  {
    struct str lhs = {.ptr = "abc", .rng = {.lo = 0, .hi = 3, .cnt = 3, .total = 3}};
    struct str rhs = {.ptr = "abc", .rng = {.lo = 0, .hi = 3, .cnt = 3, .total = 3}};
    assert(str_cmp(lhs, rhs) == 0);
  }
  {
    struct str lhs = {.ptr = "abc", .rng = {.lo = 0, .hi = 3, .cnt = 3, .total = 3}};
    struct str rhs = {.ptr = "abd", .rng = {.lo = 0, .hi = 3, .cnt = 3, .total = 3}};
    assert(str_cmp(lhs, rhs) == -1);
  }
  {
    struct str lhs = {.ptr = "abd", .rng = {.lo = 0, .hi = 3, .cnt = 3, .total = 3}};
    struct str rhs = {.ptr = "abc", .rng = {.lo = 0, .hi = 3, .cnt = 3, .total = 3}};
    assert(str_cmp(lhs, rhs) == 1);
  }
  {
    struct str lhs = {.ptr = "ab", .rng = {.lo = 0, .hi = 2, .cnt = 2, .total = 2}};
    struct str rhs = {.ptr = "abc", .rng = {.lo = 0, .hi = 3, .cnt = 3, .total = 3}};
    assert(str_cmp(lhs, rhs) == -1);
  }
   {
    struct str lhs = {.ptr = "abc", .rng = {.lo = 0, .hi = 3, .cnt = 3, .total = 3}};
    struct str rhs = {.ptr = "ab", .rng = {.lo = 0, .hi = 2, .cnt = 2, .total = 2}};
    assert(str_cmp(lhs, rhs) == 1);
  }
  {
    char test_str[64] = "cmd[stk?utf/boot/usr/str_bootbany.exe";
    const struct str hay = str0(test_str);
    int dot_pos = str_fnd(hay, strv("["));
    assert(dot_pos == 3);
    int cmd_pos = str_fnd(hay, strv("cmd"));
    assert(cmd_pos == 0);
    int str_pos = str_fnd(hay, strv("?utf"));
    assert(str_pos == 7);
    int long_pos = str_fnd(hay, strv("boot/usr/str_bootbany"));
    assert(long_pos == 12);
    int close_no = str_fnd(hay, strv("str/"));
    assert(close_no == str_len(hay));
    int no = str_fnd(hay, strv("rock"));
    assert(no == str_len(hay));
  }
  {
    struct str str = {.ptr = "hello world", .rng = {.lo = 0, .hi = 11, .cnt = 11, .total = 11}};
    struct str delim = {.ptr = " ", .rng = {.lo = 0, .hi = 1, .cnt = 1, .total = 1}};
    struct str expected_ret = {.ptr = "hello", .rng = {.lo = 0, .hi = 5, .cnt = 5, .total = 11}};
    struct str actual_ret = str_split_cut(&str, delim);
    assert(memcmp(&expected_ret.rng, &actual_ret.rng, sizeof(struct rng)) == 0);
    assert(expected_ret.ptr = str.ptr);
    assert(!memcmp(str.ptr + str.rng.lo, "world", 5) && str.rng.lo == 6 && str.rng.hi == 11 && str.rng.cnt == 5 && str.rng.total == 11);
  }
  {
    struct str str = {.ptr = "hello", .rng = {.lo = 0, .hi = 5, .cnt = 5, .total = 5}};
    struct str delim = {.ptr = " ", .rng = {.lo = 0, .hi = 1, .cnt = 1, .total = 1}};
    struct str expected_ret = {.ptr = "hello", .rng = {.lo = 0, .hi = 5, .cnt = 5, .total = 5}};
    struct str actual_ret = str_split_cut(&str, delim);
    assert(memcmp(&expected_ret, &actual_ret, sizeof(struct str)) == 0);
    assert(str.ptr == NULL && str.rng.lo == 0 && str.rng.hi == 0 && str.rng.cnt == 0 && str.rng.total == 0);
  }
  {
    struct str str = {.ptr = "hello", .rng = {.lo = 0, .hi = 5, .cnt = 5, .total = 5}};
    struct str delim = {.ptr = "", .rng = {.lo = 0, .hi = 0, .cnt = 0, .total = 0}};
    struct str expected_ret = {.ptr = "hello", .rng = {.lo = 0, .hi = 5, .cnt = 5, .total = 5}};
    struct str actual_ret = str_split_cut(&str, delim);
    assert(memcmp(&expected_ret, &actual_ret, sizeof(struct str)) == 0);
    assert(str.ptr == NULL && str.rng.lo == 0 && str.rng.hi == 0 && str.rng.cnt == 0 && str.rng.total == 0);
  }
}

/* ---------------------------------------------------------------------------
 *                                  UTF-8
 * ---------------------------------------------------------------------------
 */
#define UTF_SIZ 4
#define UTF_BUF (UTF_SIZ + 1)
#define UTF_INVALID 0xFFFD

static const unsigned char utf_byte[UTF_SIZ+1] = {0x80, 0, 0xC0, 0xE0, 0xF0};
static const unsigned char utf_mask[UTF_SIZ+1] = {0xC0, 0x80, 0xE0, 0xF0, 0xF8};
static const unsigned utf_min[UTF_SIZ+1] = {0, 0, 0x80, 0x800, 0x10000};
static const unsigned utf_max[UTF_SIZ+1] = {0x10FFFF, 0x7F, 0x7FF, 0xFFFF, 0x10FFFF};

#define utf_enc_byte(u,i) ((char)((utf_byte[i]) | ((unsigned char)u & ~utf_mask[i])))
#define utf_tst(c) (((c) & 0xC0) != 0x80)
#define utf_val(u,i) (between(u, utf_min[i], utf_max[i]) && !between(u, 0xD800, 0xDFFF))
#define utf_loop(rune, it, rest, src)\
  (struct str rest = src, it = utf_dec(rune, &rest); it.rng.cnt; it = utf_dec(rune, &rest))
#define utf_loop_rev(rune, it, rest, src)\
  (struct str rest = src, it = utf_dec_rev(rune, &rest); str_len(it); it = utf_dec_rev(rune, &rest))

static struct str
utf_dec(unsigned *rune, struct str *str) {
  requires(str__is_val(str));
  if (str_is_empty(*str)) {
    if (rune) {
      *rune = UTF_INVALID;
    }
    if (str_is_inv(*str)) {
      return str_nil;
    }
    return strptr(str->ptr, str_end(*str), str_end(*str), str->rng.total);
  }
  int cnt = 0;
  unsigned ret = 0;
  const char *ptr = str_beg(*str);
  switch (*ptr & 0xf0) {
    // clang-format off
    case 0xf0: ret = (*ptr & 0x07); cnt = 3; break;
    case 0xe0: ret = (*ptr & 0x0f); cnt = 2; break;
    case 0xc0: ret = (*ptr & 0x1f); cnt = 1; break;
    case 0xd0: ret = (*ptr & 0x1f); cnt = 1; break;
    default:   ret = (*ptr & 0xff); cnt = 0; break;
    // clang-format on
  }
  if (str_beg(*str) + cnt + 1 > str_end(*str)) {
    if (rune) {
      *rune = UTF_INVALID;
    }
    *str = strptr(str->ptr, str_end(*str), str_end(*str), str->rng.total);
    return *str;
  }
  struct str view = strptr(str->ptr, ptr, ptr + cnt + 1, str->rng.total);
  for (int i = 0; i < cnt; ++i) {
    ret = (ret << 6) | (*(++ptr) & 0x3f);
  }
  if (rune) {
    *rune = ret;
  }
  *str = strptr(str->ptr, str_beg(*str) + cnt + 1, str_end(*str), str->rng.total);
  requires(str__is_val(str));
  return view;
}
static unsigned
utf_get(struct str str) {
  unsigned rune;
  utf_dec(&rune, &str);
  return rune;
}
static struct str
utf_dec_rev(unsigned *rune, struct str *str) {
  requires(str__is_val(str));
  const char *ptr = str_end(*str);
  while (ptr > str_beg(*str)) {
    char pnt = *(--ptr);
    if (utf_tst(pnt)) {
      struct str rest = strptr(str->ptr, ptr, str_end(*str), str->rng.total);
      struct str itr = utf_dec(rune, &rest);
      *str = strptr(str->ptr, str_beg(*str), ptr, str->rng.total);
      return itr;
    }
  }
  *str = str_nil;
  *rune = UTF_INVALID;
  return str_nil;
}
static int
utf_enc(char *buf, int cap, unsigned rune) {
  requires(0 <= cap);
  requires(cap >= UTF_BUF);
  int cnt = 0;
  if (!utf_val(rune, 0)) {
    return 0;
  }
  for (cnt = 1; rune > utf_max[cnt]; ++cnt);
  if (cap < cnt || !cnt || cnt > UTF_SIZ) {
    return 0;
  }
  for (int i = cnt - 1; i != 0; --i) {
    buf[i] = utf_enc_byte(rune, 0);
    rune >>= 6;
  }
  buf[0] = utf_enc_byte(rune, cnt);
  return cnt;
}
static struct str
utf_at(unsigned *rune, struct str str, int idx) {
  requires(str__is_val(&str));

  int i = 0;
  unsigned glyph = 0;
  for utf_loop(&glyph, itr, _, str) {
    if (i >= idx) {
      if (rune) {
        *rune = glyph;
      }
      return itr;
    }
    i++;
  }
  if (rune) {
    *rune = UTF_INVALID;
  }
  if (str_is_inv(str)) {
    return str_nil;
  }
  return strptr(str.ptr, str_end(str), str_end(str), str.rng.total);
}
static purist int
utf_at_idx(struct str str, int idx) {
  requires(str__is_val(&str));
  struct str view = utf_at(0, str, idx);
  if (str_len(view)) {
    return casti(str_beg(view) - str_beg(str));
  }
  return str_len(str);
}
static purist int
utf_len(struct str str) {
  int i = 0;
  unsigned rune = 0;
  for utf_loop(&rune, _, _2, str) {
    i++;
  }
  return i;
}
static void
ut_utf(void) {
  {
    static const struct str test_cases[] = {
      strv("a"),       // ASCII character
      strv("\u00C4"),   // Umlaut character (Ä)
      strv("\u20AC"),   // Euro symbol (€)
      strv("\u4E00"),   // CJK character (一)
    };
    static const long runes[] = {
      'a', 196, 8364, 19968
    };
    for arr_loopv(i, test_cases) {
      unsigned rune;
      struct str str = test_cases[i];
      struct str result = utf_dec(&rune, &str);
      assert(result.ptr != NULL); // Ensure a valid str is returned
      assert(str_is_empty(str));   // Ensure str is advanced
      assert(rune == runes[i]); // Check decoded character (ASCII)
    }
  }
  {
    unsigned rune;
    static const char utf8_str[] = "\xF0\x9F\x98\x80"; // U+1F600 (GRINNING FACE)
    struct str input = str0(utf8_str); // Use str0 for null-terminated strings
    struct str result = utf_dec(&rune, &input);

    assert(rune == 0x1F600); // Check if the rune was decoded correctly
    assert(result.rng.cnt == 4); // Check if we consumed 4 bytes
    assert(str_len(input) == 0); // Check if all bytes were processed
  }
  {
    unsigned rune;
    const char utf8_str[] = "\xE2\x82\xAC"; // Euro sign U+20AC
    struct str input = str0(utf8_str);
    struct str result = utf_dec(&rune, &input);

    assert(rune == 0x20AC); // Euro sign should be decoded correctly
    assert(result.rng.cnt == 3); // Should consume 3 bytes
    assert(str_len(input) == 0); // All bytes processed
  }
  {
    // Test cases for incomplete UTF-8 sequences (truncated in the middle)
    static const struct str test_cases[] = {
      strv("\xC3"),    // Missing continuation byte
      strv("\xE0\x80"), // Missing continuation byte
    };
    for arr_loopv(i, test_cases) {
      unsigned rune;
      struct str str = test_cases[i];
      struct str result = utf_dec(&rune, &str);
      assert(result.ptr == str.ptr); // Ensure str remains unchanged
      assert(rune == UTF_INVALID);   // Ensure UTF_INVALID is returned
    }
  }
  {
    unsigned rune;
    const char empty_str[] = "";
    struct str input = str0(empty_str); // Empty but not str_nil
    struct str result = utf_dec(&rune, &input);

    assert(rune == UTF_INVALID); // Should return invalid for empty string
    assert(str_len(result) == 0); // No bytes consumed
    assert(str_len(input) == 0); // Input should still be empty
  }
  {
    // Test case for an empty string
    struct str str = str_nil;
    unsigned rune;
    struct str result = utf_dec_rev(&rune, &str);
    assert(result.ptr == str.ptr);  // Ensure str remains unchanged
    assert(rune == UTF_INVALID);    // Ensure UTF_INVALID is returned
  }
  {
    unsigned test_code_points[] = {
      'a',                // ASCII character
      0x00C4,             // Umlaut character (Ä)
      0x20AC,             // Euro symbol (€)
      0x4E00,             // CJK character (一)
    };
    for (size_t i = 0; i < cntof(test_code_points); ++i) {
      unsigned rune = test_code_points[i];
      char buf[UTF_SIZ+1];
      int bytes_written = utf_enc(buf, cntof(buf), rune);
      assert(bytes_written > 0);

      // Check if the encoded bytes represent the correct rune
      unsigned decoded_rune = 0;
      struct str str = strptr(buf, buf, buf + bytes_written, bytes_written);
      struct str result = utf_dec(&decoded_rune, &str);
      assert(result.ptr != NULL);
      assert(str_is_empty(str));
      assert(decoded_rune == rune);
    }
  }
  {
    // Test cases for invalid code points
    unsigned invalid_code_points[] = {
      0x110000,       // Code point outside of valid range
      0xD800,         // Surrogate pair (low)
    };
    for (size_t i = 0; i < cntof(invalid_code_points); ++i) {
      unsigned rune = invalid_code_points[i];
      char buf[UTF_SIZ+1];
      int bytes_written = utf_enc(buf, cntof(buf), rune);
      assert(bytes_written == 0); // Ensure no bytes are written for invalid code points
    }
  }
  {
    // Test case for accessing a valid character index within a UTF-8 string
    const char* str_literal = "Hello, 世界";
    struct str str = str0(str_literal);
    // Access the second character (world)
    int idx = 1;
    unsigned rune;
    struct str result = utf_at(&rune, str, idx);
    assert(result.ptr != NULL); // Ensure a valid str is returned
    assert(!str_is_empty(str));   // Ensure str is advanced
    assert(result.rng.lo = 1);
    assert(rune == 101);       // Check decoded character (world)
  }
  {
    // Test case for accessing an index beyond the string length
    struct str str = strv("Hello");
    // Access index 5 (out of bounds)
    int idx = 5;
    unsigned rune;
    struct str result = utf_at(&rune, str, idx);
    assert(result.rng.lo == str.rng.hi);  // Ensure str remains unchanged
    assert(result.rng.hi == str.rng.hi);  // Ensure str remains unchanged
    assert(result.rng.cnt == 0);
    assert(result.ptr = str.ptr);
    assert(result.rng.total = str.rng.total);
    assert(rune == UTF_INVALID);        // Ensure UTF_INVALID is returned
  }
  {
    // Test case for accessing an index in an empty string
    struct str str = str_nil;
    // Access index 0
    int idx = 0;
    unsigned rune;
    struct str result = utf_at(&rune, str, idx);
    assert(result.ptr == str.ptr);  // Ensure str remains unchanged
    assert(rune == UTF_INVALID);    // Ensure UTF_INVALID is returned
  }
  {
    struct str str = strv("Hello, 世界");
    // Get the byte offset of the second character (world)
    int idx = 1;
    int byte_offset = utf_at_idx(str, idx);
    assert(byte_offset > 0);

    // Verify that the character at the calculated byte offset is the expected character
    struct str char_str = strptr(str.ptr + byte_offset, str.ptr + byte_offset, str.ptr + byte_offset + 1, str.rng.total);
    unsigned rune;
    struct str result = utf_dec(&rune, &char_str);
    assert(result.ptr != NULL);
    assert(rune == 101); // Check decoded character (world)
  }
  {
    const char* str_literal = "Hello";
    struct str str = str0(str_literal);
    // Access index beyond the string length
    int idx = 5;
    int byte_offset = utf_at_idx(str, idx);
    assert(byte_offset == str_len(str)); // Byte offset should be at the end of the string
  }
  {
    struct str str = str_nil;
    int idx = 0;
    int byte_offset = utf_at_idx(str, idx);
    assert(byte_offset == 0); // Byte offset should be 0 for empty string
  }
  {
    const char* str_literal = "Hello, 世界";
    struct str str = str0(str_literal);
    int length = utf_len(str);
    assert(length == 9); // Expected number of characters
  }
  {
    struct str str = str_nil;
    int length = utf_len(str);
    assert(length == 0);
  }
}

/* ---------------------------------------------------------------------------
 *                                String
 * ---------------------------------------------------------------------------
 */
static inline struct str
str_set(char *buf, int cnt, struct str str) {
  requires(buf);
  requires(cnt >= 0);
  requires(str__is_val(&str));
  if (str_is_empty(str)) {
    return strn(buf,0);
  } else if (str_len(str) > cnt) {
    return str_inv;
  }
  mcpy(buf, str_beg(str), str_len(str));
  return strn(buf, str_len(str));
}
static inline struct str
str_sqz(char *buf, int cnt, struct str str) {
  requires(buf);
  requires(cnt >= 0);
  requires(str__is_val(&str));

  int len = min(cnt, str_len(str));
  mcpy(buf, str_beg(str), len);
  return strn(buf,len);
}
static struct str
str_add(char *buf, int cap, struct str in, struct str str) {
  requires(buf);
  requires(cap >= 0);
  requires(str__is_val(&str));
  requires(str__is_val(&in));

  if (str_len(in) + str_len(str) < cap) {
    mcpy(buf + str_len(in), str_beg(str), str_len(str));
    return strn(buf, str_len(str) + str_len(in));
  }
  int nnn = 0;
  unsigned rune = 0;
  int cnt = max(0, cap - str_len(in));
  for utf_loop(&rune, itr, _, str) {
    int len = casti(str_end(itr) - str_beg(str));
    if (len > cnt) {
      break;
    }
    nnn = len;
  }
  mcpy(buf + str_len(in), str_beg(str), nnn);
  return strn(buf, str_len(in) + nnn);
}
static struct str
str_rm(char *buf, struct str in, int cnt) {
  requires(buf);
  requires(cnt >= 0);
  requires(str__is_val(&in));

  int left = max(0, str_len(in) - cnt);
  return strn(buf, left);
}
static struct str
str_put(char *buf, int cap, struct str in, int pos, struct str str) {
  requires(buf);
  requires(cap >= 0);
  requires(pos >= 0);
  requires(str__is_val(&in));
  requires(str__is_val(&str));
  if (pos >= str_len(in)) {
    return str_add(buf, cap, in, str);
  }
  if (str_len(in) + str_len(str) < cap) {
    /* string fits into buffer */
    memmove(buf + pos + str_len(str), buf + pos, castsz(str_len(in) - pos));
    mcpy(buf + pos, str_beg(str), str_len(str));
    return strn(buf, str_len(in) + str_len(str));
  } else {
    int cpy = 0;
    int cnt = cap - pos;
    unsigned rune = 0;
    for utf_loop(&rune, itr, _, str) {
      int len = str_len(itr);
      if (cpy + len > cnt) {
        break;
      }
      cpy += len;
    }
    mcpy(buf + pos, str_beg(str), cpy);
    return strn(buf, cap);
  }
}
static struct str
str_del(char *buf, struct str in, int pos, int len) {
  requires(buf);
  requires(pos >= 0);
  requires(str__is_val(&in));
  if (pos < 0 || pos >= str_len(in) || len < 0) {
    return in;
  }
  assert(pos + len <= str_len(in));
  memmove(buf + pos, buf + pos + len, castsz(str_len(in) - pos));
  return strn(buf, str_len(in) - len);
}
static struct str
str_fmtsn(char *buf, int n, const char *fmt, ...) {
  requires(buf);
  requires(n >= 0);

  int ret;
  va_list va;
  va_start(va, fmt);
  ret = fmtvsn(buf, n, fmt, va);
  va_end(va);
  return strn(buf, ret);
}
static struct str
str_add_fmt(char *buf, int cap, struct str in, const char *fmt, ...) {
  requires(buf);
  requires(cap >= 0);
  requires(fmt != 0);

  va_list va;
  va_start(va, fmt);
  int left = max(0, cap - str_len(in));
  char *dst = buf + str_len(in);
  int ret = fmtvsn(dst, left, fmt, va);
  va_end(va);
  return strn(buf, str_len(in) + ret);
}
static void
ut_str2(void) {
  {
    char buf[10] = {0};
    struct str str = strv("hello");
    struct str result = str_set(buf, sizeof(buf), str);

    assert(memcmp(buf, "hello", 5) == 0);
    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 5);
    assert(result.rng.cnt == 5);
    assert(result.rng.total == 5);
  }
  {
    char buf[10];
    struct str str = str_nil;
    struct str result = str_set(buf, sizeof(buf), str);

    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 0);
    assert(result.rng.cnt == 0);
    assert(result.rng.total == 0);
  }
  {
    char buf[3];
    struct str str = strv("hello");
    struct str result = str_set(buf, sizeof(buf), str);
    assert(result.ptr == NULL); // str_inv
  }
  {
    char buf[10];
    struct str str = strv("hello world");
    struct str result = str_sqz(buf, 5, str);

    assert(memcmp(buf, "hello", 5) == 0);
    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 5);
    assert(result.rng.cnt == 5);
    assert(result.rng.total == 5);
  }
  {
    char buf[10];
    struct str str = strv("hello world");
    struct str result = str_sqz(buf, 3, str);

    assert(memcmp(buf, "hel", 3) == 0);
    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 3);
    assert(result.rng.cnt == 3);
    assert(result.rng.total == 3);
  }
  {
    char buf[10];
    struct str str = str_nil;
    struct str result = str_sqz(buf, sizeof(buf), str);

    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 0);
    assert(result.rng.cnt == 0);
    assert(result.rng.total == 0);
  }
  {
    // Test 1: Success
    char buf[20];
    strcpy(buf, "Hello, ");
    struct str in = strn(buf, 7);
    struct str str = strv("world");
    struct str result = str_add(buf, sizeof(buf), in, str);

    assert(memcmp(buf, "Hello, world", castsz(str_len(result))) == 0);
    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 12);
    assert(result.rng.cnt == 12);
    assert(result.rng.total == 12);
  }
  {
    // Test 2: Insufficient buffer space
    char buf[8];
    strcpy(buf, "Hello, ");
    struct str in = strn(buf, 7);
    struct str str = strv("world!");
    struct str result = str_add(buf, sizeof(buf), in, str);

    assert(memcmp(buf, "Hello, w", 8) == 0);
    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 8);
    assert(result.rng.cnt == 8);
    assert(result.rng.total == 8);
  }
  {
    // Test 3: Empty input string
    char buf[20] = {0};
    strcpy(buf, "Hello, ");
    struct str in = strn(buf, 7);
    struct str str = str_nil;
    struct str result = str_add(buf, sizeof(buf), in, str);

    assert(memcmp(buf, "Hello, ", strlen("Hello, ")) == 0);
    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 7);
    assert(result.rng.cnt == 7);
    assert(result.rng.total == 7);
  }
  {
    // Test 4: Empty buffer and input string
    char buf[10];
    struct str in = strn(buf,0);
    struct str str = str_nil;
    struct str result = str_add(buf, sizeof(buf), in, str);

    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 0);
    assert(result.rng.cnt == 0);
    assert(result.rng.total == 0);
  }
  {
    // Test 1: Remove characters from the end
    char buf[10];
    strcpy(buf, "hello");
    struct str str = strn(buf, 5);
    struct str result = str_rm(buf, str, 2);

    assert(memcmp(buf, "hel", 3) == 0);
    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 3);
    assert(result.rng.cnt == 3);
    assert(result.rng.total == 3);
  }
  {
    // Test 2: Remove more characters than available
    char buf[10];
    strcpy(buf, "hello");
    struct str str = strn(buf, 5);
    struct str result = str_rm(buf, str, 10);

    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 0);
    assert(result.rng.cnt == 0);
    assert(result.rng.total == 0);
  }
  {
    // Test 3: Remove no characters
    char buf[10];
    strcpy(buf, "hello");
    struct str str = strn(buf, 5);
    struct str result = str_rm(buf, str, 0);

    assert(memcmp(buf, "hello", 5) == 0);
    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 5);
    assert(result.rng.cnt == 5);
    assert(result.rng.total == 5);
  }
  {
    // Test 4: Empty string
    char buf[10];
    struct str str = strn(buf, 0);
    struct str result = str_rm(buf, str, 0);

    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 0);
    assert(result.rng.cnt == 0);
    assert(result.rng.total == 0);
  }
  {
    // Test 1: Insert at the beginning
    char buf[20] = {0};
    strcpy(buf, "world");
    struct str in = strn(buf, 5);
    struct str str = strv("Hello, ");
    struct str result = str_put(buf, sizeof(buf), in, 0, str);

    assert(memcmp(buf, "Hello, world", castsz(str_len(result))) == 0);
    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 12);
    assert(result.rng.cnt == 12);
    assert(result.rng.total == 12);
  }
  {
    // Test 2: Insert in the middle
    char buf[20];
    strcpy(buf,"He, world!");
    struct str in = str0(buf);
    struct str str = strv("llo");
    struct str result = str_put(buf, sizeof(buf), in, 2, str);

    assert(memcmp(buf, "Hello, world!", castsz(str_len(result))) == 0);
    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 13);
    assert(result.rng.cnt == 13);
    assert(result.rng.total == 13);
  }
  {
    // Test 3: Insert at the end (delegates to str_add)
    char buf[20];
    strcpy(buf,"Hello, ");
    struct str in = str0(buf);
    struct str str = strv("world");
    struct str result = str_put(buf, sizeof(buf), in, 7, str);

    assert(memcmp(buf, "Hello, world", castsz(str_len(result))) == 0);
    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 12);
    assert(result.rng.cnt == 12);
    assert(result.rng.total == 12);
  }
  {
    // Test 4: Insufficient buffer space
    char buf[5] = {0};
    strcpy(buf, "Hell");
    struct str in = str0(buf);
    struct str str = strv(" world!");
    struct str result = str_put(buf, sizeof(buf), in, 2, str);

    assert(memcmp(buf, "He wo", 5) == 0);
    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 5);
    assert(result.rng.cnt == 5);
    assert(result.rng.total == 5);
  }
  {
    // Test 5: Invalid insertion position
    char buf[5] = {0};
    strcpy(buf, "Hell");
    struct str in = str0(buf);
    struct str str = strv(" world!");
    struct str result = str_put(buf, sizeof(buf), in, 10, str);

    assert(memcmp(buf, "Hell", 4) == 0);
    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 5);
    assert(result.rng.cnt == 5);
    assert(result.rng.total == 5);
  }
  {
    // Test 6: Empty input string
    char buf[20];
    strcpy(buf, "Hello, ");
    struct str in = str0(buf);
    struct str str = str_nil;
    struct str result = str_put(buf, sizeof(buf), in, 2, str);

    assert(memcmp(buf, "Hello, ", strlen("Hello, ")) == 0);
    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 7);
    assert(result.rng.cnt == 7);
    assert(result.rng.total == 7);
  }
  {
    // Test 1: Delete from the beginning
    char buf[20];
    strcpy(buf, "hello world");
    struct str str = str0(buf);
    struct str result = str_del(buf, str, 0, 2);

    assert(memcmp(buf, "llo world", strlen("llo world")) == 0);
    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 9);
    assert(result.rng.cnt == 9);
    assert(result.rng.total == 9);
  }

  {
    // Test 2: Delete from the middle
    char buf[20];
    strcpy(buf, "hello world");
    struct str str = str0(buf);
    struct str result = str_del(buf, str, 5, 3);

    assert(memcmp(buf, "hellorld", strlen("hellorld")) == 0);
    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 8);
    assert(result.rng.cnt == 8);
    assert(result.rng.total == 8);
  }

  {
    // Test 3: Delete from the end
    char buf[20];
    strcpy(buf, "hello world");
    struct str str = str0(buf);
    struct str result = str_del(buf, str, 7, 3);

    assert(memcmp(buf, "hello wd", strlen("hello wd")) == 0);
    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 8);
    assert(result.rng.cnt == 8);
    assert(result.rng.total == 8);
  }

  {
    // Test 4: Invalid position (out of bounds)
    char buf[20];
    strcpy(buf, "hello world");
    struct str str = str0(buf);
    struct str result = str_del(buf, str, 15, 3);

    assert(memcmp(buf, "hello world", strlen("hello world")) == 0);
    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 11);
    assert(result.rng.cnt == 11);
    assert(result.rng.total == 11);
  }

  {
    // Test 5: Invalid length (negative)
    char buf[20];
    strcpy(buf, "hello world");
    struct str str = str0(buf);
    struct str result = str_del(buf, str, 5, -1);

    assert(memcmp(buf, "hello world", strlen("hello world")) == 0);
    assert(result.ptr == buf);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 11);
    assert(result.rng.cnt == 11);
    assert(result.rng.total == 11);
  }

  {
    // Test 6: Empty string
    char buf[20];
    struct str str = str_nil;
    struct str result = str_del(buf, str, 0, 0);

    assert(result.ptr == 0);
    assert(result.rng.lo == 0);
    assert(result.rng.hi == 0);
    assert(result.rng.cnt == 0);
    assert(result.rng.total == 0);
  }
}

/* ---------------------------------------------------------------------------
 *                                Time
 * ---------------------------------------------------------------------------
 */
#define time_inf      (9223372036854775807ll)
#define time_ninf     (-9223372036854775807ll)
#define time_ns(ns)   castll((ns))
#define time_us(us)   (castll((us))*1000ll)
#define time_ms(ms)   (castll((ms))*1000000ll)
#define time_sec(s)   (castll((s))*1000000000ll)
#define time_min(m)   (castll((m))*60000000000ll)
#define time_hours(h) (castll((m))*3600000000000ll)

#define time_flt_sec(s)   (castll(castd((s))*1000000000.0))
#define time_flt_min(s)   (castll(castd((s))*60000000000.0))
#define time_flt_hour(s)  (castll(castd((s))*3600000000000.0))

#define time_30fps  time_ns(33333333)
#define time_60fps  time_ns(16666666)
#define time_90fps  time_ns(11111111)
#define time_120fps time_ns(8333333)
#define time_144fps time_ns(6944444)
#define time_240fps time_ns(4166667)

#define ns_time(t)    (t)
#define us_time(t)    ((t)/1000ll)
#define ms_time(t)    ((t)/1000000ll)
#define sec_time(t)   ((t)/1000000000ll)
#define min_time(t)   ((t)/60000000000ll)
#define hour_time(t)  ((t)/3600000000000ll)

#define sec_flt_time(t)   castf(castd(t)*(1.0/1000000000.0))
#define min_flt_time(t)   castf(castd(t)*(1.0/10000000000.0))
#define hour_flt_time(t)  castf(castd(t)*(1.0/3600000000000.0))

static purist long long
time_sub(long long a, long long b) {
  if (a == time_inf) {
    return (b == time_inf) ? 0ll : time_inf;
  } else if (a == time_ninf) {
    return (b == time_ninf) ? 0ll : time_ninf;
  } else if (b == time_inf) {
    return time_ninf;
  } else if (b == time_ninf) {
    return time_inf;
  }
  return a - b;
}
static purist long long
time_add(long long a, long long b) {
  if (a == time_inf) {
    return (b == time_ninf) ? 0ll : time_inf;
  } else if (a == time_ninf) {
    return (b == time_inf) ? 0ll : time_ninf;
  } else if (b == time_inf) {
    return time_inf;
  } else if (b == time_ninf) {
    return time_ninf;
  }
  return a + b;
}
static long long
time_neg(long long a) {
  if (a == time_inf) {
    return time_ninf;
  } else if (a == time_ninf) {
    return time_inf;
  } else {
    return -a;
  }
}
static long long
time_mul(long long a, long long b) {
  return a * b;
}
static long long
time_div(long long a, long long b) {
  if (a == time_inf) {
    if (b == time_inf) {
      return 1ll;
    } else if (b == time_ninf){
      return -1ll;
    } else {
      return time_inf;
    }
  } else if (a == time_ninf) {
    if (b == time_inf) {
      return -1ll;
    } else if (b == time_ninf){
      return 1ll;
    } else {
      return time_ninf;
    }
  } else if (b == time_inf) {
    return time_inf;
  } else if (b == time_ninf) {
    return time_ninf;
  }
  return a / b;
}
static long long
time_mod(long long a, long long b) {
  assert(b != 0ll);
  if (a == time_inf || a == time_ninf) {
    return a;
  } else if (b == time_inf || b == time_ninf) {
    return 0;
  } else {
    return a % b;
  }
}

/* ---------------------------------------------------------------------------
 *                                  Path
 * ---------------------------------------------------------------------------
 */
static void
path_norm(char *path) {
  requires(path);
  char *ptr = path;
  for (; *ptr; ptr++) {
    if (*ptr == '\\') {
      *ptr = '/';
    }
  }
  if (ptr != path && ptr[-1] == '/') {
    ptr[-1] = 0;
  }
}
static struct str
path_file(struct str path) {
  requires(str__is_val(&path));
  for (const char *p = str_end(path); p > str_beg(path); --p) {
    if (p[-1] == '/') {
      return strptr(path.ptr, p, str_end(path), path.rng.total);
    }
  }
  ensures(str__is_val(&path));
  return path;
}
static struct str
path_ext(struct str path) {
  requires(str__is_val(&path));
  for (const char *p = str_end(path); p > str_beg(path); --p) {
    if (p[-1] == '.') {
      return strptr(path.ptr, p, str_end(path), path.rng.total);
    }
  }
  return str_nil;
}

/* ---------------------------------------------------------------------------
 *                              String-Buffer
 * ---------------------------------------------------------------------------
 */
// clang-format off
#define str_buf_push(b,s) str_buf__push(&(b)->cnt, (b)->mem, cntof((b)->mem), s)
#define str_buf_sqz(b,s,m) str_buf__sqz(&(b)->cnt, (b)->mem, cntof((b)->mem), s, m)
#define str_buf_get(b,h) str_buf__get((b)->mem, (b)->cnt, h)
#define str_buf_clr(b) str_buf__clr((b)->mem, &(b)->cnt)
#define str_buf_off(hdl) casti(((hdl) >> 16u) & 0xffff)
#define str_buf_len(hdl) casti((hdl) & 0xffff)
// clang-format on

static unsigned
str_buf__push(int *cnt, char *mem, int cap, struct str s) {
  requires(cnt);
  requires(*cnt >= 0);
  requires(*cnt <= 0xffff);
  requires(*cnt <= cap);
  requires(cap >= 0);
  requires(mem);
  requires(cap);

  unsigned off = castu(*cnt) & 0xffff;
  int lft = max(0, cap - *cnt);
  char *dst = mem + *cnt;
  struct str p = str_set(dst, lft, s);
  int n = str_is_val(p) * str_len(p);
  unsigned ret = (off << 16u)|(n & 0xffff);
  *cnt += n;
  return ret;
}
static unsigned
str_buf__sqz(int *cnt, char *mem, int cap, struct str s, int max_len) {
  requires(cnt);
  requires(cap <= 0x10000);
  requires(*cnt >= 0);
  requires(*cnt <= 0x10000);
  requires(*cnt <= cap);
  requires(cap >= 0);
  requires(*cnt <= cap);
  requires(mem);
  requires(cap);

  unsigned off = castu(*cnt) & 0xffff;
  int lft = min(max_len, max(0, cap - *cnt));
  char *dst = mem + *cnt;
  struct str p = str_sqz(dst, lft, s);
  *cnt += str_len(p);
  unsigned ret = (off << 16u)|(str_len(p) & 0xffff);
  return ret;
}
static purist struct str
str_buf__get(char *mem, int cnt, unsigned hdl) {
  requires(mem);
  requires(cnt >= 0);
  requires(str_buf_len(hdl) <= cnt);
  requires(str_buf_off(hdl) <= cnt);
  requires(str_buf_len(hdl) + str_buf_off(hdl) <= cnt);

  int off = str_buf_off(hdl);
  int len = str_buf_len(hdl);
  return strn(mem + off, len);
}
static void
str_buf__clr(char *mem, int *cnt) {
  requires(mem);
  requires(cnt);
  requires(*cnt >= 0);
  *cnt = 0;
}

/* ---------------------------------------------------------------------------
 *                                TABLE
 * ---------------------------------------------------------------------------
 */
// clang-format off
#define tbl__is_del(k) (((k) >> 63llu) != 0)
#define tbl__dist(h,n,i) (((i) + (n) - ((h) % n)) % (n))
#define tbl__key(k) ((k) != 0u && !tbl__is_del(k))
#define tbl__loop(n,i,t,cap) (int n = tbl__nxt_idx(t,cap,0), i = 0; n < cap && i < cap; n = tbl__nxt_idx(t,cap,n+1),++i)
#define tbl__clr(s,cnt,cap) do {mset(s,0,szof(unsigned long long)*cap); *cnt = 0;} while(0)

#define tbl_fnd(t, k) tbl__fnd((t)->keys, cntof((t)->keys), k)
#define tbl_del(t, k) tbl__del((t)->keys, cntof((t)->keys), &(t)->cnt, k)
#define tbl_clr(t) tbl__clr((t)->keys, &(t)->cnt, cntof((t)->keys))
#define tbl_loop(n,i,t) tbl__loop(n,i,(t)->keys,cntof((t)->keys))
#define tbl_val(t,i) ((i) < cntof((t)->keys))
#define tbl_inval(t,i) (!tbl_val(t,i))
#define tbl_get(t,i) (tbl_val(t,i) ? ((t)->vals + (i)): 0)
#define tbl_unref(t,i,d) (tbl_val(t,i) ? (t)->vals[i] : (d))
#define tbl_has(t,k) tbl_val(t,tbl_fnd(t,k))
#define tbl_put(t, k, v) do {\
  assert(szof(*(v)) == szof((t)->vals[0]));\
  tbl__put((t)->keys,(t)->vals,&(t)->cnt,cntof((t)->keys),k,v,szof((t)->vals[0]));\
} while(0)
// clang-format on

static purist force_inline unsigned long long
tbl__hash(unsigned long long key) {
  unsigned long long hsh = key & 0x7fffffffffffffffllu;
  return hsh | (hsh == 0);
}
static force_inline void
tbl__swap(void *restrict a, void *restrict b, int siz) {
  requires(siz <= 64);
  char tmp[64];
  mcpy(tmp, a, siz);
  mcpy(a, b, siz);
  mcpy(b, tmp, siz);
}
static force_inline long long
tbl__store(unsigned long long *keys, void *vals, int *cnt, int cap,
           unsigned long long idx, unsigned long long hsh,
           void* val, int val_siz) {

  unused(cap);
  assert(cnt);
  assert(keys);

  keys[idx] = hsh;
  if (vals) {
    unsigned long long off = (unsigned long long)val_siz * idx;
    mcpy((unsigned char*)vals + off, val, val_siz);
  }
  *cnt += 1;
  return castll(idx);
}
static long long
tbl__put(unsigned long long *keys, void *vals, int *cnt, int cap,
         unsigned long long key, void *val, int val_siz) {

  assert(cnt);
  assert(keys);
  if (*cnt >= cap) {
    return castll(cap);
  }
  unsigned long long hsh = tbl__hash(key);
  unsigned long long siz = castull(cap);
  unsigned long long idx = hsh % siz;
  unsigned long long beg = idx;
  unsigned long long dist = 0;
  do {
    unsigned long long slot = keys[idx];
    if (!slot) {
      return tbl__store(keys, vals, cnt, cap, idx, hsh, val, val_siz);
    }
    if (tbl__is_del(slot)) {
      return tbl__store(keys, vals, cnt, cap, idx, hsh, val, val_siz);
    }
    unsigned long long d = tbl__dist(slot, siz, idx);
    if (d++ > dist++) {
      continue;
    }
    iswap(hsh, keys[idx]);
    if (vals) {
      void *cur_val = (unsigned char*)vals + idx * (unsigned long long)val_siz;
      tbl__swap(cur_val, val, val_siz);
    }
    dist = d;
  } while ((idx = ((idx + 1) % siz)) != beg);
  return castll(siz);
}
static purist int
tbl__fnd(unsigned long long *set, int cap, unsigned long long key) {
  assert(set);
  unsigned long long h = tbl__hash(key);
  unsigned long long n = castull(cap);
  unsigned long long i = h % n, b = i, dist = 0;
  do {
    if (!set[i] || dist > tbl__dist(set[i],n,i)) {
      return cap;
    } else if(set[i] == h) {
      return casti(i);
    }
    dist++;
  } while ((i = ((i + 1) % n)) != b);
  return cap;
}
static int
tbl__del(unsigned long long* set, int cap, int *cnt, unsigned long long key) {
  assert(set);
  if (!cap) {
    return cap;
  }
  int i = tbl__fnd(set, cap, key);
  if (i < cap) {
    set[i] |= 0x8000000000000000llu;
    *cnt -= 1;
  }
  return i;
}
static purist int
tbl__nxt_idx(unsigned long long *keys, int cap, int i) {
  assert(keys);
  for (; i < cap; ++i) {
    if (tbl__key(keys[i])) {
      return i;
    }
  }
  return cap;
}
static void
ut_tbl(void) {
  typedef struct {
    int id;
    char name[32];
  } Person;
  {
    // 1. Normal expected inputs (int values)
    struct tbl(int, 16) table_int = {0};
    tbl_clr(&table_int);

    assert(table_int.cnt == 0);

    tbl_put(&table_int, 10, &(int){100});
    tbl_put(&table_int, 20, &(int){200});
    tbl_put(&table_int, 30, &(int){300});

    assert(table_int.cnt == 3);
    assert(tbl_has(&table_int, 10));
    assert(tbl_has(&table_int, 20));
    assert(tbl_has(&table_int, 30));
    assert(!tbl_has(&table_int, 40));

    int *val = tbl_get(&table_int, tbl_fnd(&table_int, 20));
    assert(val != 0 && *val == 200);

    tbl_del(&table_int, 20);
    assert(!tbl_has(&table_int, 20));
    assert(table_int.cnt == 2);

    tbl_clr(&table_int);
    assert(table_int.cnt == 0);
  }
  {
    // 2. Normal expected inputs (Person struct values)
    struct tbl(Person, 8) table_person = {0};
    tbl_clr(&table_person);

    Person p1 = {1, "Alice"};
    Person p2 = {2, "Bob"};
    tbl_put(&table_person, 1, &p1);
    tbl_put(&table_person, 2, &p2);

    assert(tbl_has(&table_person, 1));
    assert(tbl_has(&table_person, 2));

    int itr = tbl_fnd(&table_person, 2);
    Person *found_person = (Person *)tbl_get(&table_person, itr);
    assert(found_person != 0);
    {
      assert((found_person->id - p2.id) == 0);
      assert(!strcmp(found_person->name, p2.name));
    }
  }
  {
    // 3. Edge cases (full table, collisions, deletions)
    struct tbl(int, 4) table_edge = {0};
    tbl_clr(&table_edge);

    tbl_put(&table_edge, 2, &(int){1});
    tbl_put(&table_edge, 5, &(int){5}); // Collision with 1 (assuming a simple modulo hash)
    tbl_put(&table_edge, 9, &(int){9}); // Collision with 1 and 5
    tbl_put(&table_edge, 13, &(int){13}); // Collision with 1, 5 and 9
    tbl_put(&table_edge, 27, &(int){27}); // Collision with 1, 5 and 9

    assert(table_edge.cnt == 5);
    assert(tbl_has(&table_edge, 2));
    assert(tbl_has(&table_edge, 5));
    assert(tbl_has(&table_edge, 9));
    assert(tbl_has(&table_edge, 13));
    assert(tbl_has(&table_edge, 27));

    // Try to insert when full (should fail gracefully)
    long long result = tbl__put(table_edge.keys, table_edge.vals, &table_edge.cnt, cntof(table_edge.keys), 17, &(int){17}, szof(int));
    assert(result == cntof(table_edge.keys)); // Check for "table full" return value
    assert(table_edge.cnt == 5); // Count should not have changed

    tbl_del(&table_edge, 5);
    assert(!tbl_has(&table_edge, 5));
    assert(table_edge.cnt == 4);

    result = tbl__put(table_edge.keys, table_edge.vals, &table_edge.cnt, cntof(table_edge.keys), 17, &(int){17}, szof(int));
    assert(result < cntof(table_edge.keys)); // Check for "table full" return value
    assert(table_edge.cnt == 5); // Count should not have changed
  }
  {
    // 5. More Comprehensive Collision Testing (important)
    struct tbl(int, 16) collision_table = {0};
    tbl_clr(&collision_table);
    for (int i = 0; i < 16; i++) {
      unsigned long long key = castull(i) * 16;
      tbl_put(&collision_table, key, &(int){i}); // All keys will have the same hash.
    }
    assert(collision_table.cnt == 16);
    for (int i = 0; i < 16; i++) {
      unsigned long long key = castull(i) * 16;
      assert(tbl_has(&collision_table, key));
    }
    for (int i = 0; i < 16; i++) {
      unsigned long long key = castull(i * 16);
      int itr = tbl_fnd(&collision_table, key);
      int *val = tbl_get(&collision_table, itr);
      assert(val != 0 && *val == i);
    }
  }
  {
    // 6. Test with different value sizes
    struct tbl(long long, 16) long_table = {0};
    tbl_clr(&long_table);
    tbl_put(&long_table, 1, &(long long){100});
    assert(tbl_has(&long_table, 1));
    int itr = tbl_fnd(&long_table, 1);
    long long *long_val = tbl_get(&long_table, itr);
    assert(long_val != 0 && *long_val == 100);
  }
}

