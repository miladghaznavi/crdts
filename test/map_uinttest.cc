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

        auto size = random() % 100 + 1;
        for (auto i = 0; i < size; ++i) {
            map.put(std::to_string(i), std::to_string(i));
            EXPECT_EQ(i + 1, map.size());
        }//for
    }//TEST

    TEST(Map, PutAndContains) {
        // We compare Map with a C++ unordered_map as a reference
        Map map(REPLICA_ID);
        std::unordered_map<std::string, std::string> ref;

        for (int i = 0; i < MAP_TEST_CASES; ++i) {
            std::string k = std::to_string(random());
            std::string v = std::to_string(random());
            map.put(k, v);
            ref[k] = v;
        }//for

        // Check the existence of added elements
        for (auto e: ref) {
            EXPECT_TRUE(map.contains(e.first));
        }//for
    }//TEST

    TEST(Map, PutAndGet) {
        // We compare Map with a C++ unordered_map as a reference
        Map map(REPLICA_ID);
        std::unordered_map<std::string, std::string> ref;

        for (int i = 0; i < MAP_TEST_CASES; ++i) {
            std::string k = std::to_string(random());
            std::string v = std::to_string(random());
            map.put(k, v);
            ref[k] = v;
        }//for

        // Check the existence of added elements
        for (auto e: ref) {
            EXPECT_EQ(map.get(e.first), e.second);
        }//for
    }//TEST

    TEST(Map, PutAndRemoveAndConatinsAndGet) {
        // We compare Map with a C++ unordered_map as a reference
        Map map(REPLICA_ID);
        std::unordered_map<std::string, std::string> ref;
        std::vector<std::string> keys;

        for (int i = 0; i < MAP_TEST_CASES; ++i) {
            std::string k = std::to_string(random());
            std::string v = std::to_string(random());
            map.put(k, v);
            ref[k] = v;
            keys.push_back(k);
        }//for

        // Check the existence of added elements
        while (!keys.empty()) {
            auto rand_index = random() % keys.size();
            EXPECT_TRUE(map.contains(keys[rand_index]));
            EXPECT_EQ(map.get(keys[rand_index]), ref[keys[rand_index]]);

            map.remove(keys[rand_index]);
            EXPECT_FALSE(map.contains(keys[rand_index]));

            EXPECT_THROW(map.get(keys[rand_index]), std::exception);

            keys.erase(keys.begin() + rand_index);
        }//while
    }//TEST

    TEST(Map, Merge) {
        // We compare Map with a C++ unordered_map as a reference
        std::unordered_map<std::string, std::string> ref;
        std::vector<std::string> keys;

        Map map1(REPLICA_ID);
        #define REPLICA2_ID 2
        Map map2(REPLICA2_ID);

        for (int i = 0; i < MAP_TEST_CASES; ++i) {
            std::string k = std::to_string(random());
            std::string v = std::to_string(random());
            map1.put(k, v);

            auto old_size = ref.size();
            ref[k] = v;
            if (ref.size() > old_size)
                keys.push_back(k);
        }//for

        for (int i = 0; i < MAP_TEST_CASES; ++i) {
            std::string k = std::to_string(random());
            std::string v = std::to_string(random());
            map2.put(k, v);

            auto old_size = ref.size();
            ref[k] = v;
            if (ref.size() > old_size)
                keys.push_back(k);
        }//for

        map1.merge(map2);
        EXPECT_EQ(map1.size(), ref.size());
        for (auto kv: ref)
            EXPECT_EQ(map1.get(kv.first), kv.second);

        map2.merge(map1);
        EXPECT_EQ(map1.size(), ref.size());
        for (auto kv: ref)
            EXPECT_EQ(map2.get(kv.first), kv.second);

        // Merge random add and remove operations
        for (int i = 0; i < keys.size(); ++i) {
            // Select a set among set1 or set2
            auto first_or_second = random() % 2;
            auto* p = (first_or_second == 0) ? &map1 : &map2;

            // Call an add or remove operation of the selected set randomly
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
                keys.erase(keys.begin() + rand_ind);
            }//else
        }//for

        map1.merge(map2);
        for (auto kv: ref)
            EXPECT_EQ(map1.get(kv.first), kv.second);

        map2.merge(map1);
        for (auto kv: ref)
            EXPECT_EQ(map2.get(kv.first), kv.second);
    }//TEST
}//namespace