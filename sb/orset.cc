#include "orset.hh"

ORSet::ORSet(uint64_t replica_id) :
    _replica_id(replica_id) {
    _versions[_replica_id].update();
}

void ORSet::add(std::string &e) {
    auto elem = _observe.find(e);
    _observe[e][_replica_id] = _versions[_replica_id];
    _versions[_replica_id].update();
}

void ORSet::add(std::string&& e) {
    add(e);
}

void ORSet::remove(std::string& e) {
    _observe.erase(e);
}

void ORSet::remove(std::string&& e) {
    remove(e);
}

bool ORSet::contains(std::string &e) {
    return _observe.count(e) > 0;
}

bool ORSet::contains(std::string &&e) {
    return contains(e);
}

void ORSet::merge(ORSet &s) {

}

uint64_t ORSet::replica_id() {
    return _replica_id;
}
