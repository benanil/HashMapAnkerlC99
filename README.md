# HashMapAnkerlC99
A cache-efficient, densely stored hash map in C99

# HashMap
 
A fast, open-addressing hash map in C using Robin Hood hashing with fingerprint-based early termination. Keys are `uint64_t`, values are arbitrary fixed-size blobs.
Ankerl's hashmap is performant and heavily benchmarked(better than cpp stl and such), it also has low memory footprint

# Performance
| Function | Time |
|---|---|
| `std::unordered_map Insert` | `61ms` |
| `std::unordered_map Lookup` | `31ms` |
| `std::unordered_map Erase`  | `62ms` |
| `HashMap Insert`            | `32ms` |
| `HashMap Lookup`            | `25ms` |
| `HashMap Erase`             | `21ms` |

# Test
tested with many leetcode problems and custom test code

## Customization
 
Override allocator and memory primitives before including the header:
 
```c
#define HMRealloc(mem, size) my_realloc(mem, size)
#define HMFree(mem)          my_free(mem)
#define HMMemset(mem, val, size) my_memset(mem, val, size)
#define HMMemcpy(mem, src, size) my_memcpy(mem, src, size)
```
 
If not defined, defaults to `realloc`/`free`/`memset`/`memcpy`.
 
## API
 
### Lifecycle
 
```c
HashMap HMCreate(uint32_t reserveCount, uint32_t valueSize);
void    HMDestroy(HashMap* hm);
void    HMClear(HashMap* hm);
void    HMReserve(HashMap* hm, uint32_t capacity);
```
 
`HMCreate` allocates a map sized to hold at least `reserveCount` entries of `valueSize` bytes each. `HMClear` removes all entries but retains allocated memory. `HMReserve` pre-allocates to avoid rehashing on bulk inserts.
 
### Queries

```c
void* HMFind(const HashMap* hm, uint64_t key);
bool  HMTryGetTYPE(const HashMap* hm, uint64_t key, TYPE* out);
bool  HMContains(const HashMap* hm, uint64_t key);
// macros
HMEmpty(hm)  // bool     — true if count == 0
```

`HMFind` returns a pointer into the internal value array, or `NULL` if not found. The pointer is invalidated by any insertion or rehash.
 
### Mutation
 
```c
void* HMInsert(HashMap* hm, uint64_t key, const void* value);
void* HMInsertOrAssign(HashMap* hm, uint64_t key, const void* value);
bool  HMErase(HashMap* hm, uint64_t key);
```
 
`HMInsert` does nothing and returns `NULL` if the key already exists. `HMInsertOrAssign` overwrites an existing value and returns a pointer to it. Both return a pointer to the stored value on success. `HMErase` returns `false` if the key was not found.
 
### Iterate on dense hashmap
iterating is super fast, because its dense
```c
Value* values = (Value*)hashmap.values;
uint64_t* keys = hashmap.keys;
// loop with hashmap.count
```
### Type-safe wrappers
 
`HM_DEFINE_TYPE(NAME, TYPE)` generates a set of inline functions for a concrete value type, eliminating void-pointer casts at call sites:
 
```c
HM_DEFINE_TYPE(Int, int)
 
HashMap hm = HMCreate(0, sizeof(int));
HMInsertInt(&hm, 42, 7);
int* val = HMFindInt(&hm, 42);   // returns NULL if not found
int out;
bool ok = HMTryGetInt(&hm, 42, &out);
HMInsertOrAssignInt(&hm, 42, 99);
```
 
Generated functions for `HM_DEFINE_TYPE(NAME, TYPE)`:
 
| Function | Signature |
|---|---|
| `HMFindNAME` | `TYPE* (const HashMap*, uint64_t)` |
| `HMTryGetNAME` | `bool (const HashMap*, uint64_t, TYPE* out)` |
| `HMInsertNAME` | `TYPE* (HashMap*, uint64_t, TYPE)` |
| `HMInsertOrAssignNAME` | `TYPE* (HashMap*, uint64_t, TYPE)` |
 
## Example
 
```c
#define HM_HASHMAP_IMPLEMENTATION // only on one source file like stb
#include "HashMap.h"
 
HM_DEFINE_TYPE(Float, float)
 
int main(void) {
    HashMap hm = HMCreate(64, sizeof(float));
 
    HMInsertFloat(&hm, 1, 3.14f);
    HMInsertFloat(&hm, 2, 2.71f);
 
    float* p = HMFindFloat(&hm, 1);
    if (p) printf("%f\n", *p);
 
    HMErase(&hm, 2);
    HMDestroy(&hm);
    return 0;
}
```
 
## Pointer stability
 
Pointers returned by `HMFind`, `HMInsert`, and `HMInsertOrAssign` are valid only until the next operation that may trigger a rehash (any insert or `HMReserve` call). Do not cache them across mutations.
 
## Thread safety
 
None. External synchronization is required for concurrent access.

Credits

## Based on
[ankerl::unordered_dense](https://github.com/martinus/unordered_dense) by Martin Leitner-Ankerl

Ported and adapted to C99 by:
Anılcan Gülkaya
 
