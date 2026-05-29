// assumes __builtin_clzll exists
#ifndef vector

#include <stddef.h>
#include <stdint.h>

#ifndef VECDEF
#define VECDEF extern inline
#endif

#define vector(t) t*

struct vector { 
	size_t size, byte_capacity; 
	unsigned char data[];
};

VECDEF const struct vector* vinf(const void* vector);
VECDEF struct vector* voff(const void* vector);
VECDEF void* __resv(struct vector* restrict* restrict vector, size_t n, size_t stride);
VECDEF void* __push(struct vector* restrict* restrict vector, const void* restrict src, 
		size_t n, size_t stride);
VECDEF void* __pop(struct vector* restrict* restrict vector, size_t stride);

#define resv(vector, n) ((typeof(*(vector))) __resv((void*)(vector), (n), sizeof **(vector)))
#define push(vector, ...) ((typeof(*(vector))) __push((void*)(vector), \
			(typeof(**(vector))[]){__VA_ARGS__}, \
			sizeof((typeof(**(vector))[]){__VA_ARGS__}) / sizeof **(vector), \
			sizeof **(vector)))
#define pop(vector) ((typeof(*(vector))) __pop((void*)(vector), sizeof **(vector)))

#define vec(a, ...) push(&(typeof(a)*){0}, a, __VA_ARGS__)
#define nullvec ((void*) 0)
#define len(vector) (vinf(vector)->size)

#if !defined(VECSETIMPL) || defined(VECIMPL)

#include <stdlib.h>
#include <string.h>
#include <errno.h>

VECDEF const struct vector* vinf(const void* vector) {
	static const struct vector empty = {0};
	return (const struct vector*)((uintptr_t) (&empty + 1) * !vector 
			+ (unsigned char*) vector) - 1;
}

VECDEF struct vector* voff(const void* vector) {
	return (struct vector*) vector - !!vector;
}

VECDEF void* __resv(struct vector* restrict* restrict vector, size_t n, size_t stride) {
	struct vector info = *vinf(*vector);
	if((n = (n + info.size) * stride) > info.byte_capacity) {
		info.byte_capacity = 
			(1ull << (64 - __builtin_clzll(info.byte_capacity + sizeof(size_t[4]) - 1))) 
			- sizeof(size_t[4]);
		if(!(*vector = realloc(voff(*vector), info.byte_capacity + sizeof(struct vector)))) 
			return nullvec;
		*(*vector)++ = info;
	}
	return *vector;
}

VECDEF void* __push(struct vector* restrict* restrict vector, const void* restrict src, 
		size_t n, size_t stride) {
	if(!__resv(vector, n, stride)) return nullvec;
	memcpy((unsigned char*) *vector + (*vector)[-1].size * stride, src, n * stride);
	(*vector)[-1].size += n;
	return *vector;
}

VECDEF void* __pop(struct vector* restrict* restrict vector, size_t stride) {
	if(!vinf(*vector)->size) return errno = EDOM, NULL;
	return (unsigned char*) *vector + --(*vector)[-1].size * stride;
}

#endif

#endif
