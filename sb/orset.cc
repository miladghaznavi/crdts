#include "orset.hh"

OrSet::OrSet(uint64_t replica_id) :
    _unique_tag(replica_id) { }

void OrSet::add(std::string &e) {
    _observe[e].push_back(_unique_tag);
    _unique_tag.update();
}

void OrSet::remove(std::string &e) {
    auto _ob_elem = _observe.find(e);

    if (_ob_elem != _observe.end()) {
        auto elem = _remove.find(e);
        if (elem == _remove.end()) {
            _remove[e] = _ob_elem->second;
        }//if
        else {
            elem->second.insert(
                    elem->second.end(),
                    _ob_elem->second.begin(),
                    _ob_elem->second.end());
        }//else

        _observe.erase(_ob_elem);
    }//if
}

bool OrSet::lookup(std::string &e) {
    return _observe.count(e) > 0;
}

void OrSet::merge(OrSet &s) {

}
