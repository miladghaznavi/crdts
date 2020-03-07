#ifndef CRDTS_MAP_HH
#define CRDTS_MAP_HH

#include <unordered_map>
#include "orset.hh"
#include "lwwregister.hh"

class Map {
private:
    ORSet _keys;
    // TODO: does not work because the constructor of LWWRegister expects a replica id
    std::unordered_map<std::string, LWWRegister> _registers;

public:
    /// Creates a map object with a given replica id
    /// \param replica_id the given replica id
    explicit Map(uint64_t replica_id);

    /// Puts a given key and value pair to the map
    /// \param key the given key
    /// \param val the given value
    void put(const std::string& key, const std::string& val);

    /// Gets the value of a given key
    /// \param key the given key
    /// \return the value
    std::string get(const std::string& key);

    /// Removes a given key from the map
    /// \param key the given key
    void remove(const std::string& key);

    /// Merges a given map with the local map
    /// \param map the given map
    void merge(const Map& map);
};

#endif //CRDTS_MAP_HH
