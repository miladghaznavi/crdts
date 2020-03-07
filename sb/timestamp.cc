#include "timestamp.hh"

void Timestamp::replica_id(uint64_t replica_id) {
    this->_replica_id = replica_id;
    this->_uid = replica_id;
}

bool operator < (const Timestamp &t1, const Timestamp &t2) {
    // The comparison concatenates sequence numbers with uids to avoid ties.
    return t1._seq_number < t2._seq_number or
           (t1._seq_number == t2._seq_number and t1._uid < t2._uid);
}

bool operator == (const Timestamp& t1, const Timestamp& t2) {
    return t1._seq_number == t2._seq_number and t1._uid == t2._uid;
}

bool operator != (const Timestamp& t1, const Timestamp& t2) {
    return !(t1 == t2);
}

std::ostream& operator << (std::ostream &out, const Timestamp& t) {
    out << t._seq_number << t._uid;
    return out;
}

void Timestamp::update() {
    _seq_number ++;
    // TODO: replace replica_id with an external call that returns the replica's identifier.
    _uid = _replica_id;
}

bool Timestamp::beginning_of_time() const {
    return _seq_number == 0;
}

uint64_t Timestamp::replica_id() const {
    return _replica_id;
}

void Timestamp::copy(const Timestamp &t) {
    this->_uid = t._uid;
    this->_seq_number = t._seq_number;
}

uint64_t Timestamp::sequence_number() {
    return _seq_number;
}
