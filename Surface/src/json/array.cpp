#include "json.h"

#include <cstdlib> // free/ realloc
#include <cstring> // memmove
#include <utility> // move

namespace Surface::JSON
{

static Value fake_value = nullptr;

void Array::copy_other(const Array& other)
{
    resize(other.m_capacity);
    m_size = other.m_size;

    if (!other.m_entries)
    {
        return;
    }

    for (int index = 0; index < m_size; ++index)
    {
        m_entries[index] = other.m_entries[index];
    }
}

void Array::move_other(Array&& other) noexcept
{
    m_capacity = other.m_capacity;
    m_entries = other.m_entries;
    m_size = other.m_size;

    other.m_capacity = 0;
    other.m_entries = nullptr;
    other.m_size = 0;
}

Array::Array(size_t capacity) : m_size(0), m_entries(nullptr), m_capacity(0)
{
    resize(capacity);
}

Array::~Array()
{
    clear(); // destruct all elements, sets size to zero
    m_capacity = 0;

    // Free entries block
    std::free(m_entries);
    m_entries = nullptr;
}

Array::Array(const Array& other) : m_size(0), m_entries(nullptr), m_capacity(0)
{
    copy_other(other);
}

Array::Array(Array&& other) noexcept
{
    move_other(std::move(other));
}

Value& Array::operator[](size_t index)
{
    // Support adding new values if requesting the last element
    if (index == m_size)
    {
        insert(nullptr, index);
    }

    return get(index);
}

const Value& Array::operator[](size_t index) const
{
    return get(index);
}

Array& Array::operator=(const Array& other)
{
    if (this == &other)
    {
        return *this;
    }

    clear();
    copy_other(other);

    return *this;
}

Array& Array::operator=(Array&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    this->~Array();
    move_other(std::move(other));

    return *this;
}

size_t Array::append(Value&& value)
{
    return insert(std::move(value), m_size);
}

void Array::clear()
{
    if (m_size == 0)
    {
        return;
    }

    for (size_t index = 0; index < m_size; ++index)
    {
        m_entries[index].~Value();
    }

    m_size = 0;
}

Value& Array::get(size_t index)
{
    if (!(index < m_size))
    {
        // TODO: assert debug break
        return fake_value;
    }

    return m_entries[index];
}

const Value& Array::get(size_t index) const
{
    if (!(index < m_size))
    {
        // TODO: assert debug break
        return fake_value;
    }

    return m_entries[index];
}

size_t Array::insert(Value&& value, size_t index)
{
    if (index > m_size || index < 0)
    {
        // TODO: assert debug break
        return m_size;
    }

    size_t new_size = m_size + 1;
    if (new_size > m_capacity)
    {
        // Double capacity
        size_t new_capacity = m_capacity * 2;
        if (new_capacity < new_size)
        {
            new_capacity = new_size;
        }

        resize(new_capacity);
    }

    // Move tailing elements back
    if (m_size - index >= 1)
    {
        // clang-format off
        std::memmove(
            m_entries + index + 1, 
            m_entries + index, 
            (m_size - index) * sizeof(Value)
        );
        // clang-format on
    }

    // insert at index
    new (m_entries + index) Value(std::move(value));

    m_size = new_size;

    return m_size;
}

size_t Array::push(Value&& value)
{
    return insert(std::move(value), 0);
}

Value Array::pop()
{
    return remove(m_size - 1);
}

Value Array::pull()
{
    return remove(0);
}

Value Array::remove(size_t index)
{
    if (index >= m_size)
    {
        // TODO: assert debug break
        Value fake;
        return fake;
    }

    Value result = std::move(m_entries[index]);

    // Move all following elements back
    if (index + 1 < m_size)
    {
        // clang-format off
        std::memmove(
            m_entries + index, 
            m_entries + index + 1,
            (m_size - (index + 1)) * sizeof(Value)
        );
        // clang-format on
    }

    --m_size;

    return result;
}

void Array::resize(size_t capacity)
{
    if (capacity <= m_capacity)
    {
        return;
    }

    m_entries = (Value*) std::realloc(m_entries, capacity * sizeof(Value));
    m_capacity = capacity;
}

Value Array::set(Value&& value, size_t index)
{
    if (index >= m_size)
    {
        // TODO: assert debug break
        Value fake;
        return fake;
    }

    Value result = std::move(m_entries[index]);
    m_entries[index] = std::move(value);

    return result;
}

void Array::trim()
{
    if (m_size == 0 || !(m_capacity > m_size))
    {
        return;
    }

    m_capacity = m_size;

    // Since this always shrinks the memory, it must always succeed
#pragma warning(disable : 6308)
    m_entries = (Value*) std::realloc(m_entries, m_capacity * sizeof(Value));
#pragma warning(default : 6308)
}

} // namespace Surface::JSON
