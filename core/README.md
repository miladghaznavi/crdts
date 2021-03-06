# Timestamp
We implement a timestamp using a monotonically increasing sequence number concatenated with
a unique identifier (uid); the comparison of sequence numbers precedes that of uids in comparing
two timestamps. Both the sequence number and uid are 64 bit unsigned integers.
We can use the identifier of a replica as a uid.

A timestamp constructor takes a 64 bit integer to initialize its uid allowing us to externally
ensure its uniqueness. Each call to `assign` increments the sequence number to order local `assign`
operations. We currently do not handle the integer overflow when the 64 bit space sequence number is exhausted.

## Creating a timestamp

```cpp
TEST(Timestamp, Intialize) {
    #define REPLICA_ID 10
    Timestamp reg;
    reg.replica_id(REPLICA_ID);

    EXPECT_EQ(reg.replica_id(), REPLICA_ID);
}//TEST
```

## Updating a timestamp

```cpp
TEST(Timestamp, Update) {
    Timestamp t;
    auto random_repeat = random() % TIMESTAMP_TEST_CASES + 1;
    for (int i = 0; i < random_repeat; ++i)
        t.update();

    EXPECT_EQ(t.sequence_number(), random_repeat);
}//TEST
```

## Copying a timestamp
```cpp
TEST(Timestamp, Copy) {
    Timestamp t1;
    t1.replica_id(1);
    auto random_repeat = random() % TIMESTAMP_TEST_CASES + 1;
    for (int i = 0; i < random_repeat; ++i)
        t1.update();

    Timestamp t2;
    t2.replica_id(3);
    t2.copy(t1);
    EXPECT_TRUE(t1 == t2);

    t1.update();
    t2.update();
    EXPECT_TRUE(t1 != t2);
}//TEST
```

## Comparing timestamps

```cpp
#define TIMESTAMP_TEST_CASES 1000
TEST(Timestamp, Comparison) {
    #define REPLICA1_ID 1
    Timestamp t1, t1_copy;
    t1.replica_id(REPLICA1_ID);
    t1_copy.replica_id(t1.replica_id());
    EXPECT_TRUE(t1 == t1_copy);
    EXPECT_FALSE(t1 != t1_copy);

    #define REPLICA2_ID 2
    Timestamp t2;
    t2.replica_id(REPLICA2_ID);
    EXPECT_FALSE(t1 == t2);
    EXPECT_TRUE(t1 != t2);

    t1.update();
    t2.update();
    EXPECT_FALSE(t1 == t2);
    EXPECT_TRUE(t1 < t2);
    EXPECT_TRUE(t1 != t2);
}//TEST
```
