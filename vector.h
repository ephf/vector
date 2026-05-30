// assumes __builtin_clzll exists
#ifndef vector

#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

#ifndef VECDEF
#define VECDEF extern inline
#endif

#define vector(t) t*

struct vector { 
	size_t size, byte_capacity; 
	unsigned char data[];
};

VECDEF const struct vector* __vinf(const void* vector, size_t align);
VECDEF struct vector* __voff(const void* vector, size_t align);
VECDEF struct vector* __voff_u(const void* vector, size_t align);
VECDEF void* __resv(struct vector* restrict* restrict vector, size_t n, size_t size,
		size_t align);
VECDEF void* __push(struct vector* restrict* restrict vector, const void* restrict src, 
		size_t n, size_t size, size_t align);
VECDEF void* __pop(struct vector* restrict* restrict vector, size_t size, size_t align);

#define vinf(vector) __vinf((vector), alignof(typeof(*(vector))))
#define voff(vector) __voff((vector), alignof(typeof(*(vector))))
#define voff_u(vector) __voff_u((vector), alignof(typeof(*(vector)))

#define resv(vector, n) ((typeof(*(vector))) __resv((void*)(vector), (n), \
			sizeof **(vector), alignof(typeof(**(vector)))))
#define push(vector, ...) ((typeof(*(vector))) __push((void*)(vector), \
			(typeof(**(vector))[]){__VA_ARGS__}, \
			sizeof((typeof(**(vector))[]){__VA_ARGS__}) / sizeof **(vector), \
			sizeof **(vector), alignof(typeof(**(vector)))))
#define pop(vector) ((typeof(*(vector))) __pop((void*)(vector), sizeof **(vector), \
			alignof(typeof(**(vector)))))

#define vec(a, ...) push(&(typeof(a)*){0}, a, __VA_ARGS__)
#define nullvec ((void*) 0)
#define len(vector) (vinf(vector)->size)

#if !defined(VECSETIMPL) || defined(VECIMPL)

#include <stdlib.h>
#include <string.h>
#include <errno.h>

static inline size_t __vpadding(size_t size, size_t align) {
	return size + (align - (size % align)) % align;
}

VECDEF const struct vector* __vinf(const void* vector, size_t align) {
	static const struct vector empty = {0};

	const size_t info_padding = __vpadding(sizeof(struct vector), align);
	void* const static_empty_data = (unsigned char*) &empty + info_padding;

	return (const struct vector*)((uintptr_t) static_empty_data * !vector
			+ (unsigned char*) vector - info_padding);
}

VECDEF struct vector* __voff(const void* vector, size_t align) {
	const size_t info_padding = __vpadding(sizeof(struct vector), align);
	return (struct vector*)((unsigned char*) vector - (!!vector * info_padding));
}

VECDEF struct vector* __voff_u(const void* vector, size_t align) {
	const size_t info_padding = __vpadding(sizeof(struct vector), align);
	return (struct vector*)((unsigned char*) vector - info_padding);
}

static inline size_t __vbitceil64(size_t x) {
	return 1ull << (64 - __builtin_clzll(x - 1));
}

VECDEF void* __resv(struct vector* restrict* restrict vector, size_t n, size_t size,
		size_t align) {
	struct vector info = *__vinf(*vector, align);
	const size_t required_byte_size = (n + info.size) * __vpadding(size, align);

	if(required_byte_size > info.byte_capacity) {
		const size_t info_padding = __vpadding(sizeof(struct vector), align);
		const size_t expected_padding = 
			__vpadding(sizeof(size_t[2]), alignof(struct vector)) + info_padding;
		info.byte_capacity = 
			__vbitceil64(required_byte_size + expected_padding) - expected_padding;

		*vector = realloc(__voff(*vector, align), info_padding + info.byte_capacity);
		if(!*vector) return nullvec;

		**vector = info;
		*vector = (void*)((unsigned char*) *vector + info_padding);
	}
	return *vector;
}

VECDEF void* __push(struct vector* restrict* restrict vector, const void* restrict src, 
		size_t n, size_t size, size_t align) {
	if(!__resv(vector, n, size, align)) return nullvec;
	
	struct vector* info = __voff_u(*vector, align);
	const size_t element_padding = __vpadding(size, align);

	memcpy((unsigned char*) *vector + info->size * element_padding, src, 
			n * element_padding);

	info->size += n;
	return *vector;
}

VECDEF void* __pop(struct vector* restrict* restrict vector, size_t size, size_t align) {
	struct vector* info = (void*) __vinf(*vector, align);

	if(!info->size) {
		errno = EDOM;
		return NULL;
	}

	return (unsigned char*) *vector + --info->size * __vpadding(size, align);
}

#endif

#endif
