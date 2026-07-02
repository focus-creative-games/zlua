#pragma once

#pragma push_macro("GROUP_SIZE")
#undef GROUP_SIZE
#include "../../external/google/sparsehash/dense_hash_map.h"
#include "../../external/google/sparsehash/dense_hash_set.h"
#pragma pop_macro("GROUP_SIZE")

#include "utils/KeyWrapper.h"

#include <cstring>
#include <functional>
#include <string_view>
#include <utility>

namespace zlua
{

template <typename T>
struct RawPointerHash
{
    size_t operator()(const T* ptr) const
    {
        return reinterpret_cast<size_t>(ptr);
    }
};

template <typename T>
struct RawPointerEqual
{
    bool operator()(const T* lhs, const T* rhs) const
    {
        return lhs == rhs;
    }
};

template <typename K, typename V, typename Alloc = GOOGLE_NAMESPACE::libc_allocator_with_realloc<std::pair<const K*, V>>>
class AppendOnlyRawPointerHashMap
{
  public:
    using key_type = const K*;
    using mapped_type = V;
    using value_type = std::pair<const K*, V>;
    using hasher = RawPointerHash<K>;
    using key_equal = RawPointerEqual<K>;
    using size_type = size_t;

  private:
    using storage_type = GOOGLE_NAMESPACE::dense_hash_map<key_type, V, hasher, key_equal, Alloc>;

    storage_type m_map;

    void InitEmptyKey()
    {
        m_map.set_empty_key(nullptr);
    }

  public:
    using iterator = typename storage_type::iterator;
    using const_iterator = typename storage_type::const_iterator;

    AppendOnlyRawPointerHashMap()
    {
        InitEmptyKey();
    }

    explicit AppendOnlyRawPointerHashMap(size_type bucketCount) : m_map(bucketCount)
    {
        InitEmptyKey();
    }

    iterator begin()
    {
        return m_map.begin();
    }

    const_iterator begin() const
    {
        return m_map.begin();
    }

    iterator end()
    {
        return m_map.end();
    }

    const_iterator end() const
    {
        return m_map.end();
    }

    size_type size() const
    {
        return m_map.size();
    }

    bool empty() const
    {
        return m_map.empty();
    }

    void clear()
    {
        m_map.clear();
    }

    iterator find(key_type key)
    {
        return m_map.find(key);
    }

    const_iterator find(key_type key) const
    {
        return m_map.find(key);
    }

    mapped_type& operator[](key_type key)
    {
        return m_map[key];
    }

    std::pair<iterator, bool> insert(const value_type& value)
    {
        return m_map.insert(value);
    }

    std::pair<iterator, bool> insert(value_type&& value)
    {
        return m_map.insert(std::move(value));
    }
};

template <typename K, typename Alloc = GOOGLE_NAMESPACE::libc_allocator_with_realloc<const K*>>
class AppendOnlyRawPointerHashSet
{
  public:
    using key_type = const K*;
    using value_type = const K*;
    using hasher = RawPointerHash<K>;
    using key_equal = RawPointerEqual<K>;
    using size_type = size_t;

  private:
    using storage_type = GOOGLE_NAMESPACE::dense_hash_set<key_type, hasher, key_equal, Alloc>;

    storage_type m_set;

    void InitEmptyKey()
    {
        m_set.set_empty_key(nullptr);
    }

  public:
    using iterator = typename storage_type::iterator;
    using const_iterator = typename storage_type::const_iterator;

    AppendOnlyRawPointerHashSet()
    {
        InitEmptyKey();
    }

    explicit AppendOnlyRawPointerHashSet(size_type bucketCount) : m_set(bucketCount)
    {
        InitEmptyKey();
    }

    iterator begin()
    {
        return m_set.begin();
    }

    const_iterator begin() const
    {
        return m_set.begin();
    }

    iterator end()
    {
        return m_set.end();
    }

    const_iterator end() const
    {
        return m_set.end();
    }

    size_type size() const
    {
        return m_set.size();
    }

    bool empty() const
    {
        return m_set.empty();
    }

    void clear()
    {
        m_set.clear();
    }

    iterator find(value_type value)
    {
        return m_set.find(value);
    }

    const_iterator find(value_type value) const
    {
        return m_set.find(value);
    }

