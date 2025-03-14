#include "object.h"

#include "internal/assert.h"

#include <stdint.h> // SIZE_MAX macro
#include <string.h> // strnlen functions
#include <utility>  // std::move()

#define STR_LEN SIZE_MAX - 1
#define MAX_KEY_LEN 1024

namespace Surface::JSON
{

Object::Object(size_t capacity)
{
#ifdef DEBUG
    ++__constructs;
#endif

    this->capacity = capacity;
    update_capacity(false);
}

Object::Object(const Object& other) // Copy
{
#ifdef DEBUG
    ++__constructs;
#endif

    copy_from(other);
}

Object::Object(Object&& other) noexcept // Move
{
#ifdef DEBUG
    ++__constructs;
#endif

    move_from(std::move(other));
}

Object::~Object()
{
    delete_data();
}

Value& Object::operator[](const char* key)
{
    // Read/ Write operation
    Entry& entry = get_or_insert_entry(key);
    return entry.value;
}

// Although this is decidely not documented as such, and in fact they claim only 4700-4999 requires
// this, 4172 must ALSO be disabled BEFORE the function that would create the warning.
// Microsoft does it again!
#pragma warning(disable : 4172)

const Value& Object::operator[](const char* key) const
{
    // Read only operation, debug-asserts if entry is null
    Entry* entry = get_entry(key);

    if (entry == nullptr)
    {
        assert(("Could not get via const operator[], returning Invalid entry. You should either "
                "test if the key exists with Object::has(), or use Object::get().",
                false));

        // This is just to help callers understand they must use other methods on const Objects when
        // they don't know if a key exists. We do not return JSON::NULL because that could wrongly
        // imply the key exists and has the value JSON::NULL.
        return Value(); // Returning a reference to a local value is deliberate here
#pragma warning(default : 4172)
    }

    return entry->value;
}

Object& Object::operator=(Object&& other) noexcept // Move
{
    move_from(std::move(other));
    return *this;
}

Object& Object::operator=(const Object& other) // Copy
{
    copy_from(other);
    return *this;
}

size_t Object::clear()
{
    if (size == 0)
    {
        return 0;
    }

    size_t old_size = size;

    // We can just use size as our index
    for (/* empty */; size > 0; --size)
    {
        TableEntry& entry = entries[size - 1];

        // delete the key
        delete[] entry.key;
        entry.key = nullptr;

        // Directly call value destructor
        entry.value.~Value();

        // cut from parent, this is sufficient to clear buckets and entry chains
        entry.parent->next = nullptr;
    }

    return old_size;
}

bool Object::delete_key(const char* key)
{
    if (size == 0)
    {
        return false;
    }

    // Safe cast because we got a pointer to the entry location, no object slicing here
    TableEntry* entry = (TableEntry*) get_entry(key);
    if (entry == nullptr)
    {
        return false;
    }

    // Delete our key
    delete[] entry->key;
    entry->key = nullptr;

    // Connect our next with our parent
    entry->parent->next = entry->next;
    if (entry->next != nullptr)
    {
        entry->next->parent = entry->parent;
    }

    if (size == 1)
    {
        // Last entry, directly call the value destructor
        entry->value.~Value();
        // This is how std::vector does it, so I'm sure this is correct
    }
    else
    {
        // Feels clever but I'm sure this is what everyone does.
        // Move the last item into this entry, update parent's next pointer.
        // This should implicitly cause this entry's Value to delete any allocations it has made
        // while it moves in the other value
        *entry = std::move(entries[size - 1]);
        entry->parent->next = entry;
    }

    --size;
    return true;
}

Object::Entry& Object::entry_at(size_t index)
{
    assert(("Index must be less than size", index < size));

    return entries[index];
}

const Object::Entry& Object::entry_at(size_t index) const
{
    assert(("Index must be less than size", index < size));

    return entries[index];
}

const char* Object::find_key(const Value& value) const
{
    for (auto& [key, test] : *this)
    {
        if (test == value)
        {
            return key;
        }
    }

    return nullptr;
}

Value& Object::get(const char* key, Value& fallback)
{
    Entry* entry = get_entry(key);
    if (entry == nullptr)
    {
        return fallback;
    }

    return entry->value;
}

const Value& Object::get(const char* key, const Value& fallback) const
{
    Entry* entry = get_entry(key);
    if (entry == nullptr)
    {
        return fallback;
    }

    return entry->value;
}

bool Object::has(const char* key) const
{
    return get_entry(key) != nullptr;
}

Object& Object::set(const char* key, const Value& value) // Copy
{
    Entry& entry = get_or_insert_entry(key);
    entry.value = value;
    return *this;
}

Object& Object::set(const char* key, Value&& value) // Move
{
    Entry& entry = get_or_insert_entry(key);
    entry.value = std::move(value);
    return *this;
}

Object& Object::update(const char* key, const Value& value) // Copy
{
    Entry* entry = get_entry(key);

    if (entry != nullptr)
    {
        entry->value = value;
    }

    return *this;
}

Object& Object::update(const char* key, Value&& value) // Move
{
    Entry* entry = get_entry(key);

    if (entry != nullptr)
    {
        entry->value = std::move(value);
    }

    return *this;
}

bool Object::set_capacity(size_t new_capacity)
{
    if (capacity == new_capacity)
    {
        return false;
    }

    capacity = new_capacity;
    update_capacity(true);

    return true;
}

Object::Entry* Object::get_entry(const char* key) const
{
    size_t key_length = strnlen(key, MAX_KEY_LEN + 1);
    assert(("Keys must be 1024 characters or less", key_length <= MAX_KEY_LEN));

    if (size == 0)
    {
        return nullptr;
    }

    // TODO: below a certain size, it should be faster to just iterate each entry.
    //       Look up what a standard value for that is.

    size_t key_hash = hash(key);
    size_t bucket_idx = key_hash % total_buckets;
    TableEntry* entry = buckets[bucket_idx].next;

    // Search for matching key
    size_t max = size; // guard against recursive next pointers
    while (max && entry != nullptr)
    {
        if (strncmp(key, entry->key, MAX_KEY_LEN) == 0)
        {
            // Matched entry
            return entry;
        }
        entry = entry->next;
    }

    return nullptr;
}

Object::Entry& Object::get_or_insert_entry(const char* key)
{
    size_t key_length = strnlen(key, MAX_KEY_LEN + 1);
    assert(("Keys must be 1024 characters or less", key_length <= MAX_KEY_LEN));

    // TODO: below a certain size, it should be faster to just iterate each entry.
    //       Look up what a standard value for that is.

    if (size == 0)
    {
        // Initialize buckets to capacity, or 4, whichever is greater
        total_buckets = capacity > 4 ? capacity : 4;
        buckets = new TableBucket[total_buckets];
    }

    size_t key_hash = hash(key);
    size_t bucket_idx = key_hash % total_buckets;

    // Must track parent index, resizing may invalidate entry pointers
    TableBucket* bucket = buckets + bucket_idx;
    size_t parent_idx = 0;

    // Is there an entry in bucket?
    if (bucket->next != nullptr)
    {
        // Search for matching key, or available next pointer
        size_t max = size; // guard against recursive next pointers
        TableBucket* parent = bucket;
        do
        {
            parent = parent->next;
            TableEntry& entry = *((TableEntry*) parent);
            if (strncmp(key, entry.key, MAX_KEY_LEN) == 0)
            {
                // Matched entry, return it
                return entry;
            }
            --max;
        }
        while (max && parent->next != nullptr);

        // set parent_offset and unset bucket
        bucket = nullptr;
        parent_idx = (TableEntry*) parent - entries;
    }

    // This will be an insert
    // Insert may update capacity and relocate entries, but it returns `true` if the map was
    // rehashed and so buckets/ parents have changed
    bool rehashed = pre_insert();

    if (rehashed)
    {
        // Must update hash index and search for empty spot again
        bucket_idx = key_hash % total_buckets;
        bucket = &buckets[bucket_idx];

        if (bucket->next != nullptr)
        {
            size_t max = size; // guard against recursive next pointers
            TableBucket* parent = bucket;
            do
            {
                // No key matching here
                parent = parent->next;
                --max;
            }
            while (max && parent->next != nullptr);

            bucket = nullptr;
            parent_idx = (TableEntry*) parent - entries;
        }
    }

    // Last entry will be our spot, important that this is grabbed after pre_insert(), which may
    // have relocated the entries array.
    TableEntry& entry = entries[size - 1];

    // Copy the key
    char* key_copy = new char[key_length + 1];
    strncpy_s(key_copy, key_length + 1, key, key_length);
    entry.key = key_copy;

    // Set parent
    if (bucket != nullptr)
    {
        // From bucket
        entry.parent = bucket;
        bucket->next = &entry;
    }
    else
    {
        // Chain to entry
        entry.parent = entries + parent_idx;
        entry.parent->next = &entry;
    }

    // Is this needed?
    // entry.next = nullptr;

    return entry;
}

bool Object::pre_insert()
{
    size_t new_size = size + 1;
    if (new_size > capacity)
    {
        return resize(new_size);
    }

    size = new_size;
    return false;
}

void Object::rehash()
{
    // no-op if size is 0
    if (size == 0)
    {
        return;
    }

    // Guh... clear buckets and possibly reallocate, then rehash every entry
    size_t target_buckets = ideal_buckets();
    if (total_buckets != target_buckets)
    {
        total_buckets = target_buckets;

        delete[] buckets;
        buckets = new TableBucket[total_buckets];
    }
    else
    {
        // TODO: if we are rehashing, does this always mean we are
        //       reallocating and this code is never going to run?

        // I imagine we may have other reasons to rehash without changing bucket count, so I wrote
        // this just in case, since it was pretty simple code.
        for (size_t i = 0; i < total_buckets; ++i)
        {
            buckets[i].next = nullptr;
        }
    }

    for (size_t i = 0; i < size; ++i)
    {
        TableEntry& entry = entries[i];
        size_t bucket_idx = hash(entry.key) % total_buckets;

        // No matching keys, just search for the first entry with no next
        TableBucket* parent = buckets + bucket_idx;
        size_t max = size; // safe guard against recursive next pointers
        while (max && parent->next != nullptr)
        {
            parent = parent->next;
            --max;
        }

        parent->next = &entry;
        entry.parent = parent;
    }
}

bool Object::resize(size_t new_size)
{
    size_t old_cap = capacity;

    // Double capacity
    capacity = capacity * 2;

    // Fit to new size if still too small
    if (capacity < new_size)
    {
        capacity = new_size;
    }

    // test if a rehash will happen, temporarily setting size
    // update_capacity() should not update pointers if we are going to rehash
    size_t old_size = size;
    size = new_size;
    bool do_rehash = should_rehash(); // uses current size value
    size = old_size;

    if (capacity > old_cap)
    {
        update_capacity(!do_rehash);
    }

    // Update after capacity is updated
    size = new_size;

    if (do_rehash)
    {
        rehash();
    }

    return do_rehash;
}

// Compares pointers to large objects of the same type, a >= b.
template <class Type> static inline size_t diff_pointers(const Type* a, const Type* b)
{
    return (reinterpret_cast<std::uintptr_t>(a) - reinterpret_cast<std::uintptr_t>(b)) /
           sizeof(Type);
}

void Object::update_capacity(bool update_pointers)
{
    TableEntry* old = entries;
    size_t old_size = size;

    if (capacity < 1)
    {
        entries = nullptr; // Be empty
    }
    else
    {
        entries = new TableEntry[capacity]; // TODO: should do this without constructing values
    }

    // Clamp size to capacity
    if (size > capacity)
    {
        size = capacity;
    }

    if (old == nullptr)
    {
        return; // nothing to move
    }

    // Move entries from old
    if (entries != nullptr)
    {
        for (size_t i = 0; i < size; ++i)
        {
            // Only move key pointer and value, we set next and parent manually to avoid overwriting
            // old stuff
            TableEntry& source = old[i];
            TableEntry& entry = entries[i];
            entry.key = std::exchange(source.key, nullptr);
            entry.value = std::move(source.value);

            // A rehash will happen after this, no need to set pointers
            if (!update_pointers)
            {
                continue;
            }

            // Our next hasn't been set by a previous entry's parent
            if (entry.next == nullptr && source.next != nullptr)
            {
                TableEntry* next = source.next;
                size_t max = old_size; // guard against recursive next pointers
                do
                {
                    if (next >= old && diff_pointers(next, old) < capacity)
                    {
                        // found an entry within new list
                        break;
                    }

                    next = next->next;
                    --max;
                }
                while (max && next != nullptr);

                if (next != nullptr)
                {
                    // fix next to be valid for new entries
                    next = entries + (next - old);

                    // Assign "next"'s parent to us
                    next->parent = &entry;
                }

                entry.next = next;
            }

            // Our parent hasn't been set by a previous entry's next
            // Similar to setting next but needs an additional test for bucket parents
            if (entry.parent == nullptr && source.parent != nullptr)
            {
                TableBucket* parent = source.parent;
                size_t max = old_size;
                do
                {
                    if (parent >= old && diff_pointers((TableEntry*) parent, old) < capacity)
                    {
                        // found an entry within new list
                        parent = entries + ((TableEntry*) parent - old);
                        break;
                    }

                    if (parent >= buckets && parent < buckets + total_buckets)
                    {
                        // found a bucket parent
                        break;
                    }

                    // Parent must be an entry
                    parent = ((TableEntry*) parent)->parent;
                    --max;
                }
                while (max && parent != nullptr); // should never happen

                assert(("You messed it up, parent became nullptr!", parent != nullptr));

                entry.parent = parent;

                // Point parent to us
                parent->next = &entry;
            }
        }
    }

    delete[] old;
}

void Object::copy_from(const Object& other)
{
    // Guard against self assignment
    if (this == &other)
    {
        return;
    }

    delete_data();

    // Reserve only the size
    total_buckets = other.total_buckets;
    capacity = other.size;
    size = other.size;

    // Allocate entries and buckets
    if (capacity > 0)
    {
        entries = new TableEntry[capacity];
    }

    if (total_buckets > 0)
    {
        buckets = new TableBucket[total_buckets];
    }

    // No need to copy anything if there are no entries
    if (size == 0)
    {
        return;
    }

    // Copy entries
    for (size_t i = 0; i < size; ++i)
    {
        TableEntry& source = other.entries[i];
        TableEntry& entry = entries[i];

        // copy value
        entry.value = source.value;

        // copy key, don't guard max key length here
        size_t length = strnlen(source.key, STR_LEN);
        char* key = new char[length + 1];
        strncpy_s((char*) key, length + 1, source.key, length);
        entry.key = key;

        // Next pointers are equivalent to indexes into the list
        if (source.next != nullptr)
        {
            entry.next = entries + (source.next - other.entries);
        }

        // Parent pointers could point into the Bucket list or Entry list
        // We will assume it's the Entry list here, which could be wrong. When the Bucket list
        // is processed, it will assign the correct parents.
        if (source.parent != nullptr)
        {
            entry.parent = entries + ((TableEntry*) source.parent - other.entries);
        }
    }

    // Copy buckets
    for (size_t i = 0; i < total_buckets; ++i)
    {
        TableBucket& source = other.buckets[i];
        if (source.next == nullptr)
        {
            continue;
        }

        TableBucket& bucket = buckets[i];

        // copy next pointer
        bucket.next = entries + (source.next - other.entries);

        // we know where the entry's parent bucket is now
        bucket.next->parent = &bucket;
    }
}

void Object::move_from(Object&& other)
{
    // Guard against self assignment
    if (this == &other)
    {
        return;
    }

    delete_data();

    // Keep the same capacity when moving, since we'll just be taking over the pointers
    total_buckets = std::exchange(other.total_buckets, 0);
    capacity = std::exchange(other.capacity, 0);
    size = std::exchange(other.size, 0);

    // Take pointers
    entries = std::exchange(other.entries, nullptr);
    buckets = std::exchange(other.buckets, nullptr);
}

void Object::delete_data()
{
    delete[] entries;
    delete[] buckets;

    // Clean up state
    entries = nullptr;
    buckets = nullptr;

    total_buckets = 0;
    capacity = 0;
    size = 0;
}


} // namespace Surface::JSON
