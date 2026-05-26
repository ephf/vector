// assumes NULL == 0, __builtin_clzll exists
#ifndef vector

#include <stddef.h>
#include <stdint.h>

#ifndef VECDEF
#define VECDEF extern inline
#endif

#define vector(t) t*

struct vector { size_t s, c; unsigned char a[]; };

VECDEF const struct vector* vinf(const void* v);
VECDEF struct vector* voff(const void* v);
VECDEF void* __resv(struct vector* restrict* restrict v, size_t n, size_t s);
VECDEF void* __push(struct vector* restrict* restrict v, const void* restrict b, size_t n,
		size_t s);
VECDEF void* __pop(struct vector* restrict* restrict v, size_t s);

#define resv(v, n) ((typeof(*(v))) __resv((void*)(v), (n), sizeof **(v)))
#define push(v, ...) ((typeof(*(v))) __push((void*)(v), (typeof(**(v))[]){__VA_ARGS__}, \
			sizeof((typeof(**(v))[]){__VA_ARGS__}) / sizeof **(v), sizeof **(v)))
#define pop(v) ((typeof(*(v))) __pop((void*)(v), sizeof **(v)))

#define vec(a, ...) push(&(typeof(a)*){0}, a, __VA_ARGS__)
#define nullvec NULL
#define len(v) (vinf(v)->s)

#if !defined(VECSETIMPL) || defined(VECIMPL)

#include <stdlib.h>
#include <string.h>
#include <errno.h>

VECDEF const struct vector* vinf(const void* v) {
	static const struct vector e = {0};
	return (const struct vector*)((uintptr_t) (&e + 1) * !v + (unsigned char*) v) - 1;
}

VECDEF struct vector* voff(const void* v) {
	return (struct vector*) v - !!v;
}

VECDEF void* __resv(struct vector* restrict* restrict v, size_t n, size_t s) {
	struct vector h = *vinf(*v);
	if((n = (n + h.s) * s) > h.c) {
		h.c = (1ull << (64 - __builtin_clzll(h.c + sizeof(size_t[4]) - 1))) 
			- sizeof(size_t[4]);
		if(!(*v = realloc(voff(*v), h.c + sizeof(struct vector)))) return nullvec;
		*(*v)++ = h;
	}
	return *v;
}

VECDEF void* __push(struct vector* restrict* restrict v, const void* restrict b, size_t n,
		size_t s) {
	if(!__resv(v, n, s)) return nullvec;
	memcpy((unsigned char*) *v + (*v)[-1].s * s, b, n * s);
	(*v)[-1].s += n;
	return *v;
}

VECDEF void* __pop(struct vector* restrict* restrict v, size_t s) {
	if(!vinf(*v)->s) return errno = EDOM, NULL;
	return (unsigned char*) *v + --(*v)[-1].s * s;
}

#endif

#endif
