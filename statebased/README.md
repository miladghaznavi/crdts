# State-based CRDTs
We implement a __last writer wins register__, __optimized observed removed set__, and an 
__add wins observed removed map__.

## Last Writer Wins Register (LWWRegister)
A LWWRegister is a variant of a register, i.e., a memory cell that stores an opaque value [[1]](#1).
A LWWRegister exposes the following operations:
- `value` that queries the most recent value of the local object,
- `assign` that assigns a given value to the local object, and
- `merge` that "merges" a LWWRegister received at a downstream replica with the local object.

### Local operations: `value` and `assign`
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
    LWWRegister<std::string> reg;
    reg.replica_id(REPLICA_ID);

    for (auto i = 0; i < random() % REGISTER_TEST_CASES + 1; ++i) {
        auto rand_val = random();
        reg.assign(std::to_string(rand_val));
        EXPECT_EQ(std::to_string(rand_val), reg.value());
    }//for
}//TEST
```

### Operations at a downstream replica: `merge`
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
    LWWRegister<std::string> reg1;
    LWWRegister<std::string> reg2;
    reg1.replica_id(REPLICA1_ID);
    reg1.replica_id(REPLICA2_ID);

    // Multiple random tests select one of two registers randomly,
    // assign a random value to the selected register, and merge
    // the other register with the selected register.
    for (auto i = 0; i < REGISTER_TEST_CASES; ++i) {
        LWWRegister<std::string>* f = &reg1;
        LWWRegister<std::string>* s = &reg2;

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

A set exposes the following main operations:
- `add` that add an element to to the local object,
- `remove` that removes an element from the local object,
- `contains` that queries the existence of a given element in the local object, and
- `merge` that merges an ORSet received at a downstream replica with the local object.

In addition to above operations, ORSet provides the following functions:
- `elements` that gets all elements stored in an ORSet,
- `size` that gets the size of an ORSet, and
- `replica_id` that gets the replica id associated to an ORSet.

### Creating an ORSet
The constructors takes the identifier of a replica to create an ORSet.

```cpp
#define REPLICA_ID 10
TEST(ORSet, Constructor) {
    ORSet<std::string> set(REPLICA_ID);
    EXPECT_EQ(REPLICA_ID, set.replica_id());
}//TEST
```

### Local operations: `add`, `remove`, and `contains`
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

The following exmaple shows the uses of `add` and `remove` and `contains`:

```cpp
TEST(ORSet, AddAndRemoveContainsSingleReplica) {
    ORSet<std::string> set(REPLICA_ID);
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

### Operations at a downstream replica: `merge`
`merge` takes an ORSet object that, and merge it with the local ORSet object. We call operations performed 
by the received object __remote__. `merge` applies remote remove operations, applies remote add operations,
and updates the local version vector with the remote version vector.

The following example shows simulates merging a local ORSet (`set1`) with a remote ORSet (`set2`).

```cpp
#define REPLICA1_ID 2
#define REPLICA2_ID 2

TEST(ORSet, Merge) {
    ORSet<std::string> set1(REPLICA_ID);
    ORSet<std::string> set2(REPLICA2_ID);
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

## Map
Map implements a convergent key value store. A map exposes the following main operations,
- `get` that returns a value associated to a given key,
- `contains` that checks the existence of a given key,
- `put` that takes a key and value and add them to the map,
- `remove` that deletes a given key and its associated value, and 
- `merge` that merges a map received at a downstream replica with the local object. 

In addition to above, our implementation includes the following functions:
- `key_value_pairs` that gets all key value pairs stored in a Map,
- `size` that gets the size of a Map, and
- `replica_id` that gets the replica id associated to a Map.

### Creating a Map

```cpp
TEST(Map, Constructor) {
    #define REPLICA_ID 10
    Map<std::string,std::string> map(REPLICA_ID);
    EXPECT_EQ(REPLICA_ID, map.replica_id());
}//TEST
```

### Local operations: `get`, `put`, `remove`, and `contains`

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

The following example shows the usage of `put` and `contains`.
```cpp
TEST(Map, PutAndContains) {
    // We compare Map with a C++ unordered_map as a reference
    Map<std::string,std::string> map(REPLICA_ID);
    std::unordered_map<std::string, std::string> ref;

    // Put random key value pairs to the map
    for (int i = 0; i < MAP_TEST_CASES; ++i) {
        std::string k = std::to_string(random());
        std::string v = std::to_string(random());
        map.put(k, v);
        ref[k] = v;
    }//for

    // Check the existence of added elements
    for (const auto& e: ref) {
        EXPECT_TRUE(map.contains(e.first));
    }//for
}//TEST
```

The following example shows the usage of `put` and `get`.

```cpp
TEST(Map, PutAndGet) {
    // We compare Map with a C++ unordered_map as a reference
    Map<std::string,std::string> map(REPLICA_ID);
    std::unordered_map<std::string, std::string> ref;

    // Put random key value pairs to the map
    for (int i = 0; i < MAP_TEST_CASES; ++i) {
        std::string k = std::to_string(random());
        std::string v = std::to_string(random());
        map.put(k, v);
        ref[k] = v;
    }//for

    // Check the existence of added elements
    for (const auto& e: ref) {
        EXPECT_EQ(map.get(e.first), e.second);
    }//for
}//TEST
```

The following example shows the usage of `put` and `remove`.

```cpp
TEST(Map, PutAndRemoveAndConatinsAndGet) {
    // We compare Map with a C++ unordered_map and vector as references
    Map<std::string,std::string> map(REPLICA_ID);
    std::unordered_map<std::string, std::string> ref;
    std::vector<std::string> keys;

    // Add random key value pairs to the map
    for (int i = 0; i < MAP_TEST_CASES; ++i) {
        std::string k = std::to_string(random());
        std::string v = std::to_string(random());
        map.put(k, v);

        auto old_size = ref.size();
        ref[k] = v;
        if (ref.size() > old_size)
            keys.push_back(k);
    }//for

    // Check the existence of added elements
    // Random remove and get operations
    while (!keys.empty()) {
        auto rand_index = random() % keys.size();
        EXPECT_TRUE(map.contains(keys[rand_index]));
        EXPECT_EQ(map.get(keys[rand_index]), ref[keys[rand_index]]);

        map.remove(keys[rand_index]);
        EXPECT_FALSE(map.contains(keys[rand_index]));

        EXPECT_THROW(map.get(keys[rand_index]), std::exception);

        std::swap(keys[keys.size() - 1], keys[rand_index]);
        keys.pop_back();
    }//while
}//TEST
```

### Operations at a downstream replica: `merge`
`merge` "merges" the keys and values of the local object with those of a received map. In merging keys, some
of local key value pairs may be removed, so `merge` also removes `unordered_map` entries associated with these 
keys. Then, `merge` merges registers associated with remaining keys.

```cpp
#define MAP_TEST_CASES 1000
#define REPLICA_ID 1
#define REPLICA2_ID 2
    
TEST(Map, Merge) {
    // Define the first map and put random key value pairs to it
    Map<std::string,std::string> map1(REPLICA_ID);
    for (int i = 0; i < MAP_TEST_CASES; ++i) {
        std::string k = std::to_string(random());
        std::string v = std::to_string(random());
        map1.put(k, v);
    }//for

    // Define the second map and put random key value pairs to it
    Map<std::string,std::string> map2(REPLICA2_ID);
    for (int i = 0; i < MAP_TEST_CASES; ++i) {
        std::string k = std::to_string(random());
        std::string v = std::to_string(random());
        map2.put(k, v);
    }//for

    // Merge map1 with map2, then map1 must include all key value pairs
    map1.merge(map2);
    EXPECT_EQ(map1.size(), ref.size());

    // Merge map2 with map1, then map2 must include all key value pairs
    map2.merge(map1);
    EXPECT_EQ(map1.size(), ref.size());
    for (const auto& kv: map1.key_value_pairs())
        EXPECT_EQ(map2.get(kv.first), kv.second);
}//TEST
```

# References
<a id="1">[1]</a>
Shapiro, M., Preguiça, N., Baquero, C., & Zawirski, M. (2011, October). Conflict-free replicated data types. In Symposium on Self-Stabilizing Systems (pp. 386-400). Springer, Berlin, Heidelberg.

<a id="2">[2]</a> 
Bieniusa A, Zawirski M, Preguiça N, Shapiro M, Baquero C, Balegas V, Duarte S. (2012). 
An optimized conflict-free replicated set
arXiv preprint arXiv:1210.3368.
