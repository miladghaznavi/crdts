# Timestamp
We implement a timestamp using a monotonically increasing sequence number concatenated with
a unique identifier (uid); the comparison of sequence numbers precedes that of uids in comparing
two timestamps. Both the sequence number and uid are 64 bit unsigned integers.

A timestamp constructor takes a 64 bit integer to initialize its uid allowing us to externally
ensure its uniqueness. Each call to `assign` increments the sequence number to order local `assign`
operations. We currently do not handle the integer overflow when the 64 bit space sequence number is exhausted.
