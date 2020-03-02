#include <gtest/gtest.h>
#include "../sb/lwwregister.hh"

namespace {
    TEST(LWWRegister, Constructor) {
        #define REPLICA_ID 1
        LWWRegister reg(REPLICA_ID);
        EXPECT_EQ(REPLICA_ID, reg.replica_id());
        EXPECT_FALSE(reg.initialized());
    }//TEST

    TEST(LWWRegister, AssignAndValue) {
        #define REPLICA_ID 1
        LWWRegister reg(REPLICA_ID);

        reg.assign("1");
        EXPECT_EQ("1", reg.value());

        reg.assign("2");
        EXPECT_EQ("2", reg.value());
        EXPECT_EQ("2", reg.value());

        reg.assign("5");
        EXPECT_EQ("5", reg.value());
        EXPECT_NE("2", reg.value());
    }//TEST

    TEST(LWWRegister, Merge) {
        #define REPLICA1_ID 1
        #define REPLICA2_ID 2
        LWWRegister reg1(REPLICA1_ID);
        LWWRegister reg2(REPLICA2_ID);

        reg1.assign("1");
        reg2.assign("2");
        reg1.merge(reg2);
        EXPECT_EQ("2", reg1.value());
        EXPECT_EQ("2", reg2.value());

        reg1.assign("3");
        reg2.merge(reg1);
        EXPECT_EQ("3", reg1.value());
        EXPECT_EQ("3", reg2.value());

        reg2.assign("4");
        reg1.merge(reg2);
        EXPECT_EQ("4", reg1.value());
        EXPECT_EQ("4", reg2.value());
    }//TEST
}//namespace