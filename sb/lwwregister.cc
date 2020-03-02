#include "lwwregister.hh"

/// query the value of the register
/// \return the latest value
std::string LWWRegister::value() {
    if (_timestamp.beginning_of_time() == 0) {
        throw "Querying an uninitialized register";
    }//if

    return _value;
}

/// update the value of the register with a given value
/// \param value the given value
void LWWRegister::assign(const std::string& value) {
    this->_value = value;
    this->_timestamp.update();
}

/// updates with a given register if this register is more recent
/// \param reg the given register
void LWWRegister::merge(const LWWRegister &reg) {
    if (this->_timestamp < reg._timestamp) { // Timestamps are uniques, thus there is no tie!
        this->_value = reg._value;
    }//if
}
