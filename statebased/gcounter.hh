#ifndef CRDTS_LWWREGISTER_HH
#define CRDTS_LWWREGISTER_HH

#include <unordered_map>

template<typename ValueType>
class GCounter {
    static_assert(std::is_base_of<char, ValueType>::value, "ValueType must extend integer");
    static_assert(std::is_base_of<short, ValueType>::value, "ValueType must extend integer");
    static_assert(std::is_base_of<int, ValueType>::value, "ValueType must extend integer");
    static_assert(std::is_base_of<long, ValueType>::value, "ValueType must extend integer");
    static_assert(std::is_base_of<long long, ValueType>::value, "ValueType must extend integer");

    static_assert(std::is_base_of<unsigned char, ValueType>::value, "ValueType must extend integer");
    static_assert(std::is_base_of<unsigned short, ValueType>::value, "ValueType must extend integer");
    static_assert(std::is_base_of<unsigned int, ValueType>::value, "ValueType must extend integer");
    static_assert(std::is_base_of<unsigned long, ValueType>::value, "ValueType must extend integer");
    static_assert(std::is_base_of<unsigned long long, ValueType>::value, "ValueType must extend integer");

private:
    uint64_t _replica_id;
    std::unordered_map<uint64_t, ValueType> _counters;

public:
    /// Sets the replica id of the register.
    /// Note that this function is for test purposes and must be called right after constructing a register.
    /// An internal call can replace this function, where the internal call uses the mac address of an network
    /// interface as the unique identifier of the register unique tag
    /// \param replica_id the given replica id
    void replica_id(uint64_t replica_id) {
        this->_replica_id = replica_id;
    }

    /// Queries the value of the register
    /// \return the latest value
    ValueType value() const {
        ValueType re = 0;
        for (auto kv: _counters)
            re += kv.second;

        return re;
    }

    /// Increment the counter
    void increment() const {
        _counters[_replica_id]++;
    }

    /// Merges a given register with the local register
    /// \param reg the given register
    void merge(const GCounter<ValueType>& cnt) {
        // TODO: What happens to new replica ids?
        for (auto c: cnt._counters)
            this->_counters[c.first] = std::max(this->_counters[c.first], c.second);
    }

    /// Gets the replica id
    /// \return the replica's id
    uint64_t replica_id() const {
        return this->_replica_id;
    }
};

#endif //CRDTS_LWWREGISTER_HH
