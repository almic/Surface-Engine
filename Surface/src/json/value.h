#pragma once

#include "type.h"

#include <utility> // std::move()

#pragma push_macro("NULL")
#undef NULL

namespace Surface::JSON
{

struct Object;
struct Array;

// JSON value
struct Value
{
    // The JSON null value
    const static Value NULL;

  public: // Constructors
    // Create a value from a string
    Value(const char* string);

    // Create a value from a floating point number
    Value(double number)
        : type(Type::Number), data(*((unsigned long long*) &number)), number_variant(2)
    {
#ifdef DEBUG
        ++__constructs;
#endif
    };

    // Create a value from a signed integer
    Value(long long number)
        : type(Type::Number), data(*((unsigned long long*) &number)), number_variant(1)
    {
#ifdef DEBUG
        ++__constructs;
#endif
    };

    Value(int number) : Value((long long) number)
    {
    }

    // Create a value from an unsigned integer
    Value(unsigned long long number) : type(Type::Number), data(number)
    {
#ifdef DEBUG
        ++__constructs;
#endif
    };

    Value(unsigned int number) : Value((unsigned long long) number)
    {
    }

    // Create a value from a bool
    Value(bool boolean)
        : type(Type::Bool), data(boolean ? ((unsigned long long) -1) /* Use all 64 bits */ : 0)
    {
#ifdef DEBUG
        ++__constructs;
#endif
    };

    // Copy
    Value(const Value& other);

    // Move
    Value(Value&& other) noexcept;

    ~Value();

  public: // Static constructors for Object and Array
    // Create an object value, with an optional initial capacity, defaults to 2.
    static Value object(size_t capacity = 2);

    // Create an array value, with an optional initial capacity, defaults to 2.
    static Value array(size_t capacity = 2);

  public: // Operators
    // Move
    Value& operator=(Value&& other) noexcept;
    // Copy
    Value& operator=(const Value& other);

    // Move
    template <class Type> Value& operator=(Type&& other) noexcept
    {
        return (*this = Value(std::move(other)));
    }

    // Copy
    template <class Type> Value& operator=(const Type& other)
    {
        return (*this = Value(other));
    }

    // Equality/ Inequality, mainly useful for JSON::NULL checks. When comparing numbers, this will
    // consider numbers of different underlying types as unequal. This means `uint 1 != int 1`. It
    // is preferred to retrieve both numbers in the form you desire for comparison and do the
    // comparison on the numbers directly.
    bool operator==(const Value& right) const;

    bool operator!=(const Value& right) const
    {
        return !(*this == right);
    }

  public: // Methods
    // NOTE: It causes me great pain that null, true, and false must consume 16 bytes of memory to
    // represent. But, I suppose that's just the way of dynamic languages.

    /**
     * @brief Return this Value as an object. Returns `fallback` if this Value is not an object.
     * @param fallback default object value
     * @return Object
     */
    Object as_object(const Object& fallback) const;

    /**
     * @brief Return this Value as an array. Returns `fallback` if this Value is not an array.
     * @param fallback default array value
     * @return Array
     */
    Array as_array(const Array& fallback) const;

    /**
     * @brief Return this Value as a string. Returns `fallback` if this Value is not a string.
     * @param fallback default string value
     * @return string
     */
    const char* as_string(const char* fallback) const;

    /**
     * @brief Return this Value as a signed integer. Returns `fallback` if this Value is not a
     * number.
     * @param fallback default integer value
     * @return signed integer
     */
    long long as_int(long long fallback) const;

    /**
     * @brief Return this Value as an unsigned integer. Returns `fallback` if this Value is not a
     * number.
     * @param fallback default unsigned integer value
     * @return unsigned integer
     */
    unsigned long long as_uint(unsigned long long fallback) const;

    /**
     * @brief Return this Value as a double. Returns `fallback` if this Value is not a number.
     * @param fallback default double value
     * @return double
     */
    double as_double(double fallback) const;

    /**
     * @brief Return this Value as a boolean. Returns `fallback` if this Value is not a boolean.
     * @param fallback default boolean value
     * @return Boolean
     */
    bool as_bool(bool fallback) const;

