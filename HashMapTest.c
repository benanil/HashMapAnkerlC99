#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


typedef uint32_t u32;
typedef uint64_t u64;
typedef struct PCG_
{
    u64 state;
    u64 inc;
} PCG;
	
u32 PCGNext(PCG* pcg)
{
    u64 oldstate = pcg->state;
    pcg->state = oldstate * 6364136223846793005ULL + (pcg->inc | 1);
    u64 xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    u64 rot = oldstate >> 59u;
    #pragma warning(disable : 4146) // unary minus warning fix
    // if you get unary minus error disable sdl checks from msvc settings
    return (u32)((xorshifted >> rot) | (xorshifted << ((-rot) & 31)));
}

static void test_create_destroy(void)
{
    HashMap hm = HMCreate(0, sizeof(int));

    assert(HMEmpty(&hm));
    assert(HMSize(&hm) == 0);
    assert(HMFind(&hm, 123) == NULL);
    assert(!HMContains(&hm, 123));

    HMDestroy(&hm);
}

static void test_insert_find_contains(void)
{
    HashMap hm = HMCreate(0, sizeof(int));

    int v1 = 10;
    int v2 = 20;
    int v3 = 30;

    int* p1 = (int*)HMInsert(&hm, 1, &v1);
    int* p2 = (int*)HMInsert(&hm, 2, &v2);
    int* p3 = (int*)HMInsert(&hm, 3, &v3);

    assert(p1 && p2 && p3);
    assert(HMSize(&hm) == 3);
    assert(!HMEmpty(&hm));

    assert(HMContains(&hm, 1));
    assert(HMContains(&hm, 2));
    assert(HMContains(&hm, 3));
    assert(!HMContains(&hm, 4));

    assert(*(int*)HMFind(&hm, 1) == 10);
    assert(*(int*)HMFind(&hm, 2) == 20);
    assert(*(int*)HMFind(&hm, 3) == 30);
    assert(HMFind(&hm, 4) == NULL);

    HMDestroy(&hm);
}

static void test_insert_duplicate(void)
{
    HashMap hm = HMCreate(0, sizeof(int));

    int a = 111;
    int b = 222;

    int* first = (int*)HMInsert(&hm, 7, &a);
    int* dup   = (int*)HMInsert(&hm, 7, &b);

    assert(first != NULL);
    assert(dup == NULL || dup == first); // depending on your API behavior
    assert(HMSize(&hm) == 1);
    assert(*(int*)HMFind(&hm, 7) == 111);

    HMDestroy(&hm);
}

static void test_insert_or_assign(void)
{
    HashMap hm = HMCreate(0, sizeof(int));

    int a = 1;
    int b = 2;

    int* p1 = (int*)HMInsertOrAssign(&hm, 42, &a);
    assert(p1 != NULL);
    assert(HMSize(&hm) == 1);
    assert(*(int*)HMFind(&hm, 42) == 1);

    int* p2 = (int*)HMInsertOrAssign(&hm, 42, &b);
    assert(p2 != NULL);
    assert(HMSize(&hm) == 1);
    assert(*(int*)HMFind(&hm, 42) == 2);

    HMDestroy(&hm);
}

static void test_erase(void)
{
    HashMap hm = HMCreate(0, sizeof(int));

    int v1 = 10;
    int v2 = 20;

    HMInsert(&hm, 1, &v1);
    HMInsert(&hm, 2, &v2);

    assert(HMSize(&hm) == 2);

    assert(HMErase(&hm, 1));
    assert(!HMContains(&hm, 1));
    assert(HMFind(&hm, 1) == NULL);
    assert(HMSize(&hm) == 1);

    assert(!HMErase(&hm, 1)); // already removed
    assert(HMErase(&hm, 2));
    assert(HMEmpty(&hm));

    HMDestroy(&hm);
}

static void test_clear(void)
{
    HashMap hm = HMCreate(0, sizeof(int));

    for (u64 i = 0; i < 16; ++i)
    {
        int v = (int)(i * 3);
        HMInsert(&hm, i, &v);
    }

    assert(HMSize(&hm) == 16);

    HMClear(&hm);
    assert(HMSize(&hm) == 0);
    assert(HMEmpty(&hm));

    for (u64 i = 0; i < 16; ++i)
        assert(!HMContains(&hm, i));

    HMDestroy(&hm);
}

static void test_reserve(void)
{
    HashMap hm = HMCreate(0, sizeof(int));

    HMReserve(&hm, 128);

    for (u64 i = 0; i < 100; ++i)
    {
        int v = (int)i;
        assert(HMInsert(&hm, i, &v) != NULL);
    }

    assert(HMSize(&hm) == 100);

    for (u64 i = 0; i < 100; ++i)
        assert(*(int*)HMFind(&hm, i) == (int)i);

    HMDestroy(&hm);
}