    std::pair<iterator, bool> insert(value_type value)
    {
        return m_set.insert(value);
    }
};

struct CsStringHash
{
    size_t operator()(const char* key) const
    {
        return std::hash<std::string_view>()(std::string_view(key != nullptr ? key : ""));
    }
};

struct CsStringEqual
{
    bool operator()(const char* lhs, const char* rhs) const
    {
        if (lhs == rhs)
            return true;
        if (lhs == nullptr || rhs == nullptr)
            return false;
        return std::strcmp(lhs, rhs) == 0;
    }
};

// const char* key, insert-only dense_hash container. Uses nullptr as empty sentinel;
// set_deleted_key is intentionally omitted so entries are never tombstoned.
template <class T, class Alloc = GOOGLE_NAMESPACE::libc_allocator_with_realloc<std::pair<const char*, T>>>
class AppendOnlyStringHashMap
{
  public:
    using key_type = const char*;
    using mapped_type = T;
    using value_type = std::pair<const char*, mapped_type>;
    using hasher = CsStringHash;
    using key_equal = CsStringEqual;
    using size_type = size_t;

  private:
    using storage_type = GOOGLE_NAMESPACE::dense_hash_map<const char*, mapped_type, CsStringHash, CsStringEqual, Alloc>;

    storage_type m_map;

    void InitEmptyKey()
    {
        m_map.set_empty_key(nullptr);
    }

  public:
    using iterator = typename storage_type::iterator;
    using const_iterator = typename storage_type::const_iterator;

    AppendOnlyStringHashMap()
    {
        InitEmptyKey();
    }

    explicit AppendOnlyStringHashMap(size_type bucketCount) : m_map(bucketCount)
    {
        InitEmptyKey();
    }

    iterator begin()
    {
        return m_map.begin();
    }

    const_iterator begin() const
    {
        return m_map.begin();
    }

    iterator end()
    {
        return m_map.end();
    }

    const_iterator end() const
    {
        return m_map.end();
    }

    size_type size() const
    {
        return m_map.size();
    }

    bool empty() const
    {
        return m_map.empty();
    }

    void clear()
    {
        m_map.clear();
    }

    iterator find(const char* key)
    {
        return m_map.find(key);
    }

    const_iterator find(const char* key) const
    {
        return m_map.find(key);
    }

    mapped_type& operator[](const char* key)
    {
        return m_map[key];
    }

    std::pair<iterator, bool> insert(const value_type& value)
    {
        return m_map.insert(value);
    }

    std::pair<iterator, bool> insert(value_type&& value)
    {
        return m_map.insert(std::move(value));
    }
};

template <typename Alloc = GOOGLE_NAMESPACE::libc_allocator_with_realloc<const char*>>
class AppendOnlyStringHashSet
{
  public:
    using key_type = const char*;
    using value_type = const char*;
    using hasher = CsStringHash;
    using key_equal = CsStringEqual;
    using size_type = size_t;

  private:
    using storage_type = GOOGLE_NAMESPACE::dense_hash_set<const char*, CsStringHash, CsStringEqual, Alloc>;

    storage_type m_set;

    void InitEmptyKey()
    {
        m_set.set_empty_key(static_cast<const char*>(nullptr));
    }

  public:
    using iterator = typename storage_type::iterator;
    using const_iterator = typename storage_type::const_iterator;

    AppendOnlyStringHashSet()
    {
        InitEmptyKey();
    }

    explicit AppendOnlyStringHashSet(size_type bucketCount) : m_set(bucketCount)
    {
        InitEmptyKey();
    }

    iterator begin()
    {
        return m_set.begin();
    }

    const_iterator begin() const
    {
        return m_set.begin();
    }

    iterator end()
    {
        return m_set.end();
    }

    const_iterator end() const
    {
        return m_set.end();
    }

    size_type size() const
    {
        return m_set.size();
    }

    bool empty() const
    {
        return m_set.empty();
    }

    void clear()
    {
        m_set.clear();
    }

    iterator find(const char* value)
    {
        return m_set.find(value);
    }

    const_iterator find(const char* value) const
    {
        return m_set.find(value);
    }

    std::pair<iterator, bool> insert(const char* value)
    {
        return m_set.insert(value);
    }
};

template <class Key, class Hash>
struct KeyWrapperHash
{
    size_t operator()(const KeyWrapper<Key>& wrapped) const
    {
        if (!wrapped.isNormal())
            return 0;
        return Hash()(wrapped.key);
    }
};

template <class Key, class Equal>
struct KeyWrapperEqual
{
    bool operator()(const KeyWrapper<Key>& left, const KeyWrapper<Key>& right) const
    {
        if (left.type != right.type)
            return false;

        if (!left.isNormal())
            return true;

        return m_equal(left.key, right.key);
    }

  private:
    Equal m_equal{};
};

template <class Key, class T, class Hash = SPARSEHASH_HASH<Key>, class Equal = std::equal_to<Key>,
          class Alloc = GOOGLE_NAMESPACE::libc_allocator_with_realloc<std::pair<const KeyWrapper<Key>, T>>>
class HashMap
{
  public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const Key, mapped_type>;
    using hasher = Hash;
    using key_equal = Equal;
    using size_type = size_t;

