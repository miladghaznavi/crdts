#include "map.hh"

Map::Map(uint64_t replica_id) : _keys(replica_id) { }

void Map::put(const std::string &key, const std::string &val) {
    _keys.add(key);
    if (_registers.count(key) == 0) {
        // initialize a register for the first time
        _registers[key].init_unique_id(_keys.replica_id());
    }//if

    _registers[key].assign(val);
}

std::string Map::get(const std::string &key) {
    if (!this->contains(key)) {
        throw std::exception();
    }//if

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
            // The associated register locally exists, merge it with the remote register
            local_reg->second.merge(remote_reg.second);
        }//if
        else if (this->_keys.contains(remote_reg.first)) {
            // The register associate to the remote key does not locally exist, add the associated register.
            // We initialize the local register with the remote value, then merge two registers.
            this->_registers[remote_reg.first].init_unique_id(this->replica_id());
            this->_registers[remote_reg.first].assign(remote_reg.second.value());
            this->_registers[remote_reg.first].merge(remote_reg.second);
        }//else if
    }//for
}

bool Map::contains(const std::string& key) {
    return _keys.contains(key);
}

size_t Map::size() {
    return _keys.size();
}

uint64_t Map::replica_id() {
    return _keys.replica_id();
}

std::unordered_map<std::string, std::string> Map::key_value_pairs() {
    std::unordered_map<std::string, std::string> res;
    for (const auto& kv: _registers)
        res[kv.first] = kv.second.value();

    return res;
}