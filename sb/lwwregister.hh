#ifndef CRDTS_LWWREGISTER_HH
#define CRDTS_LWWREGISTER_HH

#include <string>
#include "timestamp.hh"

class LWWRegister {
private:
    Timestamp _timestamp;
    std::string _value;

public:
    LWWRegister(uint64_t replica_id);

    std::string value();

    void assign(const std::string& value);

    void merge(const LWWRegister& reg);

    uint64_t replica_id();

    bool initialized();
};

#endif //CRDTS_LWWREGISTER_HH
