#include "json.h"

#include <bit>     // bit_ceil
#include <cstdlib> // free/ realloc
#include <utility> // move

namespace Surface::JSON
{

// Bucket constants
inline constexpr size_t MIN_BUCKETS = 8;
inline constexpr uint8_t BUCKET_SIZE = 2;
inline constexpr uint8_t BUCKET_GROWTH_HIGH = 8;
inline constexpr uint16_t BUCKET_GROWTH_BOUND = 512;

Object::Entry::Entry(const Key& key) : key(Utility::str_copy((const char*&) key))
{
}

Object::Entry::Entry(const Entry& other)
    : key(other.key ? Utility::str_copy((const char*&) other.key) : nullptr), value(other.value),
      next(other.next ? new Entry(*other.next) : nullptr)
{
}

Object::Entry::Entry(Entry&& other) : key(other.key), value(std::move(other.value))
{
    // Don't move other's next
    other.key = nullptr;
    other.next = nullptr;
}

Object::Entry::~Entry()
{
    delete[] key;
    delete next;

    key = nullptr;
    next = nullptr;
    value.~Value();
}

bool Object::Entry::operator==(const Entry& other)
{
    if (this == &other)
    {
        return true;
    }

    if (key == other.key || key == nullptr || other.key == nullptr)
    {
        return key == other.key;
    }

    return Utility::str_equal((const char*&) key, (const char*&) other.key);
}

size_t Object::hash(const Key& key)
{
    static constexpr size_t PRIME = 1099511628211;
    size_t result = 14695981039346656037;

    for (size_t index = 0; index != Utility::MAX_SIZE; ++index)
    {
        if (key[index] == 0)
        {
            break;
        }

        result ^= key[index];
        result *= PRIME;
    }

    return result;
}

void Object::copy_other(const Object& other)
{
    resize(other.m_buckets);
    m_size = other.m_size;

    if (m_size == 0)
    {
        return;
    }

    for (size_t index = 0; index < m_buckets; ++index)
    {
        new (m_entries + index) Entry(other.m_entries[index]);
    }
}

void Object::move_other(Object&& other)
{
    m_entries = other.m_entries;
    m_buckets = other.m_buckets;
    m_size = other.m_size;

    other.m_entries = nullptr;
    other.m_buckets = 0;
    other.m_size = 0;
}

Object::Object(size_t capacity)
{
    m_size = 0;
    m_entries = nullptr;
    resize(BUCKET_SIZE * capacity);
}

Object::~Object()
{
    clear();
    m_buckets = 0;

    // Free entries block
    std::free(m_entries);
    m_entries = nullptr;
}

Object::Object(const Object& other)
{
    copy_other(other);
}

Object::Object(Object&& other) noexcept
{
    move_other(std::move(other));
}

Value& Object::operator[](const Key& key)
{
    return get_or_put(key);
}

const Value& Object::operator[](const Key& key) const
{
    const Value* value = get(key);
    if (value != nullptr)
    {
        return *value;
    }

    // TODO: assert debug break
    Value fake;
    return fake;
}

Object& Object::operator=(const Object& other)
{
    if (this == &other)
    {
        return *this;
    }

    clear();
    copy_other(other);

    return *this;
}

Object& Object::operator=(Object&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    this->~Object();
    move_other(std::move(other));

    return *this;
}

void Object::clear()
{
    if (m_size == 0)
    {
        return;
    }

    for (size_t index = 0; index < m_buckets; ++index)
    {
        m_entries[index].~Entry();
    }

    m_size = 0;
}

size_t Object::desired_buckets() const
{
    size_t desired = std::bit_ceil(BUCKET_SIZE * m_size);

    // Following MSVC example, fast growth at low sizes
    if (desired > m_buckets && m_buckets < BUCKET_GROWTH_BOUND &&
        m_buckets * BUCKET_GROWTH_HIGH > desired)
    {
        return std::bit_ceil(m_buckets * BUCKET_GROWTH_HIGH);
    }

    return desired < MIN_BUCKETS ? MIN_BUCKETS : desired;
}

Value* Object::get(const Key& key)
{
    return const_cast<Value*>(static_cast<const Object&>(*this).get(key));
}

const Value* Object::get(const Key& key) const
{
    if (key == nullptr || m_size == 0)
    {
        return nullptr;
    }

    size_t bucket_index = hash(key) & (m_buckets - 1);
    Entry* entry = m_entries + bucket_index;
    do
    {
        if (entry->key == nullptr)
        {
            return nullptr;
        }

        if (Utility::str_equal((const char*&) key, (const char*&) entry->key))
        {
            return &entry->value;
        }

        entry = entry->next;
    }
    while (entry != nullptr);

    return nullptr;
}

Value& Object::get(const Key& key, Value& dfault)
{
    return const_cast<Value&>(static_cast<const Object&>(*this).get(key, dfault));
}

const Value& Object::get(const Key& key, const Value& dfault) const
{
    const Value* value = get(key);
    if (value == nullptr)
    {
        return dfault;
    }
    return *value;
}

Value& Object::get_or_put(const Key& key)
{
    if (key == nullptr)
    {
        // TODO: assert debug break
        Value fake;
        return fake;
    }

    if (m_entries == nullptr)
    {
        resize(desired_buckets());
    }

    size_t key_hash = hash(key);
    size_t bucket_index = key_hash & (m_buckets - 1);
    Entry* entry = m_entries + bucket_index;
    Entry* parent = nullptr;

    do
    {
        if (entry->key == nullptr)
        {
            break;
        }

        if (Utility::str_equal((const char*&) key, (const char*&) entry->key))
        {
            return entry->value;
        }

        parent = entry;
        entry = entry->next;
    }
    while (entry != nullptr);

    if (should_rehash())
    {
        rehash(desired_buckets());

        // Locate placement
        bucket_index = key_hash & (m_buckets - 1);
        entry = m_entries + bucket_index;
        parent = nullptr;

        do
        {
            if (entry->key == nullptr)
            {
                break;
            }

            parent = entry;
            entry = entry->next;
        }
        while (entry != nullptr);
    }

    ++m_size;

    // If entry, that's our place (main bucket)
    if (entry != nullptr)
    {
        entry->key = Utility::str_copy((const char*&) key);
        return entry->value;
    }

    // Otherwise, new entry on parent
    parent->next = new Entry(key);
    return parent->next->value;
}

bool Object::has(const Key& key) const
{
    return get(key) != nullptr;
}

size_t Object::put(const Key& key, const Value& value)
{
    Value& current = get_or_put(key);
    current = value;
    return m_size;
}

size_t Object::put(const Key& key, Value&& value)
{
    Value& current = get_or_put(key);
    current = std::move(value);
    return m_size;
}

void Object::rehash(size_t buckets)
{
    if (buckets == m_buckets)
    {
        return;
    }

    if (m_buckets == 0 || m_size == 0)
    {
        // Simple resize, nothing to rehash
        resize(buckets);
        return;
    }

    size_t old_buckets = m_buckets;
    m_buckets = buckets < MIN_BUCKETS ? MIN_BUCKETS : std::bit_ceil(buckets);
    size_t mask = m_buckets - 1;

    Entry* old_entries = m_entries;
    m_entries = new Entry[m_buckets];

    for (size_t bucket = 0; bucket < old_buckets; ++bucket)
    {
        Entry* entry = old_entries + bucket;
        if (entry->key == nullptr)
        {
            continue;
        }

        size_t max = 0;
        do
        {
            Entry* next = entry->next;

            size_t bucket_index = hash(entry->key) & mask;

            if ((m_entries + bucket_index)->key == nullptr)
            {
                // Place in bucket
                new (m_entries + bucket_index) Entry(std::move(*entry));
            }
            else
            {
                // Linked entry
                Entry* parent = m_entries + bucket_index;
                while (parent->next != nullptr)
                {
                    parent = parent->next;
                }

                // If we already have an allocated entry, just set the pointer
                if (max > 0)
                {
                    parent->next = entry;
                    entry->next = nullptr;
                }
                else
                {
                    // Moving a root entry to a sub-entry requires new memory
                    parent->next = new Entry(std::move(*entry));
                }
            }

            entry = next;
            ++max;
        }
        while (max < m_size && entry != nullptr);
    }

    std::free(old_entries);
}

void Object::resize(size_t buckets)
{
    // Effective free for size 0
    if (buckets == 0)
    {
        clear();

        if (m_entries != nullptr)
        {
            std::free(m_entries);
            m_entries = nullptr;
            m_buckets = 0;
        }

        return;
    }

    buckets = buckets < MIN_BUCKETS ? MIN_BUCKETS : std::bit_ceil(buckets);

    // Operate under the assumption that shrinking deallocates only empty entries
    m_entries = (Entry*) std::realloc(m_entries, buckets * sizeof(Entry));

    if (m_buckets < buckets)
    {
        // Initialize memory
        for (size_t offset = m_buckets; offset < buckets; ++offset)
        {
            new (m_entries + offset) Entry;
        }
    }

    m_buckets = buckets;
}

bool Object::should_rehash() const
{
    if (m_buckets == 0)
    {
        return m_size > 0;
    }

    float load = static_cast<float>(m_size) / m_buckets;
    return load >= 2.f || (m_buckets > MIN_BUCKETS && load < 0.5f);
}

} // namespace Surface::JSON
