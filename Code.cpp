#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>
#include <vector>

template <class KeyType, class ValueType, class Hash = std::hash<KeyType>> 
class HashMap {
private:
    using Node = typename std::pair <const KeyType, ValueType>;
    using Bucket = typename std::list<Node>::iterator;

    Hash Hasher;
    size_t NodesSize;
    std::list<Node> Nodes;
    std::vector<Bucket> Table;

public:
    HashMap(const Hash &Hasher_ = Hash());
    template<class InputIt>
    HashMap(InputIt First, InputIt Last, const Hash &Hasher_ = Hash());
    HashMap(std::initializer_list<Node> InitList, const Hash &Hasher_ = Hash());

    size_t size() const;
    bool empty() const;
    Hash hash_function() const;

    void insert(const Node &Node_);
    void erase(const KeyType &Key);

    using iterator = typename std::list<Node>::iterator;
    using const_iterator = typename std::list<Node>::const_iterator;
    iterator begin() { return Nodes.begin(); }
    const_iterator begin() const { return Nodes.cbegin(); } 
    iterator end() { return Nodes.end(); }
    const_iterator end() const { return Nodes.cend(); }

    iterator find(const KeyType &Key);
    const_iterator find(const KeyType &Key) const;

    ValueType &operator[](const KeyType &Key);
    const ValueType &at(const KeyType &Key) const;

    void clear();

    void resize();
    HashMap(const HashMap &other);
    HashMap &operator=(const HashMap &other);
};

template<class KeyType, class ValueType, class Hash> 
HashMap<KeyType, ValueType, Hash>::HashMap(const Hash &Hasher_): Hasher(Hasher_), NodesSize(0) {
    Table.assign(10, Nodes.end());
}

template<class KeyType, class ValueType, class Hash>
template<class InputIt>
HashMap<KeyType, ValueType, Hash>::HashMap(InputIt First, InputIt Last, const Hash &Hasher_) : Hasher(Hasher_), NodesSize(0) {
    Table.assign(10, Nodes.end());
    for (; First != Last; ++First) {
        this->insert(*First);
    }
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(std::initializer_list<Node> InitList, const Hash &Hasher_) : Hasher(Hasher_), NodesSize(0) {
    Table.assign(10, Nodes.end());
    for (const Node to : InitList) {
        this->insert(to);
    }
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(const HashMap<KeyType, ValueType, Hash> &other) {
    if (this != &other) {
        this->clear();
        for (const Node &to : other) {
            this->insert(to);
        }
    }
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>& HashMap<KeyType, ValueType, Hash>::operator=(const HashMap<KeyType, ValueType, Hash> &other) {
    if (this != &other) {
        this->clear();
        for (const Node &to : other) {
            this->insert(to);
        }
    }
    return *this;
}

template <class KeyType, class ValueType, class Hash> 
size_t HashMap<KeyType, ValueType, Hash>::size() const {
    return NodesSize;
}

template<class KeyType, class ValueType, class Hash> 
bool HashMap<KeyType, ValueType, Hash>::empty() const {
   return size() == 0;
}

template <class KeyType, class ValueType, class Hash> 
Hash HashMap<KeyType, ValueType, Hash>::hash_function() const {
    return Hasher;
}

template<class KeyType, class ValueType, class Hash> 
void HashMap<KeyType, ValueType, Hash>::insert(const Node &Node_) {
    if ((NodesSize + 1) * 2 > Table.size()) {
        this->resize();
    }
    size_t BucketIt = Hasher(Node_.first) % Table.size();
    while (true) {
        if (Table[BucketIt] == Nodes.end()) {
            Nodes.push_back(Node_);
            ++NodesSize;
            Table[BucketIt] = --(Nodes.end());
            return;
        }
        (++BucketIt) %= Table.size();
    }
}

template<class KeyType, class ValueType, class Hash> 
void HashMap<KeyType, ValueType, Hash>::erase(const KeyType &Key) {
    size_t BucketIt = Hasher(Key) % Table.size();
    while (true) {
        if (Table[BucketIt] == Nodes.end()) {
            return;
        } else {
            if (Table[BucketIt]->first == Key) {
                break;
            }
        }
        (++BucketIt) %= Table.size();
    }
    Nodes.erase(Table[BucketIt]);
    --NodesSize;
    size_t Last = BucketIt;
    while (true) {
        (++BucketIt) %= Table.size();
        if (Table[BucketIt] == Nodes.end()) {
            Table[Last] = Nodes.end();
            return;
        } else {
            size_t RealHash = Hasher(Table[BucketIt]->first) % Table.size();
            if ((BucketIt > Last && (RealHash <= Last || RealHash > BucketIt)) ||
                (BucketIt < Last && (RealHash <= Last && RealHash > BucketIt))) {
                Table[Last] = Table[BucketIt];
                Last = BucketIt;
            }
        }
    }
}

template<class KeyType, class ValueType, class Hash> 
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::find(const KeyType& Key) {
    size_t BucketIt = Hasher(Key) % Table.size();
    while (true) {
        if (Table[BucketIt] == Nodes.end()) {
            return Nodes.end();
        } else {
            if (Table[BucketIt]->first == Key) {
                return Table[BucketIt];
            }
        }
        (++BucketIt) %= Table.size();
    }
}

template<class KeyType, class ValueType, class Hash> 
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType, ValueType, Hash>::find(const KeyType& Key) const {
    size_t BucketIt = Hasher(Key) % Table.size();
    while (true) {
        if (Table[BucketIt] == Nodes.end()) {
            return Nodes.end();
        } else {
            if (Table[BucketIt]->first == Key) {
                return Table[BucketIt];
            }
        }
        (++BucketIt) %= Table.size();
    }
}

template<class KeyType, class ValueType, class Hash> 
ValueType &HashMap<KeyType, ValueType, Hash>::operator[](const KeyType& Key) {
    size_t BucketIt = Hasher(Key) % Table.size();
    while (true) {
        if (Table[BucketIt] == Nodes.end()) {
            this->insert({Key, ValueType()});
            return this->find(Key)->second;
        } else {
            if (Table[BucketIt]->first == Key) {
                return Table[BucketIt]->second;
            }
        }
        (++BucketIt) %= Table.size();
    }
}

template<class KeyType, class ValueType, class Hash> 
const ValueType &HashMap<KeyType, ValueType, Hash>::at(const KeyType& Key) const {
    size_t BucketIt = Hasher(Key) % Table.size();
    while (true) {
        if (Table[BucketIt] == Nodes.end()) {
            throw std::out_of_range("");
        } else {
            if (Table[BucketIt]->first == Key) {
                return Table[BucketIt]->second;
            }
        }
        (++BucketIt) %= Table.size();
    }
}

template<class KeyType, class ValueType, class Hash> 
void HashMap<KeyType, ValueType, Hash>::clear() {
    NodesSize = 0;
    Nodes.clear();
    Table.assign(10, Nodes.end());
    return;
}

template <class KeyType, class ValueType, class Hash> 
void HashMap<KeyType, ValueType, Hash>::resize() {
    size_t CurSz = Table.size() * 2;
    NodesSize = 0;

    std::list<Node> CopyNodes;
    for (const Node &to : Nodes) {
        CopyNodes.push_back(to);
    }
    Nodes.clear();

    Table.assign(CurSz, Nodes.end());

    for (const Node &to : CopyNodes) {
        this->insert(to);
    }
    return;
}
