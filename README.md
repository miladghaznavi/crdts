# Conflict Free Replicated Data Types

This repository implements and test conflict free replicated data types (CRDTs).
Specifically, it focuses on state-based CRDTs.

The repository contains three directories:
- `core` including data types that can be used in implementing all CRDTs,
- `statebased` containing the implementation of state based CRDTs, and
- `test` including test suites of implementations in both `core` and `statebased`.

## State-based CRDTs
We implement a __last writer wins register__, __optimized observed removed set__, and a 
__add wins observed removed map__.

### Last Writer Wins Register (LWWRegister)
A LWWRegister is a variant of a register, i.e., a memory cell that stores an opaque value [[1]](#1).
A LWWRegister exposes the following operations:
- `value` that queries the most recent value of the local object,
- `assign` that assigns a given value to the local object, and
- `merge` that "merges" a LWWRegister received at a downstream replica with the local object.

`value` does not mutate the register.

`assign` operations across different replicas do not commute, violating the convergence
of the register. We use "last writer wins" policy to totally order `assign` operations
to *artificially* make `assign` commutative. Specifically, a *unique* timestamp is 
assigned to each `assign` operation, and an `assign` operation with a larger timestamp wins
the race with another `assign` operation. The uniqueness property prevents any tie between 
timestamps leading to globally ordering `assign` operations.  

We implement a timestamp using a monotonically increasing sequence number concatenated with
a unique identifier (uid); the comparison of sequence numbers precedes that of uids in comparing
two timestamps. Both the sequence number and uid are a 64 bit unsigned integer.

A timestamp constructor takes a 64 bit integer to initialize its uid allowing us to externally
ensure its uniqueness. Each call to `assign` increments the sequence number to order local `assign`
operations. We handle the integer overflow when the 32 bit space sequence number is exhausted by
the assumption that zero is larger than the maximum value, similar to TCP. 

We can use the identifier of a replica as a uid. A replica's identifier is unique among all 
replicas enabling us to break the ties when replicas associate the same sequence number to
their `assign` operations.

`merge` is expected to be called at a downstream replica upon receiving a LWWRegister from 
another replica. `merge` takes a LWWRegister object `r` and if the timestamp of `r` is greater
than the local timestamp, it replaces its local value and timestamp with the value and
timestamp of `r`.

### Observed Removed set (ORSet)
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
Thus, the memory usage grows by the number of add operations. OptORSet subsumes the need for tombestone
set and bound the memory usage of the set; a `remove` operation is effective only after an add,
thus there is no need to maintain the tombestone set. 

`merge` takes an ORSet object that we call it remote set, and merge it with the local ORSet object. `merge`
applies remote remove operations, applies remote add operations, and updates the local version vector
with the remote version vector.

### Map
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
