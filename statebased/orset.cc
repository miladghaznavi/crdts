#include "orset.hh"

#include <iostream>
#include <vector>

ORSet::ORSet(uint64_t replica_id) {
    this->_replica_id = replica_id;
    // Initialize the local version
    this->_versions[_replica_id].replica_id(this->_replica_id);
}

void ORSet::add(const std::string &e) {
    _versions[_replica_id].update();
    _elements[e][_replica_id] = _versions[_replica_id];
}

bool ORSet::remove(const std::string& e) {
    return _elements.erase(e) > 0;
}

bool ORSet::contains(const std::string &e) {
    return _elements.count(e) > 0;
}

void ORSet::_merge_versions(const ORSet& remote_set) {
    for (auto v: remote_set._versions) {
        auto it = this->_versions.find(v.first);

        if (it != this->_versions.end())
            it->second = std::max(it->second, v.second);
        else
            this->_versions[v.first] = v.second;
    }//for
}

void ORSet::_apply_remote_removes(const ORSet& remote_set) {
//    std::vector<std::string> removed;

    // Remove the elements that have been removed remotely.
    // Add-wins policy is applied for concurrent add and remove operations
    for (auto local_elem = this->_elements.begin(); local_elem != this->_elements.end(); /* no increment here */) {
        // If the element has been removed remotely if it does not exist in the remote set.
        if (remote_set._elements.count(local_elem->first) == 0) {
            // Applying the add-wins policy to see if there is a concurrent or newer add
            bool newer_remote_remove = true;
            for (const auto& observed_add: local_elem->second) {
                auto remote_remove = remote_set._versions.find(observed_add.first);

                if (remote_remove == remote_set._versions.end() or remote_remove->second < observed_add.second) {
                    newer_remote_remove = false;
                    break;
                }//if
            }//for

            if (newer_remote_remove) {
                // Find an evidence that there has been a remote remove by examining the versions of replicas observed
                // in the remove set
                newer_remote_remove = false;
                for (const auto &remote_remove: remote_set._versions) {
                    auto local_add = local_elem->second.find(remote_remove.first);
                    // We have an evidence if the timestamp associated to the replica id
                    // has not been seen locally or this timestamp is newer than observed timestamp locally
                    if (local_add == local_elem->second.end() or local_add->second < remote_remove.second) {
                        // Evidence is found, let's break the search
                        newer_remote_remove = true;
                        break;
                    }//if
                }//for
            }//if

            if (newer_remote_remove) {
//                removed.push_back(local_elem->first);

                // Remove the element that has been remotely removed, and move to next local element
                local_elem = this->_elements.erase(local_elem);
            }//if
            else {
                ++local_elem;
            }//else
        }//if
        else {
            ++local_elem;
        }//else
    }//for

//    std::cout << "Removed:" << std::endl;
//    for (auto r: removed) {
//        std::cout << r << std::endl;
//    }
}

void ORSet::_apply_remote_adds(const ORSet& remote_set) {
    for (const auto& remote_elem: remote_set._elements) {
        auto local_elem = this->_elements.find(remote_elem.first);

        // Check if the remote_set element exists locally
        if (local_elem != this->_elements.end()) {
            // The element already exists, update the timestamps
            for (auto remote_timestamp: remote_elem.second) {
                // Update a local timestamp with a remote timestamp that is more recent
                if (this->_versions.count(remote_timestamp.first) == 0 or
                    this->_versions[remote_timestamp.first] < remote_timestamp.second) {
                    local_elem->second[remote_timestamp.first] = remote_timestamp.second;
                }//if
            }//for
        }//if
        else {
            // The element does not exist locally,
            bool more_recent = false;
            for (auto remote_timestamp: remote_elem.second) {
                // Check if a remote timestamp is newer the a local timestamp
                // A single remote timestamp is sufficient because of add-wins policy
                if (this->_versions.count(remote_timestamp.first) == 0 or
                    this->_versions[remote_timestamp.first] < remote_timestamp.second) {
                    more_recent = true;
                    break;
                }//if
            }//for

            // Add a new element that was added remotely
            if (more_recent) {
                this->_elements[remote_elem.first] = remote_elem.second;
            }//if
        }//else
    }//for
}

void ORSet::merge(const ORSet &remote_set) {
    // apply remote remove operations
    _apply_remote_removes(remote_set);

    // apply remote add operations
    _apply_remote_adds(remote_set);

    // merge versions
    _merge_versions(remote_set);
}

std::unordered_set<std::string> ORSet::elements() const {
    std::unordered_set<std::string> result;

    for (const auto& elem: _elements)
        result.insert(elem.first);

    return result;
}

uint64_t ORSet::replica_id() {
    return _replica_id;
}

size_t ORSet::size() {
    return _elements.size();
}