static void test_keys_values(void)
{
    HashMap hm = HMCreate(0, sizeof(int));

    int values_in[4] = { 11, 22, 33, 44 };
    u64 keys_in[4]   = { 100, 200, 300, 400 };

    for (int i = 0; i < 4; ++i)
        HMInsert(&hm, keys_in[i], &values_in[i]);

    u64* keys = (u64*)hm.keys;
    int* values = (int*)hm.values;

    assert(keys != NULL);
    assert(values != NULL);
    assert(HMSize(&hm) == 4);

    // Order is usually unspecified in hash maps.
    // Verify membership instead of sequence.
    bool found_keys[4] = { false, false, false, false };
    bool found_vals[4] = { false, false, false, false };

    for (u32 i = 0; i < HMSize(&hm); ++i)
    {
        for (int k = 0; k < 4; ++k)
        {
            if (keys[i] == keys_in[k])
                found_keys[k] = true;
            if (values[i] == values_in[k])
                found_vals[k] = true;
        }
    }

    for (int k = 0; k < 4; ++k)
    {
        assert(found_keys[k]);
        assert(found_vals[k]);
    }

    HMDestroy(&hm);
}

static void test_nontrivial_value_size(void)
{
    typedef struct Item
    {
        int a;
        float b;
        char c[8];
    } Item;

    HashMap hm = HMCreate(0, sizeof(Item));

    Item x = { 7, 3.5f, "abc" };
    Item y = { 9, 1.25f, "xyz" };

    HMInsert(&hm, 1, &x);
    HMInsertOrAssign(&hm, 2, &y);

    Item* px = (Item*)HMFind(&hm, 1);
    Item* py = (Item*)HMFind(&hm, 2);

    assert(px && py);
    assert(px->a == 7);
    assert(px->b == 3.5f);
    assert(strcmp(px->c, "abc") == 0);
    assert(py->a == 9);
    assert(py->b == 1.25f);
    assert(strcmp(py->c, "xyz") == 0);

    HMDestroy(&hm);
}

static void test_stress_insert_erase(void)
{
    const u32 N = 1000000; // adjust if needed
    HashMap hm = HMCreate(0, sizeof(u64));

    // Track inserted keys to allow removals
    u64* keys = (u64*)malloc(sizeof(u64) * N);
    assert(keys);

    PCG pcg;
    // --- bulk insert ---
    for (u32 i = 0; i < N; ++i)
    {
        u64 k = PCGNext(&pcg);
        u64 v = k ^ 0xDEADBEEFCAFEBABEull;

        keys[i] = k;

        void* p = HMInsertOrAssign(&hm, k, &v);
        assert(p != NULL);
    }

    assert(HMSize(&hm) <= N); // duplicates may reduce size

    // --- verify random subset ---
    for (u32 i = 0; i < N; i += 97)
    {
        u64 k = keys[i];
        u64* v = (u64*)HMFind(&hm, k);

        if (v) // key may have been duplicated later
            assert(*v == (k ^ 0xDEADBEEFCAFEBABEull));
    }

    // --- bulk erase ---
    for (u32 i = 0; i < N; ++i)
    {
        HMErase(&hm, keys[i]);
    }

    assert(HMEmpty(&hm));
    assert(HMSize(&hm) == 0);

    // --- reinsert after erase (checks reuse correctness) ---
    for (u32 i = 0; i < N; ++i)
    {
        u64 k = PCGNext(&pcg);
        u64 v = k + 12345;

        void* p = HMInsert(&hm, k, &v);
        assert(p != NULL);
    }

    assert(HMSize(&hm) > 0);

    HMDestroy(&hm);
    free(keys);
}


typedef struct { int id; float score; } Player;

int SparseSetTest() {
    printf("Starting Two-Array Sparse Set Tests...\n");

    SparseSet ss = SSCreate(10, sizeof(Player));

    Player p1 = {1, 95.5f};
    Player p2 = {2, 80.0f};
    Player p2_updated = {2, 99.0f};

    // 1. Test SSInsert (New entry)
    bool inserted = SSInsert(&ss, 50, &p1);
    assert(inserted == true);
    assert(ss.size == 1);

    // 3. Test SSInsertOrAssign
    SSInsertOrAssign(&ss, 50, &p2_updated); // Should update existing
    Player* pCheck = (Player*)SSGet(&ss, 50);
    assert(pCheck->score == 99.0f);
    assert(ss.size == 1);

    SSInsertOrAssign(&ss, 20, &p2); // Should create new
    assert(ss.size == 2);

    // 4. Test Removal with Swap
    // Removing index 50 should move index 20 into the 0th dense slot
    SSRemove(&ss, 50);
    assert(ss.size == 1);
    assert(SSContains(&ss, 50) == false);
    assert(SSContains(&ss, 20) == true);
    
    // Check if back-link survived the swap
    assert(ss.sparse[20] == 0);
    assert(ss.dense[0] == 20);

    SSDestroy(&ss);
    printf("All tests passed! Your Sparse Set is solid.\n");

    return 0;
}

int HashMapTest(void)
{
    test_create_destroy();
    test_insert_find_contains();
    test_insert_duplicate();
    test_insert_or_assign();
    test_erase();
    test_clear();
    test_reserve();
    test_keys_values();
    test_nontrivial_value_size();
    test_stress_insert_erase();
    printf("All HashMap tests passed.\n");
    return 0;
}
int main()
{
    HashMapTest();
	SparseSetTest();
    return 0;	
}
