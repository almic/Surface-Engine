#include "json.h"

#include <utility> // move

namespace Surface::JSON
{

inline constexpr size_t MAX_SIZE = -1;

static bool str_equal(const Object::Key& a, const Object::Key& b)
{
    for (size_t index = 0; index != MAX_SIZE; ++index)
    {
        if (a[index] == 0 || b[index] == 0)
        {
            return a[index] == b[index];
        }

        if (a[index] != b[index])
        {
            return false;
        }
    }

    return true; // uhhh, got some ram huh?
}

size_t Object::hash(const Key& key)
{
    static constexpr size_t PRIME = 1099511628211;
    size_t result = 14695981039346656037;

    for (size_t index = 0; index != MAX_SIZE; ++index)
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
}

void Object::move_other(Object&& other)
{
}

Object::Object(size_t capacity)
{
}

Object::~Object()
{
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
}

const Value& Object::operator[](const Key& key) const
{
}

size_t Object::desired_buckets() const
{
}

Value* Object::get(const Key& key)
{
}

const Value* Object::get(const Key& key) const
{
}

Value& Object::get(const Key& key, Value& dfault)
{
}

const Value& Object::get(const Key& key, const Value& dfault) const
{
}

bool Object::has(const Key& key) const
{
}

size_t Object::put(const Key& key, const Value& value)
{
}

size_t Object::put(const Key& key, Value&& value)
{
}

void Object::rehash(size_t buckets)
{
}

} // namespace Surface::JSON
