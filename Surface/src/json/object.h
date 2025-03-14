#pragma once

#include "value.h"

#include <utility> // std::move()

namespace Surface::JSON
{

/**
 * @brief iterator for Object
 * @tparam Entry type that is returned by the iterator
 * @tparam ItrEntry type that is iterated on
 */
template <class Entry, class ItrEntry> struct ObjectIterator
{
    static_assert(std::is_base_of<Entry, ItrEntry>::value,
                  "ItrEntry type must be derived from Entry type");

    ObjectIterator(ItrEntry* entries) : entries(entries) {};

    ObjectIterator& operator++() noexcept
    {
        ++entries;
        return *this;
    }

    ObjectIterator operator++(int) noexcept
    {
        ObjectIterator cpy = *this;
        ++(*this);
        return cpy;
    }

    ObjectIterator& operator--() noexcept
    {
        --entries;
        return *this;
    }

    ObjectIterator operator--(int) noexcept
    {
        ObjectIterator cpy = *this;
        --(*this);
        return cpy;
    }

    inline Entry& operator[](const size_t index) const noexcept
    {
        return *(entries + index);
    }

    inline Entry* operator->() const noexcept
    {
        return entries;
    }

    inline Entry& operator*() const noexcept
    {
        return *entries;
    }

    inline bool operator==(const ObjectIterator& other) const noexcept
    {
        return entries == other.entries;
    }

    inline bool operator!=(const ObjectIterator& other) const noexcept
    {
        return !(*this == other);
    }

  private:
    ItrEntry* entries;
};

/**
 * @brief const iterator for Object
 * @tparam Entry type that is returned by the iterator
 * @tparam ItrEntry type that is iterated on
 */
template <class Entry, class ItrEntry> struct ObjectConstIterator
{
    static_assert(std::is_base_of<Entry, ItrEntry>::value,
                  "ItrEntry type must be derived from Entry type");

    ObjectConstIterator(const ItrEntry* entries) : entries(entries) {};

    ObjectConstIterator& operator++() noexcept
    {
        ++entries;
        return *this;
    }

    ObjectConstIterator operator++(int) noexcept
    {
        ObjectConstIterator cpy = *this;
        ++(*this);
        return cpy;
    }

    ObjectConstIterator& operator--() noexcept
    {
        --entries;
        return *this;
    }

    ObjectConstIterator operator--(int) noexcept
    {
        ObjectConstIterator cpy = *this;
        --(*this);
        return cpy;
    }

    inline const Entry& operator[](const size_t index) const noexcept
    {
        return *(entries + index);
    }

    inline const Entry* operator->() const noexcept
    {
        return entries;
    }

    inline const Entry& operator*() const noexcept
    {
        return *entries;
    }

    inline bool operator==(const ObjectConstIterator& other) const noexcept
    {
        return entries == other.entries;
    }

    inline bool operator!=(const ObjectConstIterator& other) const noexcept
    {
        return !(*this == other);
    }

  private:
    const ItrEntry* entries;
};

/*
I want the speed of a hash table for entry finding, and the pleasure of ordered iteration by simply
incrementing an index.

In order to ensure incrementing an index from 0 to size() will work, there must be a simple list
whose size is equal to size(), which contains all the Entries of the map. This list will act as a
stack, only adding to the back, but allows removing elements from any position. This list will be
the primary container of Entries for the map.

In order to have the speed of a hash table, there needs to be a hashing function which returns an
index to lookup. This can then be used to access a table of Entries. To handle collisions, the
Entries will contain a pointer to a neighbor entry. This will be a secondary container of all
Entries for the map, suitable only for lookups via key name.

*/

// JSON Object type, unordered per JSON specification, but supports entry iteration
struct Object
{
  public: // Entry struct
    struct Entry
    {
        const char* key = nullptr;
        Value value;
    };

  public: // Constructors
    // Copy
    Object(const Object& other);

    // Move
    Object(Object&& other) noexcept;

    ~Object();

  public: // Operators
    // Set the value stored at a key
    Value& operator[](const char* key);

    // Get the value stored at key
    const Value& operator[](const char* key) const;

    // Move other object data into this object
    Object& operator=(Object&& other) noexcept;

    // Copy
    Object& operator=(const Object& other);

