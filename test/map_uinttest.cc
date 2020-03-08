#include <gtest/gtest.h>
#include "../statebased/map.hh"

namespace {
    #define MAP_TEST_CASES 1000

    TEST(Map, Constructor) {
        #define REPLICA_ID 10
        Map map(REPLICA_ID);
        EXPECT_EQ(REPLICA_ID, map.replica_id());
    }//TEST

    TEST(Map, Size) {
        Map map(REPLICA_ID);
        EXPECT_EQ(REPLICA_ID, map.replica_id());
        EXPECT_EQ(0, map.size());

        auto size = random() % MAP_TEST_CASES + 1;
        for (auto i = 0; i < size; ++i) {
            map.put(std::to_string(i), std::to_string(i));
            EXPECT_EQ(i + 1, map.size());
        }//for
    }//TEST

    TEST(Map, PutAndContains) {
        // We compare Map with a C++ unordered_map as a reference
        Map map(REPLICA_ID);
        std::unordered_map<std::string, std::string> ref;

        // Put random key value pairs to the map
        for (int i = 0; i < MAP_TEST_CASES; ++i) {
            std::string k = std::to_string(random());
            std::string v = std::to_string(random());
            map.put(k, v);
            ref[k] = v;
        }//for

        // Check the existence of added elements
        for (const auto& e: ref) {
            EXPECT_TRUE(map.contains(e.first));
        }//for
    }//TEST

    TEST(Map, PutAndGet) {
        // We compare Map with a C++ unordered_map as a reference
        Map map(REPLICA_ID);
        std::unordered_map<std::string, std::string> ref;

        // Put random key value pairs to the map
        for (int i = 0; i < MAP_TEST_CASES; ++i) {
            std::string k = std::to_string(random());
            std::string v = std::to_string(random());
            map.put(k, v);
            ref[k] = v;
        }//for

        // Check the existence of added elements
        for (const auto& e: ref) {
            EXPECT_EQ(map.get(e.first), e.second);
        }//for
    }//TEST

    TEST(Map, PutAndRemoveAndConatinsAndGet) {
        // We compare Map with a C++ unordered_map and vector as references
        Map map(REPLICA_ID);
        std::unordered_map<std::string, std::string> ref;
        std::vector<std::string> keys;

        // Add random key value pairs to the map
        for (int i = 0; i < MAP_TEST_CASES; ++i) {
            std::string k = std::to_string(random());
            std::string v = std::to_string(random());
            map.put(k, v);

            auto old_size = ref.size();
            ref[k] = v;
            if (ref.size() > old_size)
                keys.push_back(k);
        }//for

        // Check the existence of added elements
        // Random remove and get operations
        while (!keys.empty()) {
            auto rand_index = random() % keys.size();
            EXPECT_TRUE(map.contains(keys[rand_index]));
            EXPECT_EQ(map.get(keys[rand_index]), ref[keys[rand_index]]);

            map.remove(keys[rand_index]);
            EXPECT_FALSE(map.contains(keys[rand_index]));

            EXPECT_THROW(map.get(keys[rand_index]), std::exception);

            std::swap(keys[keys.size() - 1], keys[rand_index]);
            keys.pop_back();
        }//while
    }//TEST

    TEST(Map, Merge) {
        // We compare Map with a C++ unordered_map and vector as references
        std::unordered_map<std::string, std::string> ref;
        std::vector<std::string> keys;

        // Define the first map and put random key value pairs to it
        Map map1(REPLICA_ID);
        for (int i = 0; i < MAP_TEST_CASES; ++i) {
            std::string k = std::to_string(random());
            std::string v = std::to_string(random());
            map1.put(k, v);

            auto old_size = ref.size();
            ref[k] = v;
            if (ref.size() > old_size)
                keys.push_back(k);
        }//for

        // Define the second map and put random key value pairs to it
        #define REPLICA2_ID 2
        Map map2(REPLICA2_ID);
        for (int i = 0; i < MAP_TEST_CASES; ++i) {
            std::string k = std::to_string(random());
            std::string v = std::to_string(random());
            map2.put(k, v);

            auto old_size = ref.size();
            ref[k] = v;
            if (ref.size() > old_size)
                keys.push_back(k);
        }//for

        // Merge map1 with map2, then map1 must include all key value pairs
        map1.merge(map2);
        EXPECT_EQ(map1.size(), ref.size());
        for (const auto& kv: ref)
            EXPECT_EQ(map1.get(kv.first), kv.second);

        // Merge map2 with map1, then map2 must include all key value pairs
        map2.merge(map1);
        EXPECT_EQ(map1.size(), ref.size());
        for (const auto& kv: ref)
            EXPECT_EQ(map2.get(kv.first), kv.second);

        // Merge random add and remove operations
        for (int i = 0; i < keys.size(); ++i) {
            // Select a set among set1 or set2
            auto first_or_second = random() % 2;
            auto* p = (first_or_second == 0) ? &map1 : &map2;

            // Randomly call an add or remove operation of the selected set
            auto add_or_remove = random() % 2;
            if (add_or_remove == 0) {//add
                std::string d = std::to_string(random());
                p->put(d, d);

                auto old_size = ref.size();
                ref[d] = d;
                if (ref.size() > old_size)
                    keys.push_back(d);
            }//if
            else {//remove
                auto rand_ind = random() % keys.size();
                p->remove(keys[rand_ind]);

                ref.erase(keys[rand_ind]);

                std::swap(keys[rand_ind], keys[keys.size() - 1]);
                keys.pop_back();
            }//else
        }//for
        map1.merge(map2);
        for (const auto& kv: ref)
            EXPECT_EQ(map1.get(kv.first), kv.second);
        map2.merge(map1);
        for (const auto& kv: ref)
            EXPECT_EQ(map2.get(kv.first), kv.second);

        // Merge random add and remove operations
        for (int i = 0; i < keys.size() / 10; ++i) {
            // Select a set among set1 or set2
            auto first_or_second = random() % 2;
            auto* p = (first_or_second == 0) ? &map1 : &map2;

            // Randomly call an add or remove operation of the selected set
            auto add_or_remove = random() % 2;
            if (add_or_remove == 0) {//add
                std::string d = std::to_string(random());
                p->put(d, d);

                auto old_size = ref.size();
                ref[d] = d;
                if (ref.size() > old_size)
                    keys.push_back(d);
            }//if
            else {//remove
                auto rand_ind = random() % keys.size();
                p->remove(keys[rand_ind]);

                ref.erase(keys[rand_ind]);

                std::swap(keys[rand_ind], keys[keys.size() - 1]);
                keys.pop_back();
            }//else

            auto merge = random() % 4;
            if (merge > 0) {
                auto* other = &map2;
                if (p == &map2)
                    other = &map1;

                auto p_copy = *p;
                auto other_copy = *other;

                other->merge(p_copy);
                p->merge(other_copy);

                EXPECT_EQ(map1.size(), map2.size());
                EXPECT_TRUE(map1.key_value_pairs() == map2.key_value_pairs());
            }//if
        }//for

        map1.merge(map2);
        map2.merge(map1);

        EXPECT_EQ(map1.size(), map2.size());
        EXPECT_TRUE(map1.key_value_pairs() == map2.key_value_pairs());
    }//TEST
}//namespace