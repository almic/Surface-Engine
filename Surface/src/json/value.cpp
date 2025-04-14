#include "json.h"

#include <utility>

namespace Surface::JSON
{

static uint64_t cast_ptr(void* ptr)
{
    return *((uint64_t*) &ptr);
}

void Value::copy_other(const Value& other)
{
    switch (other.type)
    {
    case Null:
    {
        type = Null;
        value = 0;
        break;
    }
    case Array:
    {
        type = Array;
        JSON::Array* copy = new JSON::Array(other.to_array());
        value = cast_ptr(copy);
        break;
    }
    case Object:
    {
        type = Object;
        JSON::Object* copy = new JSON::Object(other.to_object());
        value = cast_ptr(copy);
        break;
    }
    case String:
    {
        type = String;
        char* copy = Utility::str_copy(other.to_string());
        value = cast_ptr(copy);
        break;
    }
    case Boolean:
    {
        type = Boolean;
        value = other.value;
        break;
    }
    case Number:
    {
        type = Number;
        value = other.value;
        break;
    }
    default:
    {
        // TODO: assert something in debug to crash
        type = Null;
        value = 0;
    }
    }
}

void Value::move_other(Value&& other)
{
    type = other.type;
    value = other.value;

    other.type = Null;
    other.value = 0;
}

Value::Value(const Value& other)
{
    copy_other(other);
}

Value::Value(Value&& other) noexcept
{
    move_other(std::move(other));
}

Value::~Value()
{
    clear();
}

Value::Value(const char* string)
{
    char* copy = Utility::str_copy(string);
    value = cast_ptr(copy);
    type = String;
}

Value Value::array(size_t capacity)
{
    JSON::Array* array = new JSON::Array(capacity);
    return Value(Array, cast_ptr(array));
}

Value Value::object(size_t capacity)
{
    JSON::Object* object = new JSON::Object(capacity);
    return Value(Object, cast_ptr(object));
}

Value& Value::operator=(const Value& other)
{
    if (this == &other)
    {
        return *this;
    }

    clear();
    copy_other(other);

    return *this;
}

Value& Value::operator=(Value&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    clear();
    move_other(std::move(other));

    return *this;
}

Value::operator bool() const
{
    switch (type)
    {
    case Null:
        return false;
    case Number:
        return static_cast<bool>(to_number()); // however c++ casts doubles
    case Boolean:
        return to_bool();
    case String:
        return to_string()[0] != 0; // not an empty string
    case Array:
        return to_array().size() > 0; // not an empty array
    case Object:
        return to_object().size() > 0; // not an empty map
    default:
        return false; // unknown type is false
    }
}

void Value::clear()
{
    switch (type)
    {
    case Null:
    {
        return; // already clear
    }
    case String:
    {
        delete[] (char*) value;
        break;
    }
    case Object:
    {
        delete (JSON::Object*) value;
        break;
    }
    case Array:
    {
        delete (JSON::Array*) value;
        break;
    }
    }

    type = Null;
    value = 0;
}

Value::Type Value::get_type() const
{
    return type;
}

bool Value::is_null() const
{
    return type == Null;
}

bool Value::is_array() const
{
    return type == Array;
}

bool Value::is_object() const
{
    return type == Object;
}

bool Value::is_string() const
{
    return type == String;
}

bool Value::is_bool() const
{
    return type == Boolean;
}

bool Value::is_number() const
{
    return type == Number;
}

JSON::Array& Value::as_array(JSON::Array& dfault)
{
    if (type != Array)
    {
        return dfault;
    }

    return *(JSON::Array*) value;
}

const JSON::Array& Value::as_array(const JSON::Array& dfault) const
{
    if (type != Array)
    {
        return dfault;
    }

    return *(JSON::Array*) value;
}

JSON::Object& Value::as_object(JSON::Object& dfault)
{
    if (type != Object)
    {
        return dfault;
    }

    return *(JSON::Object*) value;
}

const JSON::Object& Value::as_object(const JSON::Object& dfault) const
{
    if (type != Object)
    {
        return dfault;
    }

    return *(JSON::Object*) value;
}

const char* Value::as_string(const char* dfault) const
{
    if (type != String)
    {
        return dfault;
    }

    return (char*) value;
}

bool Value::as_bool(bool dfault) const
{
    if (type != Boolean)
    {
        return dfault;
    }

    return to_bool();
}

double Value::as_number(double dfault) const
{
    if (type != Number)
    {
        return dfault;
    }

    return *(double*) &value;
}

JSON::Array& Value::to_array()
{
    return *(JSON::Array*) value;
}

const JSON::Array& Value::to_array() const
{
    return *(JSON::Array*) value;
}

JSON::Object& Value::to_object()
{
    return *(JSON::Object*) value;
}

const JSON::Object& Value::to_object() const
{
    return *(JSON::Object*) value;
}

const char* Value::to_string() const
{
    return (char*) value;
}

bool Value::to_bool() const
{
    // true is stored as all 1s, false is all 0s
    static constexpr uint64_t MID = (((uint64_t) -1) / 3) << 1;
    return value >= MID;
}

double Value::to_number() const
{
    return *(double*) &value;
}

} // namespace Surface::JSON
