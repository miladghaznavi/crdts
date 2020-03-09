# Conflict Free Replicated Data Types

This repository implements and test conflict free replicated data types (CRDTs).
Specifically, it focuses on state-based CRDTs.

The repository contains three directories:
- `core` including data types that can be used in implementing all CRDTs,
- `statebased` containing the implementation of state based CRDTs, and
- `test` including test suites of implementations in both `core` and `statebased`.

The [statebased](https://github.com/miladghaznavi/crdts/tree/master/statebased) folder
provides the details of using CRDT datatypes.
For testcases look at the examples in the [test](https://github.com/miladghaznavi/crdts/tree/master/test) foler
provides the details of using CRDT datatypes.


## State-based CRDTs
We implement a __last writer wins register__, __optimized observed removed set__, and a 
__add wins observed removed map__.

### Last Writer Wins Register (LWWRegister)
A LWWRegister is a variant of a register, i.e., a memory cell that stores an opaque value [[1]](#1).
A LWWRegister exposes the following operations:
- `value` that queries the most recent value of the local object,
- `assign` that assigns a given value to the local object, and
- `merge` that "merges" a LWWRegister received at a downstream replica with the local object.

### Observed Removed set (ORSet)
An optimized observed removed set (ORSet) [[2]](#2) is a variant of set, that is, a collection of
unique elements.

A set exposes the following operations:
- `add` that add an element to to the local object,
- `remove` that removes an element from the local object,
- `contains` that queries the existence of a given element in the local object, and
- `merge` that merges an ORSet received at a downstream replica with the local object.

### Map
Map implements a convergent key value store. A map exposes the following operations,
- `get` that returns a value associated to a given key,
- `contains` that checks the existence of a given key,
- `put` that takes a key and value and add them to the map,
- `remove` that deletes a given key and its associated value, and 
- `merge` that merges a map received at a downstream replica with the local object. 

# References
<a id="1">[1]</a>
Shapiro, M., Preguiça, N., Baquero, C., & Zawirski, M. (2011, October). Conflict-free replicated data types. In Symposium on Self-Stabilizing Systems (pp. 386-400). Springer, Berlin, Heidelberg.

<a id="2">[2]</a> 
Bieniusa A, Zawirski M, Preguiça N, Shapiro M, Baquero C, Balegas V, Duarte S. (2012). 
An optimized conflict-free replicated set
arXiv preprint arXiv:1210.3368.
