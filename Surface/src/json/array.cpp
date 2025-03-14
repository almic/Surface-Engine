#include "array.h"

#include "internal/assert.h"

#include <utility> // std::move()

namespace Surface::JSON
{
Array::Array(size_t capacity)
{
#ifdef DEBUG
    ++__constructs;
#endif

    this->capacity = capacity;
    update_capacity();
}

Array::Array(const Array& other) // Copy
{
#ifdef DEBUG
    ++__constructs;
#endif

    copy_from(other);
}

Array::Array(Array&& other) noexcept // Move
{
#ifdef DEBUG
    ++__constructs;
#endif

    move_from(std::move(other));
}

Array::~Array()
{
    delete_data();
}

Value& Array::operator[](size_t index)
{
    assert(("Index must be less than size", index < size));

    return values[index];
}

const Value& Array::operator[](size_t index) const
{
    assert(("Index must be less than size", index < size));

    return values[index];
}

Array& Array::operator=(const Array& other) // Copy
{
    // Guard against self assignment
    if (this == &other)
    {
        return *this;
    }

    delete_data();

    copy_from(other);

    return *this;
}

Array& Array::operator=(Array&& other) noexcept // Move
{
    // Guard against self assignment
    if (this == &other)
    {
        return *this;
    }

    delete_data();

    move_from(std::move(other));

    return *this;
}

size_t Array::clear()
{
    size_t old_size = size;
    size = 0;

    for (size_t index = 0; index < old_size; ++index)
    {
        values[index].delete_data();
    }

    return old_size;
}

size_t Array::find(const Value& value) const
{
    for (size_t i = 0; i < size; ++i)
    {
        if (values[i] == value)
        {
            return i;
        }
    }
    return -1;
}

size_t Array::insert(const Value& value, size_t index)
{

    assert(("Insertion index must be less than or equal to size", index <= size));

    _pre_insert(index);

    // Copy into position
    values[index] = value;

    return size;
}

size_t Array::insert(Value&& value, size_t index)
{
    assert(("Insertion index must be less than or equal to size", index <= size));

    _pre_insert(index);

    // Move into position
    values[index] = std::move(value);

    return size;
}

size_t Array::push(Value&& value)
{
    return insert(std::move(value), size);
}

Value Array::pop()
{
    if (size == 0)
    {
        return Value(Type::Invalid, 0);
    }

    --size;
    return Value(std::move(values[size]));
}

bool Array::remove(const Value& value)
{
    if (size == 0)
    {
        return false;
    }

    size_t index = 0;
    for (; index < size; ++index)
    {
        if (values[index] == value)
        {
            break;
        }
    }

    if (index == size)
    {
        return false;
    }

    // Delete data
    values[index].delete_data();

    --size;

    for (; index < size; ++index)
    {
        values[index] = std::move(values[index + 1]);
    }

    return true;
}

size_t Array::remove_all(const Value& value)
{
    if (size == 0)
    {
        return 0;
    }

    size_t removed = 0, index = 0;

    // Use removed and index together to track two pointers, the next free space and the current
    // value being checked. This lets us perform only 1 move per remaining element, and only if a
    // move was needed to fill a hole left by a removed element.
    do
    {
        Value& val = values[index + removed];
        if (val == value)
        {
            // Delete data
            val.delete_data();

            ++removed;
            continue;
        }

        if (removed)
        {
            values[index] = std::move(val);
        }

        ++index;
    }
    while (index + removed < size);

    size -= removed;
    return removed;
}

bool Array::set_capacity(size_t new_capacity)
{
    if (capacity == new_capacity)
    {
        return false;
    }

    capacity = new_capacity;
    update_capacity();

    return true;
}

void Array::_pre_insert(size_t index)
{
    size_t new_size = size + 1;
    if (new_size > capacity)
    {
        resize(new_size);
    }
    else
    {
        size = new_size;
    }

    // Shortcircuit when there's no need to shift indexes
    if (index == size - 1)
    {
        return;
    }

    // Shift elements up, size is guaranteed to be at least 2.
    for (size_t i = size - 2; i >= index; --i)
    {
        // TODO: This kinda sucks, is there a better way?
        values[i + 1] = std::move(values[i]);

        // umm... is my computer okay? i did not think of the underflow.
        if (i == 0)
        {
            break;
        }
    }

    return;
}

void Array::resize(size_t new_size)
{
    size_t old_cap = capacity;

    // Double capacity
    capacity = capacity * 2;

    // Fit to new size if still too small
    if (capacity < new_size)
    {
        capacity = new_size;
    }

    if (capacity > old_cap)
    {
        update_capacity();
    }

    // Update after capacity is updated
    size = new_size;
}

void Array::update_capacity()
{
    Value* old = values;

    if (capacity < 1)
    {
        values = nullptr; // Be empty
    }
    else
    {
        values = new Value[capacity]; // TODO: should do this without constructing values
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

    // Move values from old
    if (values != nullptr)
    {
        for (size_t i = 0; i < size; ++i)
        {
            values[i] = std::move(old[i]);
        }
    }

    delete[] old;
}

void Array::copy_from(const Array& other)
{
#ifdef DEBUG
    ++__copies;
#endif

    // Reserve only the size
    capacity = other.size;
    size = other.size;

    if (size == 0)
    {
        return; // Done
    }

    update_capacity();

    // copy values
    for (size_t i = 0; i < size; ++i)
    {
        values[i] = Value(other.values[i]); // copy and move
    }
}

void Array::move_from(Array&& other)
{
#ifdef DEBUG
    ++__moves;
#endif

    // Retain capacity since we are moving
    capacity = std::exchange(other.capacity, 0);
    size = std::exchange(other.size, 0);

    // Simply exchange the pointer
    values = std::exchange(other.values, nullptr);
}

void Array::delete_data()
{
    delete[] values;

    // clean up
    values = nullptr;

    capacity = 0;
    size = 0;
}

} // namespace Surface::JSON
