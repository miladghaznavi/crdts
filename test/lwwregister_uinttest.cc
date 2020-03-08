#include <gtest/gtest.h>
#include "../sb/lwwregister.hh"

namespace {
    TEST(LWWRegister, AssignAndValue) {
        #define REPLICA_ID 1
        LWWRegister reg;
        reg.init_unique_id(REPLICA_ID);

        for (auto i = 0; i < random() % 100 + 1; ++i) {
            auto rand_val = random();
            reg.assign(std::to_string(rand_val));
            EXPECT_EQ(std::to_string(rand_val), reg.value());
        }//for
    }//TEST

    TEST(LWWRegister, Merge) {
        #define REPLICA1_ID 1
        #define REPLICA2_ID 2
        LWWRegister reg1;
        LWWRegister reg2;
        reg1.init_unique_id(REPLICA1_ID);
        reg1.init_unique_id(REPLICA2_ID);

        // Try 100 random test where one of two registers is selected randomly,
        // is assigned a random value, and merged with the other register.
        for (auto i = 0; i < 100; ++i) {
            LWWRegister* f = &reg1;
            LWWRegister* s = &reg2;

            auto swap = random() % 2;
            if (swap == 1)
                std::swap(s, f);

            auto rand_val = random();
            f->assign(std::to_string(rand_val));
            s->merge(*f);
            EXPECT_EQ(f->value(), s->value());
        }//for
    }//TEST
}//namespace