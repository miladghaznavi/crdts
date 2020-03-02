#include <gtest/gtest.h>
#include "../sb/timestamp.hh"

namespace {
    TEST(Timestamp, DefaultConstructor) {
        #define REPLICA1_ID 1
        Timestamp t(REPLICA1_ID);

        EXPECT_EQ(REPLICA1_ID, t.replica_id());
        EXPECT_TRUE(t.beginning_of_time());
    }//TEST

    TEST(Timestamp, Update) {
        #define REPLICA1_ID 1
        Timestamp t(REPLICA1_ID);
        EXPECT_TRUE(t.beginning_of_time());

        t.update();
        EXPECT_FALSE(t.beginning_of_time());
    }

    TEST(Timestamp, Copy) {
        #define REPLICA1_ID 1
        #define REPLICA2_ID 2

        Timestamp t1(REPLICA1_ID);
        Timestamp t2(REPLICA2_ID);

        EXPECT_TRUE(t1 < t2);
        EXPECT_FALSE(t2 < t1);

        t2.copy(t1);
        EXPECT_FALSE(t1 < t2);
        EXPECT_FALSE(t2 < t1);

        t1.update();
        t2.update();
        EXPECT_TRUE(t1 < t2);
        EXPECT_FALSE(t2 < t1);
    }
}//namespace
