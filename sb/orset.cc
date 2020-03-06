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

void ORSet::merge(ORSet &src) {
    // Remove the elements that have been removed remotely.
    // If there are concurrent add operations, we use the
    // add-wins policy
    for (auto elem = this->_observe.begin(); elem != this->_observe.end(); ) {
        if (src._observe.count(elem->first) == 0) {
            // The element is either have not been delivered to the remote replica,
            // or a remote replica has removed it.
            // If latter is the case, the received timestamp of every replica must be
            // more recent
            bool older = false;
            for (const auto& e2: elem->second) {
                auto ff = src._versions.find(e2.first);
                if (ff == src._versions.end() and ff->second < e2.second) {
                    older = true;
                    break;
                }//if
            }//for
            if (older) {
                elem = this->_observe.erase(elem);
            }//if
            else {
                ++elem;
            }//else
        }//if
        else {
            ++elem;
        }//else
    }//for

    for (const auto& elem: src._observe) {
        auto local_elem = this->_observe.find(elem.first);

        // Add elements that the remote replica have added
        if (local_elem == this->_observe.end()) {
            bool more_recent = false;
            for (auto e2: elem.second) {
                // Check to see if a remote element has been added after
                // an observed remove operations
                if (this->_versions.count(e2.first) == 0 or
                    this->_versions[e2.first] < e2.second) {
                    more_recent = true;
                    break;
                }//if
            }//for

            if (more_recent) {
                this->_observe[elem.first] = elem.second;
            }//if
        }//if
        else {
            // The element already exists just update the timestamps
            for (auto e2: elem.second) {
                if (this->_versions.count(e2.first) == 0 or
                    this->_versions[e2.first] < e2.second) {
                    local_elem->second[e2.first] = e2.second;
                }//if
            }//for
        }//else
    }//for

    for (auto v: src._versions) {
        auto it = this->_versions.find(v.first);

        if (it != this->_versions.end())
            it->second = std::max(it->second, v.second);
        else
            this->_versions[v.first] = v.second;
    }//for
}

std::unordered_set<std::string> ORSet::elements() {
    std::unordered_set<std::string> result;

    for (const auto& elem: _observe)
        result.insert(elem.first);

    return result;
}

uint64_t ORSet::replica_id() {
    return _replica_id;
}