    // Objects are never equal to each other, unless they point to the same memory, which is
    // normally impossible
    inline bool operator==(const Object& right)
    {
        return this == &right;
    }

    inline bool operator!=(const Object& right)
    {
        return !(*this == right);
    }

  public: // Methods
    /**
     * @brief Deletes all entries from the map, returning the previous size.
     *
     * @return previous size of the map
     */
    size_t clear();

    /**
     * @brief Deletes a key from the map, returning `true` if the key existed.
     *
     * If this returns `true`, references to the associated Value WILL be invalidated. This method
     * will never down-size the map, use set_capacity() to reduce memory usage if you need.
     *
     * @param key
     * @return true if a key was deleted, false otherwise
     */
    bool delete_key(const char* key);

    /**
     * @brief Retrieve an entry at the given index. Entries are kept in insertion-order, so you can
     * always iterate over the size of the Object to get all Entries.
     *
     * Object also supports range iteration with the `for (auto& entry : object)` syntax.
     *
     * @param index index
     * @return Entry
     */
    Entry& entry_at(size_t index);
    const Entry& entry_at(size_t index) const;

    /**
     * @brief Finds any key with an equal value, or nullptr if no key was found.
     *
     * When searching by numbers, please see the == operator for Value, which considers numbers of
     * different types as unequal (i.e. 1 uint != 1 int).
     *
     * This function iterates over the entries list, so it may be slow.
     *
     * @param value value
     * @return const char* key, or nullptr if not found
     */
    const char* find_key(const Value& value) const;

    template <class Type> inline const char* find_key(const Type& value) const
    {
        return find_key(Value(value));
    }

    /**
     * @brief Get the value associated with the key, returning a default if not present. Does not
     * insert the key if it does not exist.
     *
     * @param key key
     * @param fallback default value
     * @return value
     */
    Value& get(const char* key, Value& fallback);
    const Value& get(const char* key, const Value& fallback) const;

    // Get the size of this object, which is the number of entries
    inline size_t get_size() const
    {
        return size;
    }

    /**
     * @brief Test if the map contains an entry for a given key.
     * @param key key
     * @return true if the key exists, false otherwise
     */
    bool has(const char* key) const;

    /**
     * @brief Set the value of a key, copying the provided value. The key is inserted if it does not
     * exist, and overwritten otherwise.
     *
     * This function may trigger a rehash of the map.
     *
     * @param key key
     * @param value value
     * @return this Object
     */
    Object& set(const char* key, const Value& value);

    template <class Type> inline Object& set(const char* key, const Type& value)
    {
        return set(key, Value(value));
    }

    /**
     * @brief Set the value of a key, moving the value. The key is inserted if it does not exist,
     * and overwritten otherwise.
     *
     * This function may trigger a rehash of the map.
     *
     * @param key key
     * @param value value
     * @return this Object
     */
    Object& set(const char* key, Value&& value);

    template <class Type> Object& set(const char* key, Type&& value)
    {
        return set(key, Value(std::move(value)));
    }

    /**
     * @brief Updates a key ONLY if it exists in the map. The provided value is copied into the
     * existing value such that existing references are also modified.
     *
     * This is an optimized method for calling `has()` and then `set()`.
     *
     * @param key key
     * @param value value
     * @return this Object
     */
    Object& update(const char* key, const Value& value);

    template <class Type> inline Object& update(const char* key, const Type& value)
    {
        return update(key, Value(value));
    }

    /**
     * @brief Updates a key ONLY if it exists in the map. The provided value is moved into the
     * existing value such that existing references are also modified.
     *
     * This is an optimized method for calling `has()` and then `set()`.
     *
     * @param key key
     * @param value value
     * @return this Object
     */
    Object& update(const char* key, Value&& value);

    template <class Type> Object& update(const char* key, Type&& value)
    {
        return update(key, Value(std::move(value)));
    }

  public: // Methods not intended for external use, but available anyway
    // Get the number of buckets for the hash table
    inline size_t get_buckets() const
    {
        return total_buckets;
    }

    // Get the capacity of the entries list
    inline size_t get_capacity() const
    {
        return capacity;
    }

    /**
     * @brief Set the capacity of this map, returns true if the new capacity was different from
     * the old capacity. This will delete arbitrary entries if new_capacity < size. This may trigger
     * a rehash if the new capacity is significantly different from the current size. This is the
     * only method capable of triggering a down-size of the hash table.
     *
     * @param new_capacity
     * @return true if the capacity was changed
     */
    bool set_capacity(size_t new_capacity);

