#ifndef CRDTS_TIMESTAMP_HH
#define CRDTS_TIMESTAMP_HH

#include <ostream>

/// Timestamp can represent a unique tag or a unique timestamp across
/// replicas with unique identifier. Timestamp can be used to provide
/// global ordering.
class Timestamp {
private:
    uint64_t _seq_number{}; // A monotonically increasing sequence number.
    uint64_t _uid{}; // An identifier that is unique across all replicas.
    uint64_t _replica_id{}; // TODO: remove replica_id and use an external call for the replica id.

public:
    /// Creates a new timestamp
    /// \param replica_id the replica's unique identifier
    void replica_id(uint64_t replica_id);

    /// Compares the object timestamp with a given timestamp
    /// \param t1 the given timestamp
    /// \param t2 the given timestamp
    /// \return true if t1 ``happens before'' t2, otherwise false.
    friend bool operator < (const Timestamp& t1, const Timestamp& t2);

    /// Compares the equality of two timestamp objects
    /// \param t1 the first timestamp
    /// \param t2 the second timestamp
    /// \return true of the two timestamps are equal, otherwise false
    friend bool operator == (const Timestamp& t1, const Timestamp& t2);

    /// Compares the inequality of two timestamp objects
    /// \param t1 the first timestamp
    /// \param t2 the second timestamp
    /// \return true of the two timestamps are unequal, otherwise false
    friend bool operator != (const Timestamp& t1, const Timestamp& t2);

    /// Update the timestamp by incrementing its sequence number
    void update();

    /// Checks if the current timestamp is at the beginning of time
    /// \return true if at beginning of time, otherwise false.
    bool beginning_of_time() const;

    /// Gets the value of replica
    /// \return
    uint64_t replica_id() const;

    /// Copies the sequence number and uid of a given timestamp.
    /// The replica id is not copied.
    /// \param t the given timestamp
    void copy(const Timestamp &t);

    /// Gets the sequence number
    /// \return the sequence number
    uint64_t sequence_number();
};

#endif //CRDTS_TIMESTAMP_HH
