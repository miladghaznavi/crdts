#include "lwwregister.hh"

/// Constructs a LWWRegister
/// \param replica_id the replica
LWWRegister::LWWRegister(uint64_t replica_id) :
    _timestamp(replica_id) { }

/// Queries the value of the register
/// \return the latest value
std::string LWWRegister::value() {
    if (!initialized()) {
        throw "Querying an uninitialized register";
    }//if

    return _value;
}

/// Updates the value of the register with a given value
/// \param value the given value
void LWWRegister::assign(const std::string& value) {
    this->_value = value;
    this->_timestamp.update();
}

/// Updates with a given register if this register is more recent
/// \param reg the given register
void LWWRegister::merge(const LWWRegister &reg) {
    if (this->_timestamp < reg._timestamp) {
        this->_value = reg._value;
        this->_timestamp.copy(reg._timestamp);
    }//if
}

/// Gets the replica id
/// \return the replica's id
uint64_t LWWRegister::replica_id() {
    return this->_timestamp.replica_id();
}

/// Gets if the register has been initialized
/// \return true if the register is initialized, otherwise false
bool LWWRegister::initialized() {
    return !this->_timestamp.beginning_of_time();
}

