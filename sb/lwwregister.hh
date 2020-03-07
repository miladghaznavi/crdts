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
    /// Initialize the unique tag of the register with a given replica id.
    /// This function is for test purposes and must be called right after constructing a register.
    /// An internal call can replace this function, where the internal call uses the mac address of an network
    /// interface as the unique identifier of the register unique tag
    /// \param replica_id the given replica id
    void init_unique_tag(uint64_t replica_id);

    /// Queries the value of the register
    /// \return the latest value
    std::string value() const;

    /// Assigns a given value to the register
    /// \param value the given value
    void assign(const std::string& value);

    /// Merge a given register with the local register
    /// \param reg the given register
    void merge(const LWWRegister& reg);

    /// Gets the replica id
    /// \return the replica's id
    uint64_t replica_id() const;
};

#endif //CRDTS_LWWREGISTER_HH
