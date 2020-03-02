#ifndef CRDTS_LWWREGISTER_HH
#define CRDTS_LWWREGISTER_HH

#include <string>
#include "timestamp.hh"

class LWWRegister {
private:
    Timestamp _timestamp;
    std::string _value;

public:
    std::string value();

    void assign(const std::string& value);

    void merge(const LWWRegister& reg);
};

#endif //CRDTS_LWWREGISTER_HH
