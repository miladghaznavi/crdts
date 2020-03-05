#ifndef CRDTS_ORSET_HH
#define CRDTS_ORSET_HH

#include <unordered_map>
#include <unordered_set>
#include <string>
#include "timestamp.hh"

class ORSet {
private:
    std::unordered_map<std::string, std::unordered_map<uint64_t, Timestamp>> _observe;
    std::unordered_map<uint64_t, Timestamp> _versions;
    uint64_t _replica_id;

public:
    explicit ORSet(uint64_t replica_id);

    void add(std::string& e);

    void add(std::string&& e);

    void remove(std::string& e);

    void remove(std::string&& e);

    bool contains(std::string& e);

    bool contains(std::string&& e);

    void merge(ORSet& other);

    std::unordered_set<std::string> elements();

    uint64_t replica_id();
};

#endif //CRDTS_ORSET_HH
