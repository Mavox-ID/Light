#pragma once
// Реализации libc-функций для LightOS (нет стандартной libc).
// Включай ТОЛЬКО из одного .cpp файла, ПЕРЕД любыми сторонними библиотеками.

extern "C" {

// ── string.h ─────────────────────────────────────────────────────────────────

void *memcpy(void *dst, const void *src, size_t n) {
	EsMemoryCopy(dst, src, n); return dst;
}
void *memset(void *dst, int c, size_t n) {
	uint8_t *p = (uint8_t *)dst;
	while (n--) *p++ = (uint8_t)c;
	return dst;
}
void *memmove(void *dst, const void *src, size_t n) {
	if (!n) return dst;
	const uint8_t *s = (const uint8_t *)src;
	uint8_t *d = (uint8_t *)dst;
	if (d < s) { while (n--) *d++ = *s++; }
	else { d += n; s += n; while (n--) *--d = *--s; }
	return dst;
}
int memcmp(const void *a, const void *b, size_t n) {
	return (int)EsMemoryCompare(a, b, n);
}
void *memchr(const void *s, int c, size_t n) {
	const uint8_t *p = (const uint8_t *)s;
	while (n--) { if (*p == (uint8_t)c) return (void *)p; p++; }
	return nullptr;
}
size_t strlen(const char *s) {
	const char *p = s; while (*p) p++; return p - s;
}

// ── stdlib.h ──────────────────────────────────────────────────────────────────
// malloc/free/realloc хранят размер в скрытом заголовке для поддержки realloc.

void *malloc(size_t size) {
	if (!size) size = 1;
	size_t *p = (size_t *)EsHeapAllocate(size + sizeof(size_t), false);
	if (!p) return nullptr;
	*p = size;
	return p + 1;
}
void free(void *ptr) {
	if (ptr) EsHeapFree((size_t *)ptr - 1);
}
void *realloc(void *ptr, size_t newSize) {
	if (!ptr)    return malloc(newSize);
	if (!newSize){ free(ptr); return nullptr; }
	size_t oldSize = *((size_t *)ptr - 1);
	void *newPtr = malloc(newSize);
	if (newPtr)
		EsMemoryCopy(newPtr, ptr, oldSize < newSize ? oldSize : newSize);
	free(ptr);
	return newPtr;
}
void *calloc(size_t count, size_t size) {
	size_t total = count * size;
	void *p = malloc(total);
	if (p) memset(p, 0, total);
	return p;
}

int abs(int x)   { return x < 0 ? -x : x; }
long labs(long x){ return x < 0 ? -x : x; }

// Простой qsort (insertion sort — достаточно для таблиц minimp3/stb).
void qsort(void *base, size_t n, size_t sz,
		int (*cmp)(const void *, const void *)) {
	if (n < 2) return;
	uint8_t *b = (uint8_t *)base;
	uint8_t *tmp = (uint8_t *)malloc(sz);
	if (!tmp) return;
	for (size_t i = 1; i < n; i++) {
		EsMemoryCopy(tmp, b + i * sz, sz);
		size_t j = i;
		while (j > 0 && cmp(b + (j-1)*sz, tmp) > 0) {
			EsMemoryCopy(b + j*sz, b + (j-1)*sz, sz);
			j--;
		}
		EsMemoryCopy(b + j*sz, tmp, sz);
	}
	free(tmp);
}

// assert — no-op, но с __noreturn чтобы не было warning.
__attribute__((noreturn))
void __assert_fail(const char *, const char *, unsigned int, const char *) {
	for (;;) {} // никогда не возвращаемся
}

// ── math.h — float (используем x87 FPU напрямую) ─────────────────────────────

float sinf(float x)  { float r; asm volatile("fsin":"=t"(r):"0"(x)); return r; }
float cosf(float x)  { float r; asm volatile("fcos":"=t"(r):"0"(x)); return r; }
float sqrtf(float x) { float r; asm volatile("fsqrt":"=t"(r):"0"(x)); return r; }
float fabsf(float x) { float r; asm volatile("fabs":"=t"(r):"0"(x)); return r; }

// ldexpf(x, n) = x * 2^n — через бит-манипуляцию мантиссы/экспоненты.
float ldexpf(float x, int n) {
	union { float f; uint32_t u; } v;
	v.f = x;
	int e = (int)((v.u >> 23) & 0xFF) + n;
	if (e <= 0)   return 0.0f;
	if (e >= 255) return v.f < 0 ? -__builtin_inff() : __builtin_inff();
	v.u = (v.u & 0x807FFFFF) | ((uint32_t)e << 23);
	return v.f;
}

// expf(x): e^x = 2^(x * log2e), реализовано через f2xm1 + fscale.
float expf(float x) {
	float result;
	asm volatile(
		"fldl2e\n\t"    // st0 = log2(e)
		"fmulp\n\t"     // st0 = x * log2(e)
		"fld %%st(0)\n\t"
		"frndint\n\t"   // st0 = round(x*log2e), st1 = x*log2e
		"fxch\n\t"      // swap
		"fsub %%st(1)\n\t" // st0 = frac, st1 = int
		"f2xm1\n\t"     // st0 = 2^frac - 1
		"fld1\n\t"
		"faddp\n\t"     // st0 = 2^frac
		"fscale\n\t"    // st0 = 2^frac * 2^int = 2^(x*log2e)
		"fstp %%st(1)\n\t"
		: "=t"(result)
		: "0"(x)
	);
	return result;
}

// logf(x): ln(x) = log2(x) / log2(e) = log2(x) * ln(2).
float logf(float x) {
	float result;
	asm volatile(
		"fldln2\n\t"  // st0 = ln(2)
		"fxch\n\t"    // st0 = x, st1 = ln(2)
		"fyl2x\n\t"   // st0 = ln(2) * log2(x) = ln(x)
		: "=t"(result)
		: "0"(x)
	);
	return result;
}

// powf(x, y) = e^(y * ln(x)).
float powf(float base, float exp_) {
	if (base <= 0.0f) return 0.0f;
	return expf(exp_ * logf(base));
}

// Двойные версии (нужны cmath).
double sin(double x)  { double r; asm volatile("fsin":"=t"(r):"0"(x)); return r; }
double cos(double x)  { double r; asm volatile("fcos":"=t"(r):"0"(x)); return r; }
double sqrt(double x) { double r; asm volatile("fsqrt":"=t"(r):"0"(x)); return r; }
double fabs(double x) { double r; asm volatile("fabs":"=t"(r):"0"(x)); return r; }
double floor(double x){ double r; asm volatile("frndint":"=t"(r):"0"(x)); return r; }
double ceil(double x) { return -floor(-x); }
double ldexp(double x, int n) { return (double)ldexpf((float)x, n); }
double exp(double x)  { return (double)expf((float)x); }
double log(double x)  { return (double)logf((float)x); }
double log2(double x) { double r; asm volatile("fld1\n\tfxch\n\tfyl2x":"=t"(r):"0"(x)); return r; }
float  log2f(float x) { return (float)log2((double)x); }
double pow(double b, double e_) { return (double)powf((float)b,(float)e_); }
double tan(double x)  { double r; asm volatile("fptan\n\tfstp %%st(0)":"=t"(r):"0"(x)); return r; }
float  tanf(float x)  { return (float)tan((double)x); }

} // extern "C"
