#include <gtest/gtest.h>
#include "../statebased/lwwregister.hh"

namespace {
    #define REGISTER_TEST_CASES 1000

    TEST(LWWRegister, AssignAndValue) {
        #define REPLICA_ID 1
        LWWRegister reg;
        reg.replica_id(REPLICA_ID);

        for (auto i = 0; i < random() % REGISTER_TEST_CASES + 1; ++i) {
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
        reg1.replica_id(REPLICA1_ID);
        reg1.replica_id(REPLICA2_ID);

        // Multiple random tests select one of two registers randomly,
        // assign a random value to the selected register, and merge
        // the other register with the selected register.
        for (auto i = 0; i < REGISTER_TEST_CASES; ++i) {
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