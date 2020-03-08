#include <gtest/gtest.h>
#include "../core/timestamp.hh"

namespace {
#define TIMESTAMP_TEST_CASES 1000

    TEST(Timestamp, Intialize) {
        #define REPLICA_ID 10
        Timestamp reg;
        reg.replica_id(REPLICA_ID);

        EXPECT_EQ(reg.replica_id(), REPLICA_ID);
        EXPECT_TRUE(reg.beginning_of_time());
    }//TEST

    TEST(Timestamp, Update) {
        Timestamp t;
        auto random_repeat = random() % TIMESTAMP_TEST_CASES + 1;
        for (int i = 0; i < random_repeat; ++i)
            t.update();

        EXPECT_EQ(t.sequence_number(), random_repeat);
    }//TEST

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
}//namespace
