#include "lwwregister.hh"

LWWRegister::LWWRegister(uint64_t replica_id) {
    _timestamp.replica_id(replica_id);
}

std::string LWWRegister::value() const {
    if (!initialized()) {
        throw "Querying an uninitialized register";
    }//if

    return _value;
}

void LWWRegister::assign(const std::string& value) {
    this->_timestamp.update();
    this->_value = value;
}

void LWWRegister::merge(const LWWRegister &reg) {
    if (this->_timestamp < reg._timestamp) {
        this->_value = reg._value;
        this->_timestamp.copy(reg._timestamp);
    }//if
}

uint64_t LWWRegister::replica_id() {
    return this->_timestamp.replica_id();
}

bool LWWRegister::initialized() const {
    return !this->_timestamp.beginning_of_time();
}

