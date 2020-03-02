#ifndef CRDTS_TIMESTAMP_HH
#define CRDTS_TIMESTAMP_HH

struct Timestamp {
private:
    uint64_t _seq_number{}; // A monotonically increasing sequence number.
    uint64_t _uid; // An identifier that is unique across all replicas.
    uint64_t _replica_id; // TODO: remove replica_id and use an external call for the replica id.

public:
    /// Creates a new timestamp
    /// \param replica_id the replica's unique identifier
    explicit Timestamp(uint64_t replica_id) {
        this->_replica_id = replica_id;
        this->_uid = replica_id;
    }

    /// Compares the object timestamp with a given timestamp
    /// \param t the given timestamp
    /// \return true if the object timestamp ``happens before'' the given timestamp, otherwise false.
    bool operator < (const Timestamp& t) {
        // The comparison concatenates sequence numbers with uids to avoid ties.
        return this->_seq_number < t._seq_number or
               (this->_seq_number == t._seq_number and this->_uid < t._uid);
    }

    /// Update the timestamp by incrementing its sequence number
    void update() {
        // TODO: handle the overflow of seq_number
        // TODO: replace replica_id with an external call that returns the replica's identifier.
        _seq_number ++;
        _uid = _replica_id;
    }

    /// Checks if the current timestamp is at the beginning of time
    /// \return true if at beginning of time, otherwise false.
    bool beginning_of_time() {
        return _seq_number == 0;
    }

    /// Gets the value of replica
    /// \return
    uint64_t replica_id() {
        return _replica_id;
    }

    /// Copies the sequence number and uid of a given timestamp.
    /// The replica id is not changed.
    /// \param t the given timestamp
    void copy(const Timestamp &t) {
        this->_uid = t._uid;
        this->_seq_number = t._seq_number;
    }
};

#endif //CRDTS_TIMESTAMP_HH
