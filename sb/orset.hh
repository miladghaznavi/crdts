#ifndef CRDTS_ORSET_HH
#define CRDTS_ORSET_HH

#include <unordered_map>
#include <unordered_set>
#include <string>
#include "timestamp.hh"

class OrSet {
private:
    std::unordered_map<std::string, std::unordered_set<Timestamp>> _observe;
    std::unordered_map<std::string, std::unordered_set<Timestamp>> _remove;
    Timestamp _unique_tag;

public:
    explicit OrSet(uint64_t replica_id);

    void add(std::string& e);

    void remove(std::string& e);

    bool lookup(std::string& e);

    void merge(OrSet& other);
};

#endif //CRDTS_ORSET_HH
