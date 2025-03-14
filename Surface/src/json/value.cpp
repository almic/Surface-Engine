#include "value.h"

#include "array.h"
#include "internal/assert.h"
#include "object.h"

#include <stdint.h> // SIZE_MAX macro
#include <string.h> // strnlen functions
#include <utility>  // std::move()

#undef NULL

#define STR_LEN SIZE_MAX - 1

// Value type
namespace Surface::JSON
{

Value::Value(const char* string)
{
#ifdef DEBUG
    ++__constructs;
#endif

    // Copy string to new location
    size_t length = strnlen(string, STR_LEN);
    char* d = new char[length + 1];
    strncpy_s(d, length + 1, string, length);

    // save pointer on data
    data = _to_data(d);
    type = Type::String;
}

Value::Value(const Value& other) // Copy
{
#ifdef DEBUG
    ++__constructs;
#endif

    // Copying null is disallowed
    assert(("Cannot copy JSON::NULL. Please check for null with `&other == &JSON::NULL`.",
            other.type != Type::Null));

    copy_from(other);
}

Value::Value(Value&& other) noexcept // Move
{
#ifdef DEBUG
    ++__constructs;
#endif

    // The compiler should disallow moving JSON::NULL since it is const.
    // Moving Value::NULL should be disallowed as well since it is also const.

    move_from(std::move(other));
}

Value::~Value()
{
    delete_data();
}

Value Value::object(size_t capacity)
{
    Object* obj = new Object(capacity);
    return Value(Type::Object, _to_data(obj));
}

Value Value::array(size_t capacity)
{
    Array* jhn_hlo = new Array(capacity);
    return Value(Type::Array, _to_data(jhn_hlo));
}

Value& Value::operator=(const Value& other) // Copy
{
    // Guard against self assignment
    if (&other == this)
    {
        return *this;
    }

    // Assigning null is disallowed
    assert(("Assigning to JSON:NULL is not allowed.", type != Type::Null));
    // Copying null is disallowed
    assert(("Cannot assign JSON::NULL. Please check for null with `other == JSON::NULL`.",
            other.type != Type::Null));

    delete_data();

    copy_from(other);

    return *this;
}

Value& Value::operator=(Value&& other) noexcept // Move
{
    // Guard against self assignment
    if (&other == this)
    {
        return *this;
    }

    // Assigning null is disallowed
    assert(("Assigning to JSON:NULL is not allowed.", type != Type::Null));
    // Moving null is disallowed
    assert(("Cannot move JSON::NULL. Please check for null with `other == JSON::NULL`.",
            other.type != Type::Null));

    delete_data();

    move_from(std::move(other));

    return *this;
}

bool Value::operator==(const Value& right) const
{
    if (type != right.type)
    {
        return false;
    }

    // Quickly test Null and Invalid
    if (type == Type::Null)
    {
        return true;
    }

    // Invalid are never equal to each other
    if (type == Type::Invalid)
    {
        return false;
    }

    switch (type)
    {
    case Type::Object:
    {
        // Compare using object comparison
        return this->to_object() == right.to_object();
    }
    case Type::Array:
    {
        // Compare using array comparison
        return this->to_array() == right.to_array();
    }
    case Type::String:
    {
        // Compare using string null termination rules
        const char* self = this->to_string();
        const char* other = right.to_string();

        for (size_t i = 0; i < STR_LEN; ++i)
        {
            if (self[i] == 0 || other[i] == 0)
            {
                return self[i] == other[i];
            }

            if (self[i] != other[i])
            {
                return false;
            }
        }

        return true; // that's... not... impossible. Improbable? Very.
    }
    case Type::Number:
    {
        // Simply compare the number variant and data value.
        // This means `uint 1 != int 1`.
        return this->number_variant == right.number_variant && this->data == right.data;
    }
    case Type::Bool:
    {
        // Compare boolean results, since data could in theory be different
        return this->to_bool() == right.to_bool();
    }
    default:
    {
        return false; // Should never be reached
    }
    }
}

/**
 * @brief Return this Value as an object. Returns `fallback` if this Value is not an object.
 * @param fallback default object value
 * @return Object
 */
Object Value::as_object(const Object& fallback) const
{
    if (!is_object())
    {
        return fallback;
    }

    return to_object();
}

/**
 * @brief Return this Value as an array. Returns `fallback` if this Value is not an array.
 * @param fallback default array value
 * @return Array
 */
Array Value::as_array(const Array& fallback) const
{
    if (!is_array())
    {
        return fallback;
    }

    return to_array();
}

/**
 * @brief Return this Value as a string. Returns `fallback` if this Value is not a string.
 * @param fallback default string value
 * @return string
 */
const char* Value::as_string(const char* fallback) const
{
    if (!is_string())
    {
        return fallback;
    }

    return to_string();
}

/**
 * @brief Return this Value as a signed integer. Returns `fallback` if this Value is not a
 * number.
 * @param fallback default integer value
 * @return signed integer
 */
long long Value::as_int(long long fallback) const
{
    if (!is_number())
    {
        return fallback;
    }

    return to_int();
}

/**
 * @brief Return this Value as an unsigned integer. Returns `fallback` if this Value is not a
 * number.
 * @param fallback default unsigned integer value
 * @return unsigned integer
 */
unsigned long long Value::as_uint(unsigned long long fallback) const
{
    if (!is_number())
    {
        return fallback;
    }

    return to_uint();
}

/**
 * @brief Return this Value as a double. Returns `fallback` if this Value is not a number.
 * @param fallback default double value
 * @return double
 */
double Value::as_double(double fallback) const
{
    if (!is_number())
    {
        return fallback;
    }

    return to_double();
}

/**
 * @brief Return this Value as a boolean. Returns `fallback` if this Value is not a boolean.
 * @param fallback default boolean value
 * @return Boolean
 */
bool Value::as_bool(bool fallback) const
{
    if (!is_bool())
    {
        return fallback;
    }

    return to_bool();
}

/**
 * @brief Get this value as an object. Behavior undefined if the value is not an object.
 * @return Object
 */
Object& Value::to_object() const
{
    assert(("This value is invalid because it was moved or deleted", type != Type::Invalid));
    assert(("Value is not an object", is_object()));

    return *((Object*) data); // data points to object
}

/**
 * @brief Get this value as an array. Behavior undefined if the value is not an array.
 * @return Array
 */
Array& Value::to_array() const
{
    assert(("This value is invalid because it was moved or deleted", type != Type::Invalid));
    assert(("Value is not an array", is_array()));

    return *((Array*) data); // data points to array
}

/**
 * @brief Get this value as a string. Behavior undefined if the value is not a string.
 * @return String
 */
const char* Value::to_string() const
{
    assert(("This value is invalid because it was moved or deleted", type != Type::Invalid));
    assert(("Value is not a string", is_string()));

    return (char*) data; // data is the char*
}

long long Value::to_int() const
{
    assert(("This value is invalid because it was moved or deleted", type != Type::Invalid));
    assert(("Value is not a number", is_number()));

    switch (number_variant)
    {
    case 2: // double
        return (long long) _to_double();
    case 1: // int
        return _to_int();
    case 0:
    default:
        return (long long) data;
    }
}

unsigned long long Value::to_uint() const
{
    assert(("This value is invalid because it was moved or deleted", type != Type::Invalid));
    assert(("Value is not a number", is_number()));

    switch (number_variant)
    {
    case 2: // double
        return (unsigned long long) _to_double();
    case 1: // int
        return (unsigned long long) _to_int();
    case 0:
    default:
        return data;
    }
}

/**
 * @brief Get this value as a double. Behavior undefined if the value is not a number.
 * @return double
 */
double Value::to_double() const
{
    assert(("This value is invalid because it was moved or deleted", type != Type::Invalid));
    assert(("Value is not a number", is_number()));

    switch (number_variant)
    {
    case 2: // double
        return _to_double();
    case 1: // int
        return (double) _to_int();
    case 0: // unsigned int
    default:
        return (double) data;
    }
}

/**
 * @brief Get this value as a boolean. Behavior undefined if the value is not a boolean.
 * @return Boolean
 */
bool Value::to_bool() const
{
    assert(("This value is invalid because it was moved or deleted", type != Type::Invalid));
    assert(("Value is not a boolean", is_bool()));

    // You might be asking: Why?
    // I answer: don't trust 64 bits to stay zero...
    // Also, this means any set of bits is true 50% of the time. But, divided by 3? This was
    // necessary to alternate the bits 0101, see the binary form for yourself. It was a choice
    // between 0xA (1010) and 0x5 (0101), and 0x5 just needs a divide by 3.

    // You might now ask: Why do you want this to return true 50% of the time for any set of bits?
    // I answer: because I think the undefined behavior should quickly look wrong, and what's more
    // obviously incorrect than randomness? If you load a settings file and all the options look
    // disturbed, that's going to be noticed quickly. But if `true` happened to be the default in
    // most situations, then bad data could look correct if we simply `return data != 0`.

    // a value with alternating bits
    static const unsigned long long lim = (unsigned long long) -1 / 3;
    return !(data < lim);
}

void Value::copy_from(const Value& other)
{
#ifdef DEBUG
    ++__copies;
#endif

    // We must copy referenced objects
    switch (other.type)
    {
    case Type::Object:
    {
        Object* cpy = new Object(other.to_object());
        data = _to_data(cpy);
        type = Type::Object;
        break;
    }
    case Type::Array:
    {
        Array* cpy = new Array(other.to_array());
        data = _to_data(cpy);
        type = Type::Array;
        break;
    }
    case Type::String:
    {
        const char* str = other.to_string();
        size_t length = strnlen(str, STR_LEN);
        char* cpy = new char[length + 1];
        strncpy_s(cpy, length + 1, str, length);

        data = _to_data(cpy);
        type = Type::String;
        break;
    }
    default:
        // Copy by value
        data = other.data;
        type = other.type;
        number_variant = other.number_variant;
    }
}

void Value::move_from(Value&& other)
{
#ifdef DEBUG
    ++__moves;
#endif

    // Pretty simple, just set other data to zero
    // data is always a pointer or is the data itself
    data = other.data;
    type = other.type;
    number_variant = other.number_variant;

    // When moving, make other invalid, we have stolen the resources
    other.invalidate();
}

// Delete referenced data if it exists
void Value::delete_data()
{
    switch (type)
    {
    case Type::Object:
    {
        Object& o = to_object();
        delete &o;
        break;
    }
    case Type::Array:
    {
        Array& a = to_array();
        delete &a;
        break;
    }
    case Type::String:
    {
        delete[] ((char*) data);
        break;
    }
    }

    invalidate();
}

void Value::invalidate()
{
    data = 0;
    number_variant = 0;
    type = Type::Invalid; // no longer safe to use
}

// Interpret the bits as a double
double Value::_to_double() const
{
    return *((double*) &data);
}

// Interpret the bits as an integer
long long Value::_to_int() const
{
    return *((long long*) &data);
}

// Cast a pointer to a uint
unsigned long long Value::_to_data(void* ptr)
{
    return *((unsigned long long*) &ptr);
}

} // namespace Surface::JSON