  private:
    using key_wrapper = KeyWrapper<Key>;
    using hash_wrapper = KeyWrapperHash<Key, Hash>;
    using key_equal_wrapper = KeyWrapperEqual<Key, Equal>;
    using storage_type = GOOGLE_NAMESPACE::dense_hash_map<key_wrapper, mapped_type, hash_wrapper, key_equal_wrapper, Alloc>;

    storage_type m_map;

    static key_wrapper EmptyKey()
    {
        return key_wrapper(key_wrapper::KeyType_Empty);
    }

    static key_wrapper DeletedKey()
    {
        return key_wrapper(key_wrapper::KeyType_Deleted);
    }

    static key_wrapper Wrap(const Key& key)
    {
        return key_wrapper(key);
    }

    void InitSentinelKeys()
    {
        m_map.set_deleted_key(DeletedKey());
        m_map.set_empty_key(EmptyKey());
    }

  public:
    using iterator = typename storage_type::iterator;
    using const_iterator = typename storage_type::const_iterator;

    HashMap()
    {
        InitSentinelKeys();
    }

    explicit HashMap(size_type bucketCount) : m_map(bucketCount)
    {
        InitSentinelKeys();
    }

    iterator begin()
    {
        return m_map.begin();
    }

    const_iterator begin() const
    {
        return m_map.begin();
    }

    iterator end()
    {
        return m_map.end();
    }

    const_iterator end() const
    {
        return m_map.end();
    }

    size_type size() const
    {
        return m_map.size();
    }

    bool empty() const
    {
        return m_map.empty();
    }

    void clear()
    {
        m_map.clear();
    }

    iterator find(const Key& key)
    {
        return m_map.find(Wrap(key));
    }

    const_iterator find(const Key& key) const
    {
        return m_map.find(Wrap(key));
    }

    mapped_type& operator[](const Key& key)
    {
        return m_map[Wrap(key)];
    }

    std::pair<iterator, bool> insert(const value_type& value)
    {
        return m_map.insert(std::make_pair(Wrap(value.first), value.second));
    }

    std::pair<iterator, bool> insert(value_type&& value)
    {
        return m_map.insert(std::make_pair(Wrap(value.first), std::move(value.second)));
    }

    size_type erase(const Key& key)
    {
        return m_map.erase(Wrap(key));
    }

    void erase(iterator it)
    {
        m_map.erase(it);
    }
};

template <class Value, class Hash = SPARSEHASH_HASH<Value>, class Equal = std::equal_to<Value>,
          class Alloc = GOOGLE_NAMESPACE::libc_allocator_with_realloc<KeyWrapper<Value>>>
class HashSet
{
  public:
    using key_type = Value;
    using value_type = Value;
    using hasher = Hash;
    using key_equal = Equal;
    using size_type = size_t;

  private:
    using key_wrapper = KeyWrapper<Value>;
    using hash_wrapper = KeyWrapperHash<Value, Hash>;
    using key_equal_wrapper = KeyWrapperEqual<Value, Equal>;
    using storage_type = GOOGLE_NAMESPACE::dense_hash_set<key_wrapper, hash_wrapper, key_equal_wrapper, Alloc>;

    storage_type m_set;

    static key_wrapper EmptyKey()
    {
        return key_wrapper(key_wrapper::KeyType_Empty);
    }

    static key_wrapper DeletedKey()
    {
        return key_wrapper(key_wrapper::KeyType_Deleted);
    }

    static key_wrapper Wrap(const Value& value)
    {
        return key_wrapper(value);
    }

    void InitSentinelKeys()
    {
        m_set.set_deleted_key(DeletedKey());
        m_set.set_empty_key(EmptyKey());
    }

  public:
    using iterator = typename storage_type::iterator;
    using const_iterator = typename storage_type::const_iterator;

    HashSet()
    {
        InitSentinelKeys();
    }

    explicit HashSet(size_type bucketCount) : m_set(bucketCount)
    {
        InitSentinelKeys();
    }

    iterator begin()
    {
        return m_set.begin();
    }

    const_iterator begin() const
    {
        return m_set.begin();
    }

    iterator end()
    {
        return m_set.end();
    }

    const_iterator end() const
    {
        return m_set.end();
    }

    size_type size() const
    {
        return m_set.size();
    }

    bool empty() const
    {
        return m_set.empty();
    }

    void clear()
    {
        m_set.clear();
    }

    iterator find(const Value& value)
    {
        return m_set.find(Wrap(value));
    }

    const_iterator find(const Value& value) const
    {
        return m_set.find(Wrap(value));
    }

    std::pair<iterator, bool> insert(const Value& value)
    {
        return m_set.insert(Wrap(value));
    }

    size_type erase(const Value& value)
    {
        return m_set.erase(Wrap(value));
    }

    void erase(iterator it)
    {
        m_set.erase(it);
    }
};
} // namespace zlua
