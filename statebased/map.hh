#ifndef CRDTS_MAP_HH
#define CRDTS_MAP_HH

#include <unordered_map>
#include "orset.hh"
#include "lwwregister.hh"

/// Map implements a convergent map with the ``add wins'' policy for keys and ``last writer wins'' policy for values.
/// Map maintains keys in an ORSet to implement the former policy. By keeping each value in a LWWRegister, Map
/// implements the latter policy.
class Map {
private:
    ORSet _keys;
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

    /// Checks the existence of a given key
    /// \param key the given key
    /// \return true if the key exists, otherwise false
    bool contains(const std::string& key);

    /// Gets the number of key value pairs in the map
    /// \return the number of key value pairs
    size_t size();

    /// Gets the replica id
    /// \return the replica id
    uint64_t replica_id();

    /// Gets the key value pairs stored in the map
    /// \return all key value pairs
    std::unordered_map<std::string, std::string> key_value_pairs();
};

#endif //CRDTS_MAP_HH
