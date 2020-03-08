#include <gtest/gtest.h>
#include "../statebased/orset.hh"

namespace {
    #define SET_TEST_CASES 1000
    #define SET_MULTI_OBJECTS 100

    TEST(ORSet, Constructor) {
        #define REPLICA_ID 10
        ORSet set(REPLICA_ID);
        EXPECT_EQ(REPLICA_ID, set.replica_id());
    }//TEST

    TEST(ORSet, AddAndRemoveContainSingleReplica) {
        ORSet set(REPLICA_ID);
        // Use an C++ unordered_set as a reference for testing
        std::unordered_set<std::string> ref;

        // Add random values to the set
        for (int i = 0; i < SET_TEST_CASES; ++i) {
            std::string b = std::to_string(random());
            set.add(b);
            ref.insert(b);
        }//for

        // Check the existence of added elements
        for (const auto& e: ref) {
            EXPECT_TRUE(set.contains(e));
        }//for

        // Check the existence of removed elements
        for (const auto& e: ref) {
            set.remove(e);
            EXPECT_FALSE(set.contains(e));
        }//for
    }//TEST

    TEST(ORSet, Elements) {
        ORSet set(REPLICA_ID);
        // Use C++ unordered_set and vector as references
        std::unordered_set<std::string> ref;
        std::vector<std::string> keys;

        // Adding random elements to set
        for (int i = 0; i < SET_TEST_CASES; ++i) {
            std::string b = std::to_string(random());
            set.add(b);

            auto old_size = ref.size();
            ref.insert(b);
            if (ref.size() > old_size)
                keys.push_back(b);
        }//for

        auto elems = set.elements();

        // Check if all random elements exist in `elems`
        for (int i = 0; i < ref.size(); i++) {
            EXPECT_TRUE(ref.count(*elems.begin()));
            elems.erase(elems.begin());
        }//for

        // Remove some elements randomly
        auto to_remove = random() % ref.size() + 1;
        for (int i = 0; i < to_remove; ++i) {
            auto ind = random() % keys.size();
            ref.erase(keys[ind]);
            set.remove(keys[ind]);

            std::swap(keys[ind], keys[keys.size() - 1]);
            keys.pop_back();
        }//for

        elems = set.elements();

        // Check if all remaining elements exist in `elems`
        for (int i = 0; i < elems.size(); i++) {
            EXPECT_TRUE(ref.count(*elems.begin()));
            elems.erase(elems.begin());
        }//for
    }//TEST

    TEST(ORSet, Merge) {
        // We compare an ORSet with a C++ unordered_set and vector as references
        std::unordered_set<std::string> ref;
        std::vector<std::string> keys;

        ORSet set1(REPLICA_ID);
        #define REPLICA2_ID 2
        ORSet set2(REPLICA2_ID);

        // Merging two empty set
        set1.merge(set2);
        set2.merge(set1);
        EXPECT_EQ(0, set1.size());
        EXPECT_EQ(set1.size(), set2.size());

        // Merging an empty set with another set
        for (int i = 0; i < SET_TEST_CASES; ++i) {
            std::string b = std::to_string(random());
            set1.add(b);

            auto old_size = ref.size();
            ref.insert(b);
            if (ref.size() > old_size)
                keys.push_back(b);
        }//for
        set2.merge(set1);
        auto elems = set2.elements();
        EXPECT_TRUE(elems == ref);

        // Remove and then merge
        for (int i = 0; i < elems.size() / 2; ++i) {
            auto rand_ind = random() % keys.size();
            set1.remove(keys[rand_ind]);
            ref.erase(keys[rand_ind]);
            std::swap(keys[rand_ind], keys[keys.size() - 1]);
            keys.pop_back();
        }//for
        EXPECT_TRUE(ref == set1.elements());
        set2.merge(set1);
        EXPECT_TRUE(set1.elements() == set2.elements());

        // Merge random add and remove operations
        for (int i = 0; i < elems.size(); ++i) {
            // Select a set among set1 or set2
            auto first_or_second = random() % 2;
            ORSet* p = (first_or_second == 0) ? &set1 : &set2;

            // Call an add or remove operation of the selected set randomly
            auto add_or_remove = random() % 2;
            if (add_or_remove == 0) {//add
                std::string b = std::to_string(random());
                p->add(b);

                auto old_size = ref.size();
                ref.insert(b);
                if (ref.size() > old_size)
                    keys.push_back(b);
            }//if
            else {//remove
                auto rand_ind = random() % keys.size();
                p->remove(keys[rand_ind]);
                ref.erase(keys[rand_ind]);
                std::swap(keys[rand_ind], keys[keys.size() - 1]);
                keys.pop_back();
            }//else
        }//for
        set1.merge(set2);
        set2.merge(set1);
        EXPECT_TRUE(set1.elements() == set2.elements());

        // Check add-wins policy: a more recent remove operation must win over an older add operation
        for (auto i = 0; i < SET_TEST_CASES / 10 + 1; ++i) {
            auto b = std::to_string(random());
            set1.add(b);
            set2.merge(set1);
            EXPECT_TRUE(set1.contains(b));
            EXPECT_TRUE(set2.contains(b));

            set2.remove(b);
            set1.merge(set2);
            EXPECT_FALSE(set1.contains(b));
            EXPECT_FALSE(set2.contains(b));
        }//for

        // Check add-wins policy: an add operation must win over concurrent remove operation
        for (auto i = 0; i < SET_TEST_CASES / 10 + 1; ++i) {
            auto b = std::to_string(random());
            set1.add(b);
            EXPECT_TRUE(set1.contains(b));

            set2.add(b);
            EXPECT_TRUE(set2.contains(b));

            set2.remove(b);
            EXPECT_FALSE(set2.contains(b));

            set1.merge(set2);
            EXPECT_TRUE(set1.contains(b));

            set2.add(b);
            EXPECT_TRUE(set2.contains(b));

            set2.remove(b);
            EXPECT_FALSE(set2.contains(b));

            // b must reappear in set2
            set2.merge(set1);
            EXPECT_TRUE(set2.contains(b));
        }//for
    }//TEST
}//namespace
