#include "map.hh"


Map::Map(uint64_t replica_id) : _keys(replica_id) {

}

void Map::put(const std::string &key, const std::string &val) {
    _keys.add(key);
    _registers[key].assign(val);
}

std::string Map::get(const std::string &key) {
    if (!_keys.contains(key))
        throw "Key does not exist!";

    return _registers[key].value();
}

void Map::remove(const std::string &key) {
    _keys.remove(key);
    _registers.erase(key);
}

void Map::merge(const Map &map) {
    ORSet keys_copy = this->_keys;

    // Merge keys
    this->_keys.merge(map._keys);

    // Remove keys deleted in merging keys (above)
    for (const auto& k: keys_copy.elements()) {
        if (!this->_keys.contains(k)) {
            this->_registers.erase(k);
        }//if
    }//for

    // Merge registers associated with remaining keys
    for (const auto& remote_reg: map._registers) {
        auto local_reg = this->_registers.find(remote_reg.first);
        if (local_reg != this->_registers.end()) {
            // The associated register exists locally, merge it with the remote register
            local_reg->second.merge(remote_reg.second);
        }//if
        else if (this->_keys.contains(remote_reg.first)) {
            // TODO: Find a more intuitive implementation because reasoning about the correctness of the
            //  current implementation is complicated. The current implementation is correct because
            //  the local register will have the same value as the remote register, although the local
            //  register may win in merge. Since registers do not expose their unique tags (timestamps),
            //  the local and remote values are consistent to outside world. Future merges will be still
            //  valid.
            // The register associate to the remote key does not exist locally, add the associated register
            // We initialize the local register with the remote value, then merge two registers.
            this->_registers[remote_reg.first].assign(remote_reg.second.value());
            this->_registers[remote_reg.first].merge(remote_reg.second);
        }//else if
    }//for
}
