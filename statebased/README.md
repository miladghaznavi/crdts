# State-based CRDTs
We implement a __last writer wins register__, __optimized observed removed set__, and a 
__add wins observed removed map__.

## Last Writer Wins Register (LWWRegister)
A LWWRegister is a variant of a register, i.e., a memory cell that stores an opaque value [[1]](#1).
A LWWRegister exposes the following operations:
- `value` that queries the most recent value of the local object,
- `assign` that assigns a given value to the local object, and
- `merge` that "merges" a LWWRegister received at a downstream replica with the local object.

### `value` and `assign`
`value` does not mutate the register.

`assign` operations across different replicas do not commute, violating the convergence
of the register. We use "last writer wins" policy to totally order `assign` operations
to *artificially* make `assign` commutative. Specifically, a *unique* timestamp is 
assigned to each `assign` operation, and an `assign` operation with a larger timestamp wins
the race with another `assign` operation. The uniqueness property prevents any tie between 
timestamps leading to globally ordering `assign` operations.
A replica's identifier is unique among all 
replicas enabling us to break the ties when replicas associate the same sequence number to
their `assign` operations.

The following test case shows an exmaple of using these functions.

```cpp
#define REGISTER_TEST_CASES 1000

TEST(LWWRegister, AssignAndValue) {
    #define REPLICA_ID 1
    LWWRegister reg;
    reg.replica_id(REPLICA_ID);

    for (auto i = 0; i < random() % REGISTER_TEST_CASES + 1; ++i) {
        auto rand_val = random();
        reg.assign(std::to_string(rand_val));
        EXPECT_EQ(std::to_string(rand_val), reg.value());
    }//for
}//TEST
```

### `merge`
`merge` is expected to be called at a downstream replica upon receiving a LWWRegister from 
another replica. `merge` takes a LWWRegister object `r` and if the timestamp of `r` is greater
than the local timestamp, it replaces its local value and timestamp with the value and
timestamp of `r`.

The following test case shows exmaples of simulating merge at two replicas.

```cpp
#define REGISTER_TEST_CASES 1000
TEST(LWWRegister, Merge) {
    #define REPLICA1_ID 1
    #define REPLICA2_ID 2
    LWWRegister reg1;
    LWWRegister reg2;
    reg1.replica_id(REPLICA1_ID);
    reg1.replica_id(REPLICA2_ID);

    // Multiple random tests select one of two registers randomly,
    // assign a random value to the selected register, and merge
    // the other register with the selected register.
    for (auto i = 0; i < REGISTER_TEST_CASES; ++i) {
        LWWRegister* f = &reg1;
        LWWRegister* s = &reg2;

        auto swap = random() % 2;
        if (swap == 1)
            std::swap(s, f);

        auto rand_val = random();
        f->assign(std::to_string(rand_val));
        s->merge(*f);
        EXPECT_EQ(f->value(), s->value());
    }//for
}//TEST
```

## Observed Removed set (ORSet)
An optimized observed removed set (ORSet) [[2]](#2) is a variant of set, that is, a collection of
unique elements.

A set exposes the following operations:
- `add` that add an element to to the local object,
- `remove` that removes an element from the local object,
- `contains` that queries the existence of a given element in the local object, and
- `merge` that merges an ORSet received at a downstream replica with the local object.

`add` and `remove` are idempotent and commutative, and `contains` does not mutate an object.
However, concurrent `add` and `remove` on the same element do not commute. To enforce convergence, 
the original ORSet distinguishes between `add` operations on the same element by assigning a unique
tag to each `add` invocation and storing the unique identifier alongside the  element. An element 
is removed by moving it to a tombstone set. An element can be always added to the set because it
is assigned a new tag. This implements *add wins* policy where an `add` on an  element `e` wins
over concurrent `remove` operations on `e`.

The original ORSet consumes unbounded memory because `remove` does not release any memory allocation.
Thus, the memory usage grows by the number of add operations. The optimized variant of ORSet subsumes
the need for tombestone set and bound the memory usage of the set; a `remove` operation is effective
only after an add, thus there is no need to maintain the tombestone set.

```cpp
#define SET_TEST_CASES 1000

TEST(ORSet, Constructor) {
    #define REPLICA_ID 10
    ORSet set(REPLICA_ID);
    EXPECT_EQ(REPLICA_ID, set.replica_id());
}//TEST
```

```cpp
TEST(ORSet, AddAndRemoveContainSingleReplica) {
    ORSet set(REPLICA_ID);
    // Use an C++ unordered_set as a reference for testing
    std::unordered_set<std::string> ref;

    // Add random values to the set
    for (int i = 0; i < SET_TEST_CASES; ++i) {
        std::string b = std::to_string(random());
        set.add(b);
        ref.insert(b);
    }//for

    // Check the existence of added elements
    for (const auto& e: ref) {
        EXPECT_TRUE(set.contains(e));
    }//for

    // Check the existence of removed elements
    for (const auto& e: ref) {
        set.remove(e);
        EXPECT_FALSE(set.contains(e));
    }//for
}//TEST
```

```cpp
TEST(ORSet, Elements) {
    ORSet set(REPLICA_ID);
    // Use C++ unordered_set and vector as references
    std::unordered_set<std::string> ref;
    std::vector<std::string> keys;

    // Adding random elements to set
    for (int i = 0; i < SET_TEST_CASES; ++i) {
        std::string b = std::to_string(random());
        set.add(b);

        auto old_size = ref.size();
        ref.insert(b);
        if (ref.size() > old_size)
            keys.push_back(b);
    }//for

    auto elems = set.elements();

    // Check if all random elements exist in `elems`
    for (int i = 0; i < ref.size(); i++) {
        EXPECT_TRUE(ref.count(*elems.begin()));
        elems.erase(elems.begin());
    }//for

    // Randomly remove some elements
    auto to_remove = random() % ref.size() + 1;
    for (int i = 0; i < to_remove; ++i) {
        auto ind = random() % keys.size();
        ref.erase(keys[ind]);
        set.remove(keys[ind]);

        std::swap(keys[ind], keys[keys.size() - 1]);
        keys.pop_back();
    }//for

    elems = set.elements();

    // Check if all remaining elements exist in `elems`
    for (int i = 0; i < elems.size(); i++) {
        EXPECT_TRUE(ref.count(*elems.begin()));
        elems.erase(elems.begin());
    }//for
}//TEST
```

```cpp
TEST(ORSet, Merge) {
    // We compare an ORSet with a C++ unordered_set and vector as references
    std::unordered_set<std::string> ref;
    std::vector<std::string> keys;

    ORSet set1(REPLICA_ID);
    #define REPLICA2_ID 2
    ORSet set2(REPLICA2_ID);

    // Merging two empty set
    set1.merge(set2);
    set2.merge(set1);
    EXPECT_EQ(0, set1.size());
    EXPECT_EQ(set1.size(), set2.size());

    // Merging an empty set with another set
    for (int i = 0; i < SET_TEST_CASES; ++i) {
        std::string b = std::to_string(random());
        set1.add(b);

        auto old_size = ref.size();
        ref.insert(b);
        if (ref.size() > old_size)
            keys.push_back(b);
    }//for
    set2.merge(set1);
    auto elems = set2.elements();
    EXPECT_TRUE(elems == ref);

    // Remove and then merge
    for (int i = 0; i < elems.size() / 2; ++i) {
        auto rand_ind = random() % keys.size();
        set1.remove(keys[rand_ind]);
        ref.erase(keys[rand_ind]);
        std::swap(keys[rand_ind], keys[keys.size() - 1]);
        keys.pop_back();
    }//for
    EXPECT_TRUE(ref == set1.elements());
    set2.merge(set1);
    EXPECT_TRUE(set1.elements() == set2.elements());

    // Merge random add and remove operations
    for (int i = 0; i < elems.size(); ++i) {
        // Select a set among set1 or set2
        auto first_or_second = random() % 2;
        ORSet* p = (first_or_second == 0) ? &set1 : &set2;

        // Call an add or remove operation of the selected set randomly
        auto add_or_remove = random() % 2;
        if (add_or_remove == 0) {//add
            std::string b = std::to_string(random());
            p->add(b);

            auto old_size = ref.size();
            ref.insert(b);
            if (ref.size() > old_size)
                keys.push_back(b);
        }//if
        else {//remove
            auto rand_ind = random() % keys.size();
            p->remove(keys[rand_ind]);
            ref.erase(keys[rand_ind]);
            std::swap(keys[rand_ind], keys[keys.size() - 1]);
            keys.pop_back();
        }//else
    }//for
    set1.merge(set2);
    set2.merge(set1);
    EXPECT_TRUE(set1.elements() == set2.elements());

    // Check add-wins policy: a more recent remove operation must win over an older add operation
    for (auto i = 0; i < SET_TEST_CASES / 10 + 1; ++i) {
        auto b = std::to_string(random());
        set1.add(b);
        set2.merge(set1);
        EXPECT_TRUE(set1.contains(b));
        EXPECT_TRUE(set2.contains(b));

        set2.remove(b);
        set1.merge(set2);
        EXPECT_FALSE(set1.contains(b));
        EXPECT_FALSE(set2.contains(b));
    }//for

    // Check add-wins policy: an add operation must win over concurrent remove operation
    for (auto i = 0; i < SET_TEST_CASES / 10 + 1; ++i) {
        auto b = std::to_string(random());
        set1.add(b);
        EXPECT_TRUE(set1.contains(b));

        set2.add(b);
        EXPECT_TRUE(set2.contains(b));

        set2.remove(b);
        EXPECT_FALSE(set2.contains(b));

        set1.merge(set2);
        EXPECT_TRUE(set1.contains(b));

        set2.add(b);
        EXPECT_TRUE(set2.contains(b));

        set2.remove(b);
        EXPECT_FALSE(set2.contains(b));

        // b must reappear in set2
        set2.merge(set1);
        EXPECT_TRUE(set2.contains(b));
    }//for
}//TEST
```

`merge` takes an ORSet object that, and merge it with the local ORSet object. We call operations performed 
by the received object __remote__. `merge` applies remote remove operations, applies remote add operations,
and updates the local version vector with the remote version vector.

## Map
Map implements a convergent key value store. A map exposes the following operations,
- `get` that returns a value associated to a given key,
- `contains` that checks the existence of a given key,
- `put` that takes a key and value and add them to the map,
- `remove` that deletes a given key and its associated value, and 
- `merge` that merges a map received at a downstream replica with the local object. 

`get` and `contains` do not mutate the object. 

Concurrent `put` operations on the same key do not commute. We use "last writer wins" policy to make
this operation commutative. We implement this by storing the value associated to a key  in a LWWRegister.
We store these registers with their associated keys in a C++ `unordered_map`.

Individual `remove` operations commute. `remove` and `put` operations on different keys are also commutative. 
However, `remove` and `put` on the same key do not commute. To resolve this issue, we use the "add wins" policy
where a key remains in the map if replicas concurrently put and remove this key. We implement this policy by 
maintaining keys in an ORSet. `put` and `remove` simultaneously operate on the ORSet and C++ `unordered_map` objects
to ensure to ensure that keys stored in these objects are consistent. Specifically, `put` adds/updates a key 
into the `unordered_map` object, when it adds/updates this key in the ORSet object. When `remove` deletes a 
key from the ORSet, it also deletes this key from the `unordered_map` object. 

`merge` merges the keys and values of the local object with those of a received map. In merging keys, some
of local key value pairs may be removed, so `merge` also removes `unordered_map` entries associated with these 
keys. Then, `merge` merges registers associated with remaining keys.

# References
<a id="1">[1]</a>
Shapiro, M., Preguiça, N., Baquero, C., & Zawirski, M. (2011, October). Conflict-free replicated data types. In Symposium on Self-Stabilizing Systems (pp. 386-400). Springer, Berlin, Heidelberg.

<a id="2">[2]</a> 
Bieniusa A, Zawirski M, Preguiça N, Shapiro M, Baquero C, Balegas V, Duarte S. (2012). 
An optimized conflict-free replicated set
arXiv preprint arXiv:1210.3368.
