#include <gtest/gtest.h>
#include "../sb/orset.hh"

namespace {
    TEST(ORSet, Constructor) {
        #define REPLICA_ID 10
        ORSet set(REPLICA_ID);
        EXPECT_EQ(REPLICA_ID, set.replica_id());
    }//TEST

    TEST(ORSet, AddAndRemoveContainSingleReplica) {
        ORSet set(REPLICA_ID);
        std::unordered_set<std::string> ref;

        for (int i = 0; i < 10; ++i) {
            std::string b = std::to_string(random());
            set.add(b);
            ref.insert(b);
        }//for

        // Check the existence of added elements
        for (auto e: ref) {
            EXPECT_TRUE(set.contains(e));
        }//for

        // Check the existence of removed elements
        for (auto e: ref) {
            set.remove(e);
            EXPECT_FALSE(set.contains(e));
        }//for
    }//TEST

    TEST(ORSet, Elements) {
        ORSet set(REPLICA_ID);

        std::unordered_set<std::string> ref;
        for (int i = 0; i < 10; ++i) {
            std::string b = "B" + std::to_string(i);
            set.add(b);
            ref.insert(b);
        }//for

        auto elems = set.elements();
        for (int i = 0; i < ref.size(); i++) {
            EXPECT_TRUE(ref.count(*elems.begin()));
            elems.erase(elems.begin());
        }//for

        auto to_remove = random() % ref.size() + 1;
        for (int i = 0; i < to_remove; ++i) {
            auto ind = random() % 10;
            std::string b = "B" + std::to_string(ind);
            ref.erase(b);
            set.remove(b);
        }//for

        elems = set.elements();
        for (int i = 0; i < elems.size(); i++) {
            EXPECT_TRUE(ref.count(*elems.begin()));
            elems.erase(elems.begin());
        }//for
    }//TEST

    TEST(ORSet, Merge) {
        // We compare an ORSet with a C++ unordered_set as a reference
        std::unordered_set<std::string> ref;

        ORSet set1(REPLICA_ID);
        #define REPLICA2_ID 2
        ORSet set2(REPLICA2_ID);

        // Merging two empty set
        set1.merge(set2);
        set2.merge(set1);
        EXPECT_EQ(0, set1.size());
        EXPECT_EQ(set1.size(), set2.size());

        // Merging an empty set with another set
        for (int i = 0; i < 10; ++i) {
            std::string b = "B" + std::to_string(i);
            set1.add(b);
            ref.insert(b);
        }//for
        set2.merge(set1);
        auto elems = set2.elements();
        EXPECT_TRUE(elems == ref);

        // Merge random add and remove operations
        for (int i = 0; i < elems.size(); ++i) {
            // Select a set among set1 or set2
            auto first_or_second = random() % 2;
            ORSet* p = (first_or_second == 0) ? &set1 : &set2;

            // Call an add or remove operation of the selected set randomly
            auto add_or_remove = random() % 2;
            if (add_or_remove == 0) {//add
                std::string d = "D" + std::to_string(i);
                p->add(d);

                ref.insert(d);
            }//if
            else {//remove
                auto b_or_c = random() % 2;
                auto val = random() % 10;
                std::string bc = ((b_or_c) ? "B" : "C") + std::to_string(val);

                p->remove(bc);
                ref.erase(bc);
            }//else
        }//for
        set1.merge(set2);
        set2.merge(set1);
        EXPECT_TRUE(ref == set1.elements());
        EXPECT_TRUE(set1.elements() == set2.elements());

        // Check add-wins policy
        // Concurrent add and remove, merging with add operation
        std::string add_wins = "add-wins";
        set1.add(add_wins);
        set2.remove(add_wins);
        set2.merge(set1);
        EXPECT_TRUE(set2.contains(add_wins));

        // Concurrent add and remove, merging with remove operation
        add_wins = "add-wins-2";
        set1.add(add_wins);
        set2.remove(add_wins);
        set1.merge(set2);
        EXPECT_TRUE(set1.contains(add_wins));

        // Concurrent add and remove, merge both sets, first add then remove
        add_wins = "add-wins-3";
        set1.add(add_wins);
        set2.remove(add_wins);
        set1.merge(set2);
        set2.merge(set1);
        EXPECT_TRUE(set1.contains(add_wins));
        EXPECT_TRUE(set2.contains(add_wins));

        // Concurrent add and remove, merging both sets, first remove then add
        add_wins = "add-wins-4";
        set1.add(add_wins);
        set2.remove(add_wins);
        set2.merge(set1);
        set1.merge(set2);
        EXPECT_TRUE(set1.contains(add_wins));
        EXPECT_TRUE(set2.contains(add_wins));
    }//TEST
}//namespace