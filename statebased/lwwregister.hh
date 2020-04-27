#ifndef CRDTS_LWWREGISTER_HH
#define CRDTS_LWWREGISTER_HH

#include "../core/timestamp.hh"

/// A LWWRegister is a variant of a register, i.e., a memory cell that stores a value.
/// LWWRegister implements the ``last write wins'' policy, where among concurrent write
/// operation across replicas, the latest one -- based on a global ordering -- wins the
/// race. A LWWRegister contains a Timestamp object that globally order operations.
template<typename ValueType>
class LWWRegister {
private:
    Timestamp _timestamp;
    ValueType _value;

public:
    /// Sets the replica id of the register.
    /// Note that this function is for test purposes and must be called right after constructing a register.
    /// An internal call can replace this function, where the internal call uses the mac address of an network
    /// interface as the unique identifier of the register unique tag
    /// \param replica_id the given replica id
    void replica_id(uint64_t replica_id) {
        this->_timestamp.replica_id(replica_id);
    }

    /// Queries the value of the register
    /// \return the latest value
    ValueType value() const {
        return _value;
    }

    /// Assigns a given value to the register
    /// \param value the given value
    void assign(const ValueType& value) {
        this->_timestamp.update();
        this->_value = value;
    }
    void assign(const ValueType&& value) {
        this->assign(value);
    }

    /// Merges a given register with the local register
    /// \param reg the given register
    void merge(const LWWRegister<ValueType>& reg) {
        if (this->_timestamp < reg._timestamp) {
            this->_value = reg._value;
            this->_timestamp.copy(reg._timestamp);
        }//if
    }

    /// Gets the replica id
    /// \return the replica's id
    uint64_t replica_id() const {
        return this->_timestamp.replica_id();
    }
};

#endif //CRDTS_LWWREGISTER_HH
