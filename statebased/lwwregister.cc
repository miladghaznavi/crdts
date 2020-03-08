#include "lwwregister.hh"

void LWWRegister::init_unique_id(uint64_t replica_id) {
    this->_timestamp.replica_id(replica_id);
}

std::string LWWRegister::value() const {
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

uint64_t LWWRegister::replica_id() const {
    return this->_timestamp.replica_id();
}
