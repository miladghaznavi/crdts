#ifndef CRDTS_ORSET_HH
#define CRDTS_ORSET_HH

#include <unordered_map>
#include <unordered_set>
#include <string>
#include "timestamp.hh"

class ORSet {
private:
    std::unordered_map<std::string, std::unordered_map<uint64_t, Timestamp>> _elements;
    std::unordered_map<uint64_t, Timestamp> _versions;
    uint64_t _replica_id;

    /// Merges the local version vector with a given set
    /// \param remote_set the given set
    void _merge_versions(const ORSet& remote_set);

    /// Applies remove operations from a given remote set, only operations that are more recent than
    /// observed add operations are effective.
    /// \param remote_set the remote set object
    void _apply_remote_removes(const ORSet& remote_set);

    /// Applies add operations from a given remote set with add-wins policy
    /// \param remote_set the given remote set
    void _apply_remote_adds(const ORSet& remote_set);

public:
    /// Creates an ORSet object at a replica identified with a given id
    /// \param replica_id the given replica id
    explicit ORSet(uint64_t replica_id);

    /// Adds a given element to the set
    /// \param e the given element
    void add(const std::string& e);

    /// Removes a given element from the set
    /// \param e the given element
    /// \return true if the given element existed and was removed, otherwise false
    bool remove(const std::string& e);

    /// Check if the given element exists in the set
    /// \param e the given element
    /// \return true if the given element exists, otherwise false
    bool contains(const std::string& e);

    /// Merges the local set with a given remote set
    /// \param remote_set the given remote set
    void merge(const ORSet& remote_set);

    /// Retrieves elements stored in the local replica
    /// \return the set of elements
    std::unordered_set<std::string> elements() const;

    /// Retrieves the local replica id
    /// \return the local replica id
    uint64_t replica_id();

    /// Retrieves the number of elements in the set
    /// \return the number of elements
    size_t size();
};

#endif //CRDTS_ORSET_HH
