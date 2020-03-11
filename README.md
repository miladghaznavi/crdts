# Conflict Free Replicated Data Types

This repository implements and tests several state-based conflict free replicated data types (CRDTs).

The repository contains three directories:
- `core` including basic data types that are used in implementing CRDTs,
- `statebased` containing the implementation of state based CRDTs, and
- `test` including test suites for both basic data types and state based CRDTs.

The [statebased](https://github.com/miladghaznavi/crdts/tree/master/statebased) folder
also provides the development guide of using implemented CRDTs. For source code examples
of using CRDTs look at the [test](https://github.com/miladghaznavi/crdts/tree/master/test) folder.

## Setup and Test
We use [Google Test](https://github.com/google/googletest) for testing.
Script `setup.sh` from the root folder of this repository installs Google Test.

```bash
sudo ./setup.sh
```

Run `test.sh` from the root folder of this repository to test the entire implementation.
This script compiles the repository, places the compiled files in the folder `install`, and 
creates and executes a binary file `crdts_test` that runs test cases. Run the following to test the repository:

```bash
./test.sh
```

## State-based CRDTs
The repository implements a __last writer wins register__, an __optimized observed removed set__, and a 
__add wins observed removed map__.

### Last Writer Wins Register (LWWRegister)
A LWWRegister is a variant of a register, i.e., a memory cell that stores a value [[1]](#1).
A LWWRegister exposes the following operations:
- `value` that queries the most recent value of the local LWWRegister object,
- `assign` that assigns a given value to the local LWWRegister object, and
- `merge` that "merges" a LWWRegister received at a downstream replica with the local LWWRegister object.

### Observed Removed set (ORSet)
An optimized observed removed set (ORSet) [[2]](#2) is a variant of set, that is, a collection of
unique elements.

A set exposes the following operations:
- `add` that add an element to to the local ORSet object,
- `remove` that removes an element from the local ORSet object,
- `contains` that queries the existence of a given element in the local ORSet object, and
- `merge` that merges an ORSet received at a downstream replica with the local ORSet object.

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