    // Get the load factor of the hash table; size / buckets
    inline float load_factor() const
    {
        if (total_buckets == 0)
        {
            return static_cast<float>(1e+300 *
                                      1e+300); // overflows to return float positive infinity
        }
        return 1.0f * size / total_buckets;
    }

    // Test if the hash table should be remade. Under normal operation, this method should never
    // return true to external callers.
    inline bool should_rehash() const
    {
        return load_factor() >= max_load_factor();
    }

    // Compute the ideal number of buckets given the current size. Targets a 50% load.
    inline size_t ideal_buckets() const
    {
        return size * 2;
    }

    // The maximum load factor of the hash table. In this case, it would size up at 200% load. The
    // hash table is remade upon reaching this capacity.
    static inline float max_load_factor()
    {
        return 2.0f;
    }

    // The miminum load factor of the hash table. In this case, it would size down at 25% load. This
    // is only tested in calls to set_capacity() which reduce the capacity of the map.
    static inline float min_load_factor()
    {
        return 0.25;
    }

    // Hashes a null-terminated string
    static inline size_t hash(const char* key)
    {
        // Taken, er, borrowed :) from MSVC `type_traits`
        // Looked good enough to me.
        static const size_t PRIME = 1099511628211ULL;
        size_t result = 14695981039346656037ULL;

        for (size_t i = 0; i < 16; ++i) // Will not hash more than 16 characters
        {
            if (key[i] == 0)
            {
                break;
            }

            result ^= static_cast<size_t>(key[i]);
            result *= PRIME;
        }

        return result;
    }

  private: // Necessary declaration for the iterators
    struct TableEntry;

  public: // Iterators
    using iterator = ObjectIterator<Entry, TableEntry>;
    using const_iterator = ObjectConstIterator<Entry, TableEntry>;

    inline iterator begin()
    {
        return iterator(entries);
    }

    inline iterator end()
    {
        return iterator(entries + size);
    }

    inline const_iterator begin() const
    {
        return const_iterator(entries);
    }

    inline const_iterator end() const
    {
        return const_iterator(entries + size);
    }

  private: // Constructors
    Object(size_t capacity);

  private: // Hash table
    // Table bucket
    struct TableBucket
    {
        // Pointer to the next table entry
        TableEntry* next = nullptr;
    };

    // Number of buckets
    size_t total_buckets = 0;

    // list of buckets
    TableBucket* buckets = nullptr;

  private: // Entry list
    // Table entry, may point to a neighbor entry
    struct TableEntry : TableBucket, Entry
    {
        // Pointer to a parent, which could also be another TableEntry
        TableBucket* parent = nullptr;
    };

    // Represents the capacity of the entry list
    size_t capacity;

    // Represents the number of entries in the list
    size_t size = 0;

    // Table entries, will be kept unordered
    TableEntry* entries = nullptr;

  private: // Internal maintenance methods
    // Common logic to retrieve an entry
    Entry* get_entry(const char* key) const;

    // Common logic to retrieve or insert an entry, guaranteed to return an entry
    Entry& get_or_insert_entry(const char* key);

    // Runs all the needed tasks prior to adding a new entry at the end of the list.
    // Returns `true` if the operation included a rehash.
    bool pre_insert();

    // Recomputes hashes of all entries and rebuilds the table.
    void rehash();

    // Resizes the entry array, possibly rehashing the table if it needs to grow.
    // Returns `true` if it rehashed the table
    bool resize(size_t new_size);

    // Grows or shrinks the entry array to match current capacity
    // Does a lot of pointer work, so we may need to skip if a rehash is coming
    void update_capacity(bool update_pointers);

  private: // common data operations
    // Copy
    void copy_from(const Object& other);

    // Move
    void move_from(Object&& other);

    // Delete
    void delete_data();

#ifdef DEBUG
  public: // debug only
    inline static int _constructs()
    {
        return __constructs;
    }

    inline static int _copies()
    {
        return __copies;
    }

    inline static int _moves()
    {
        return __moves;
    }

  private:
    static int __constructs, __copies, __moves;
#endif

  public: // Friends
    friend Value;
};

} // namespace Surface::JSON
