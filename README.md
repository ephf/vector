# `vector`

Small vector implementation in C.

```c
#include "vector.h"
#include <assert.h>

int main(void) {
    vector(int) numbers = vec(1, 2, 3);

    resv(&numbers, 3);

    push(&numbers, 4, 5, 6);
    pop(&numbers);

    assert(len(numbers) == 5);
    assert(numbers[2] == 3);

    free(voff(numbers));

    vector(char) string_like = nullvec;
    push(&string_like, 'h', 'i', '\0');

    puts(string_like); // hi

    for(size_t i = 0; i < len(string_like); i++) {
        printf("0x%02x, ", string_like[i]); // 0x68, 0x69, 0x00, 
    }

    free(voff(string_like));
}
```

## Interface

| Definition | Description |
| --- | --- |
| `vector(t)` | Vector type (expands to `t*`) |
| `struct vector` | Meta-data pertaining to allocated vectors |
| `nullvec` | An empty vector |

| Function / Macro | Description | Return Value |
| --- | --- | --- |
| `const struct vector* vinf(vector(t) v)` | Gets the meta-data of vector `v` | A `const` pointer to the vector meta-data |
| `struct vector* voff(vector(t) v)` | Gets the allocation offset of vector `v`, use in combination with `free()` to free vectors | A pointer to the start of the vector allocation |
| `vector(t) resv(vector(t)* v, size_t n)` | Reserves `n` items on vector `v` | A pointer to the newly allocated vector on success, or sets `errno` and returns `nullvec` on error |
| `vector(t) push(vector(t)* v, t a...)` | Pushes items `a` to the end of vector `v` | A pointer to the newly allocated vector on success, or sets `errno` and returns `nullvec` on error |
| `t* pop(vector(t)* v)` | Removes the last element from `v` | A pointer to the removed element on success, or sets `errno` and returns `NULL` on error |
| `size_t len(vector(t) v)` | Gets the length of vector `v` | The length in items of vector `v` |
| `vector(t) vec(t a...)` | Creates a vector of items `a` | A newly allocated vector of type `typeof(a[0])` including all items `a` |

## Notes

- Assumes `NULL == 0`
- Assumes `__builtin_clzll` exists
