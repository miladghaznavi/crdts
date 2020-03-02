# Conflict Free Replicated Data Types

This repository implements conflict free replicated data types (CRDTs).
Specifically, this repository implements and tests on state-based CRDTs,  
It contains two directories.
`sb` includes the implementation of state based state objects, 
and `test` contains test suites to test the implementation.

## State-based CRDTs
We implement a __last-writer-wins register__ and a __map__.

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

A timestamp constructor takes a 64 bit integer to initialize its uid allowing
us to externally ensure its uniqueness.
Each call to `assign` increments the sequence number to order local `assign` operations.
The current implementation does not handle the integer overflow when increments exhaust 64 bit
space of the sequence number. 

We can use the identifier of a replica as a uid. A replica's identifier is unique among all 
replicas enabling us to break the ties when replicas associate the same sequence number to
their `assign` operations.

`merge` is expected to be called at a downstream replica upon receiving a LWWRegister from 
another replica. `merge` takes a LWWRegister object `r` and if the timestamp of `r` is greater
than the local timestamp,  it replaces its local value and timestamp with the value and
timestamp of `r`.

An `assign` operation proceeding a `merge` must be still able to use the local replica's 
identifier as the uid of its timestamp. To implement this, a timestamp resets its uid to 
the identifier of its local replica in addition to incrementing its sequence number. The 
timestamp still captures the casual dependency because the updated timestamp is still larger 
than the timestamp of `merge` (recall that a timestamp "happens before" another one if its 
sequence number is smaller).

### Map

