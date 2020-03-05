# Conflict Free Replicated Data Types

This repository implements conflict free replicated data types (CRDTs).
Specifically, this repository implements and tests on state-based CRDTs,  
It contains two directories.
`sb` includes the implementation of state based state objects, 
and `test` contains test suites to test the implementation.

## State-based CRDTs
We implement a __last-writer-wins register__, __optimized-observed-removed set__, and a 
__add-wins-observed-removed-map__.

### Last-writer-wins Register (LWWRegister)
A LWWRegister object is a variant of a register, i.e., a memory cell that stores a value.
A LWWRegister exposes the following operations:
- `value` that queries the most recent value of the local object,
- `assign` that assigns a given value to the local object, and
- `merge` that "merges" a LWWRegister received at a downstream replica with the local object.

`value` does not mutate the value of the register. If the LWWRegister is not initialized,
calling `value` throws an exception.

`assign` is idempotent but not commutative, violating the convergence property of the object.
We use "last writer wins" policy to totally order `assign` operations allowing us to 
*artificially* make this operation commutative.
Specifically, a *unique* timestamp is assigned to each `assign` operation, and an `assign`
operation with a larger timestamp wins the race with another `assign` operation. The uniqueness
property of a timestamp is particularly important because it prevents any tie
between timestamps.

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

An `assign` operation proceeding a `merge` must be still able to use the local replica's 
identifier as the uid of its timestamp. To implement this, a timestamp resets its uid to 
the identifier of its local replica in addition to incrementing its sequence number. The 
timestamp still captures the casual dependency because the updated timestamp is still larger 
than the timestamp of `merge` (recall that a timestamp "happens before" another one if its 
sequence number is smaller).

### Optimized Observed Removed set (OptORSet)
An optimized observed removed set (OptORSet) [[1]](#1) is a variant of set, that is, a collection of unique elements.

A set exposes the following operations:
- `add` that add an element to to the local object,
- `remove` that removes an element from the local object,
- `contains` that queries the existence of a given element in the local object, and
- `merge` that merges a ORSet received at a downstream replica with the local object.

`add` and `remove` are idempotent and commutative, and `contains` does not mutate an object.
However, concurrent `add` and `remove` on the same element do not commute. To enforce convergence, 
the orignal observed removed set (ORSet) distinguishes between `add` operations on the same element
by assigning a unique tag to each `add` invocation and storing the unique identifier alongside the 
element. An element is removed by moving it to a tombstone set. An element can be always added to
the set because it is assigned a new tag. This implements *add-wins* policy where an `add` on an 
element `e` wins over concurrent `remove` operations on `e`.

The ORSet consumes unbounded memory because `remove` does not release any memory allocation. Thus,
the memory usage grows by the number of add operations. OptORSet subsumes the need for tombestone set
and bound the memory usage of the set. The intution is that because a `remove` operation is effective
only after an add, there is no need to maintain the tombestone set. 

`merge` takes an ORSet object and performs the following for each element `e`.

### Map


# References
<a id="1">[1]</a> 
Bieniusa A, Zawirski M, Preguiça N, Shapiro M, Baquero C, Balegas V, Duarte S. (2012). 
An optimized conflict-free replicated set
arXiv preprint arXiv:1210.3368.
