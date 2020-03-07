#ifndef CRDTS_LWWREGISTER_HH
#define CRDTS_LWWREGISTER_HH

#include <string>
#include "timestamp.hh"

/// A LWWRegister is a variant of a register, i.e., a memory cell that stores a value.
class LWWRegister {
private:
    Timestamp _timestamp;
    std::string _value;

public:
    /// Constructs a LWWRegister
    /// \param replica_id the replica
    explicit LWWRegister(uint64_t replica_id);

    /// Queries the value of the register
    /// \return the latest value
    std::string value();

    /// Assigns a given value to the register
    /// \param value the given value
    void assign(const std::string& value);

    /// Merge a given register with the local register
    /// \param reg the given register
    void merge(const LWWRegister& reg);

    /// Gets the replica id
    /// \return the replica's id
    uint64_t replica_id();

    /// Gets if the register has been initialized
    /// \return true if the register is initialized, otherwise false
    bool initialized();
};

#endif //CRDTS_LWWREGISTER_HH
