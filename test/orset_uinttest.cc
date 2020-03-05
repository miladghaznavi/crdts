#include <gtest/gtest.h>
#include "../sb/orset.hh"

namespace {
    TEST(ORSet, Constructor) {
        #define REPLICA_ID 10
        ORSet set(REPLICA_ID);
        EXPECT_EQ(REPLICA_ID, set.replica_id());
    }//TEST

    TEST(ORSet, AddAndRemoveSingleReplica) {
        ORSet set(REPLICA_ID);

        set.add("A");
        EXPECT_TRUE(set.contains("A"));
        set.remove("A");
        EXPECT_FALSE(set.contains("A"));

        for (int i = 0; i < 10; ++i) {
            std::string b = "B" + std::to_string(i);
            set.add(b);
            EXPECT_TRUE(set.contains(b));

            set.remove(b);
            EXPECT_FALSE(set.contains(b));
        }//for
    }//TEST


}//namespace