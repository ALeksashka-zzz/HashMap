#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>
#include <vector>

template <class KeyType, class ValueType, class Hash = std::hash<KeyType>> 
class HashMap {
private:
    static size_t InitSize;
    static size_t LoadFactor; 

    using Node = typename std::pair <const KeyType, ValueType>;
    using NodeIter = typename std::list<Node>::iterator;

    Hash Hasher;
    size_t NodeCount = 0;
    std::list<Node> Nodes;
    std::vector<NodeIter> Table;

public:
    explicit HashMap(const Hash &Hasher_ = Hash());
    template<class InputIt>
    explicit HashMap(InputIt First, InputIt Last, const Hash &Hasher_ = Hash());
    explicit HashMap(std::initializer_list<Node> InitList, const Hash &Hasher_ = Hash());

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
size_t HashMap<KeyType, ValueType, Hash>::InitSize = 10;

template<class KeyType, class ValueType, class Hash> 
size_t HashMap<KeyType, ValueType, Hash>::LoadFactor = 2;

template<class KeyType, class ValueType, class Hash> 
HashMap<KeyType, ValueType, Hash>::HashMap(const Hash &Hasher_): Hasher(Hasher_) {
    Table.assign(InitSize, Nodes.end());
}

template<class KeyType, class ValueType, class Hash>
template<class InputIt>
HashMap<KeyType, ValueType, Hash>::HashMap(InputIt First, InputIt Last, const Hash &Hasher_) : Hasher(Hasher_) {
    size_t Size = std::distance(First, Last);
    Table.resize(std::max((Size + 1) * LoadFactor, InitSize), Nodes.end());
    for (; First != Last; ++First) {
        insert(*First);
    }
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(std::initializer_list<Node> InitList, const Hash &Hasher_) : Hasher(Hasher_) {
    size_t Size = InitList.size();
    Table.resize(std::max((Size + 1) * LoadFactor, InitSize), Nodes.end());
    for (const Node &to : InitList) {
        insert(to);
    }
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(const HashMap<KeyType, ValueType, Hash> &other) {
    if (this != &other) {
        clear();
        size_t Size = other.NodeCount;
        Table.resize(std::max((Size + 1) * LoadFactor, InitSize), Nodes.end());
        for (const Node &to : other) {
            insert(to);
        }
    }
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>& HashMap<KeyType, ValueType, Hash>::operator=(const HashMap<KeyType, ValueType, Hash> &other) {
    if (this != &other) {
        clear();
        size_t Size = other.NodeCount;
        Table.resize(std::max((Size + 1) * LoadFactor, InitSize), Nodes.end());
        for (const Node &to : other) {
            insert(to);
        }
    }
    return *this;
}

template <class KeyType, class ValueType, class Hash> 
size_t HashMap<KeyType, ValueType, Hash>::size() const {
    return NodeCount;
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
    if ((NodeCount + 1) * LoadFactor > Table.size()) {
        resize();
    }
    size_t Index = Hasher(Node_.first) % Table.size();
    while (true) {
        if (Table[Index] == Nodes.end()) {
            Nodes.push_back(Node_);
            ++NodeCount;
            Table[Index] = --(Nodes.end());
            return;
        }
        (++Index) %= Table.size();
    }
}

template<class KeyType, class ValueType, class Hash> 
void HashMap<KeyType, ValueType, Hash>::erase(const KeyType &Key) {
    size_t Index = Hasher(Key) % Table.size();
    while (true) {
        if (Table[Index] == Nodes.end()) {
            return;
        } else if (Table[Index]->first == Key) {
            break;
        }
        (++Index) %= Table.size();
    }
    Nodes.erase(Table[Index]);
    --NodeCount;
    size_t Last = Index;
    // Searching until first empty cell occurs.  
    while (true) {
        (++Index) %= Table.size();
        if (Table[Index] == Nodes.end()) {
            Table[Last] = Nodes.end();
            return;
        } else {
            /* The current node (Index) must be moved to the last empty cell (Last) if it obeys the following conditions: 
                1. Let (Index > Last) and (RealHash <= Last). It means that find(Table[Index]->first) will return Nodes.end() due to the empty cell’s occurring (Last) during the search through the Table. To avoid that mistake, the current node (Index) must be moved.
                2. Let (Index > Last) and (RealHash > Index). The situation is similar because the empty cell (Last) will be faced first in the search (in find call).
                3. Let (Index > Last) and (RealHash > Last && RealHash <= Index). It is obvious that Last won’t cause any problem, thus current node stays still.   
                4. Let (Index < Last). Basing on the previous arguments, it is easily observed that only in case when (RealHash <= Last && RealHash > Index) the current node had to be moved to the last empty cell (Last).
                5. Notice that Index never equals Last before 'if'.
            */
            size_t RealHash = Hasher(Table[Index]->first) % Table.size();
            if ((Index > Last && (RealHash <= Last || RealHash > Index)) ||
                (Index < Last && (RealHash <= Last && RealHash > Index))) {
                Table[Last] = Table[Index];
                Last = Index;
            }
        }
    }
}

template<class KeyType, class ValueType, class Hash> 
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::find(const KeyType& Key) {
    size_t Index = Hasher(Key) % Table.size();
    while (true) {
        if (Table[Index] == Nodes.end()) {
            return Nodes.end();
        } else if (Table[Index]->first == Key) {
            return Table[Index];
        }
        (++Index) %= Table.size();
    }
}

template<class KeyType, class ValueType, class Hash> 
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType, ValueType, Hash>::find(const KeyType& Key) const {
    size_t Index = Hasher(Key) % Table.size();
    while (true) {
        if (Table[Index] == Nodes.end()) {
            return Nodes.end();
        } else if (Table[Index]->first == Key) {
            return Table[Index];
        }
        (++Index) %= Table.size();
    }
}

template<class KeyType, class ValueType, class Hash> 
ValueType &HashMap<KeyType, ValueType, Hash>::operator[](const KeyType& Key) {
    size_t Index = Hasher(Key) % Table.size();
    while (true) {
        if (Table[Index] == Nodes.end()) {
            insert({Key, ValueType()});
            return find(Key)->second;
        } else if (Table[Index]->first == Key) {
            return Table[Index]->second;
        }
        (++Index) %= Table.size();
    }
}


template<class KeyType, class ValueType, class Hash> 
const ValueType &HashMap<KeyType, ValueType, Hash>::at(const KeyType& Key) const {
    size_t Index = Hasher(Key) % Table.size();
    while (true) {
        if (Table[Index] == Nodes.end()) {
            throw std::out_of_range("Error, there is no such key :(");
        } else if (Table[Index]->first == Key) {
            return Table[Index]->second;
        }
        (++Index) %= Table.size();
    }
}

template<class KeyType, class ValueType, class Hash> 
void HashMap<KeyType, ValueType, Hash>::clear() {
    NodeCount = 0;
    Nodes.clear();
    Table.assign(InitSize, Nodes.end());
    return;
}

template <class KeyType, class ValueType, class Hash> 
void HashMap<KeyType, ValueType, Hash>::resize() {
    size_t NewSize = Table.size() * LoadFactor;
    NodeCount = 0;

    std::list<Node> CopyNodes;
    for (const Node &to : Nodes) {
        CopyNodes.push_back(to);
    }
    Nodes.clear();

    Table.assign(NewSize, Nodes.end());

    for (const Node &to : CopyNodes) {
        insert(to);
    }
}