    /**
     * @brief Get this value as an object. Behavior undefined if the value is not an object.
     * @return Object
     */
    Object& to_object() const;

    /**
     * @brief Get this value as an array. Behavior undefined if the value is not an array.
     * @return Array
     */
    Array& to_array() const;

    /**
     * @brief Get this value as a string, does not do the same thing as `JSON::to_string(Value)`.
     * Behavior is undefined if the value is not a string.
     * @return String
     */
    const char* to_string() const;

    /**
     * @brief Get this value as an integer. Behavior undefined if the value is not a number.
     * @return integer
     */
    long long to_int() const;

    /**
     * @brief Get this value as an unsigned integer. Although the underlying data of JSON::Value is
     * stored as an unsigned integer, if it represents a number like a signed integer or double,
     * this will return a converted value.
     *
     * If you would like the raw data value, use `get_data()`.
     *
     * @return unsigned integer
     */
    unsigned long long to_uint() const;

    /**
     * @brief Get this value as a double. Behavior undefined if the value is not a number.
     * @return double
     */
    double to_double() const;

    /**
     * @brief Get this value as a boolean. Behavior undefined if the value is not a boolean.
     * @return Boolean
     */
    bool to_bool() const;

    // Get the plain data value stored by this JSON::Value
    inline unsigned long long get_data() const
    {
        return data;
    }

    // Get the type of this value
    inline Type get_type() const
    {
        return type;
    }

    // Test if the type is an array
    inline bool is_array() const
    {
        return type == Type::Array;
    }

    // Test if the type is a boolean
    inline bool is_bool() const
    {
        return type == Type::Bool;
    }

    // Test if the type is a number
    inline bool is_number() const
    {
        return type == Type::Number;
    }

    // Test if the type is an object
    inline bool is_object() const
    {
        return type == Type::Object;
    }

    // Test if the type is a string
    inline bool is_string() const
    {
        return type == Type::String;
    }

    // Test if the type is a number and holds a double. Only use this to deduce the raw type, you
    // can use any of the `as_number()` methods regardless.
    inline bool is_double() const
    {
        return type == Type::Number && number_variant == 2;
    }

    // Test if the type is a number and holds a signed integer. Only use this to deduce the raw
    // type, you can use any of the `as_number()` methods regardless.
    inline bool is_int() const
    {
        return type == Type::Number && number_variant == 1;
    }

    // Test if the type is a number and holds an unsigned integer. Only use this to deduce the raw
    // type, you can use any of the `as_number()` methods regardless.
    inline bool is_uint() const
    {
        return type == Type::Number && number_variant == 0;
    }

    // Test if this value is valid. Values become invalid when moved, as opposed to proliferating
    // nulls. Invalid values can become valid again if a valid value is copied or moved into them.
    bool is_valid() const
    {
        return type != Type::Invalid;
    }

  private: // Private constructors
    // For NULL, object() and array()
    Value(Type type, unsigned long long value) : type(type), data(value)
    {
#ifdef DEBUG
        ++__constructs;
#endif
    };

    // This is for Object/ Array constructors
    Value() : type(Type::Invalid), data(0)
    {
#ifdef DEBUG
        ++__constructs;
#endif
    };

  private: // Private members
    // What type is this value
    Type type;

    // Hint for what a stored number's bits represent.
    // 0 = uint, 1 = int, 2 = double
    char number_variant = 0;

    // The data of this value.
    // Is a pointer for object, array, and string. Is the value itself for number and bool.
    unsigned long long data;

  private: // private methods
    // Copy referenced data from other
    void copy_from(const Value& other);

    // Move referenced data from other
    void move_from(Value&& other);

    // Delete referenced data if it exists and invalidate
    void delete_data();

    // Marks the value as invalid. You must ensure data is moved or deleted before this or you WILL
    // leak the memory!
    void invalidate();

    // Interpret the bits as a double
    double _to_double() const;

    // Interpret the bits as an integer
    long long _to_int() const;

    // Cast a pointer to a uint
    static unsigned long long _to_data(void* ptr);

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
    friend Object;
    friend Array;
};

} // namespace Surface::JSON

#pragma pop_macro("NULL")
