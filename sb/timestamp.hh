#ifndef CRDTS_TIMESTAMP_HH
#define CRDTS_TIMESTAMP_HH

struct Timestamp {
private:
    uint64_t seq_number{}; // A monotonically increasing sequence number.
    uint64_t uid; // An identifier that is unique across all replicas.
    uint64_t replica_uid; // TODO: remove replica_uid and use an external call for the replica uid.

public:
    /// Creates a new timestamp
    /// \param uid the unique identifier
    explicit Timestamp(uint64_t replica_uid) {
        this->replica_uid = replica_uid;
        this->uid = replica_uid;
    }

    /// Copies the sequence number and uid of a given timestamp.
    /// The replica_uid is not changed.
    /// \param t
    void copy(const Timestamp &t) {
        this->uid = t.uid;
        this->seq_number = t.seq_number;
    }

    /// Compares the object timestamp with a given timestamp
    /// \param t the given timestamp
    /// \return true if the object timestamp ``happens before'' the given timestamp, otherwise false.
    /// Ties are prevented!
    bool operator < (const Timestamp& t) {
        // The comparison concatenates sequence numbers with uids to avoid ties.
        // The comparison of sequence numbers precedes considering the uids for a
        // total ordering that is more ''graceful''.
        return this->seq_number < t.seq_number or
                (this->seq_number == t.seq_number and this->uid < t.uid);
    }

    /// Update the timestamp by incrementing its sequence number
    void update() {
        // TODO: handle the overflow of seq_number
        // TODO: replace replica_uid with an external call that returns the replica's identifier.
        seq_number ++;
        uid = replica_uid;
    }

    /// checks if the current timestamp is at the beginning of time
    /// \return true if at beginning of time, otherwise false.
    bool beginning_of_time() {
        return seq_number == 0;
    }
};

#endif //CRDTS_TIMESTAMP_HH
