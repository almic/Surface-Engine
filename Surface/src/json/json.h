#pragma once

#include <stdint.h> // uint64_t

namespace Surface::JSON
{

struct Value;
struct Array;
struct Object;
struct ParseResult;
struct StringResult;

// Parse a json string to a json value, contained within ParseResult, which evaluates to boolean
// `false` and contains an exception message if the parse fails.
ParseResult parse(const char* json);

// Convert a json value to a string, result is wrapped for automatic resource cleanup
StringResult to_string(const Value& value);

namespace Utility
{
// String max size
inline constexpr size_t MAX_SIZE = -1;

// Copy a string and return a new string
char* str_copy(const char*& str);

// Test if two strings are equal
bool str_equal(const char*& a, const char*& b);
} // namespace Utility

struct Value
{
    enum Type : unsigned char
    {
        Null,
        Array,
        Object,
        String,
        Boolean,
        Number
    };

  public: // Contructors
    Value() : type(Null), value(0) {};
    Value(const Value& other);
    Value(Value&& other);
    ~Value();

    Value(const char* string);

    Value(bool boolean) : value(boolean ? -1 : 0), type(Boolean) {};

    Value(double number) : value(*((uint64_t*) &number)), type(Number) {};
    Value(long double number) : Value((double) number) {};
    Value(int number) : Value((double) number) {};
    Value(long int number) : Value((double) number) {};
    Value(long long int number) : Value((double) number) {};
    Value(unsigned int number) : Value((double) number) {};
    Value(unsigned long int number) : Value((double) number) {};
    Value(unsigned long long int number) : Value((double) number) {};

    static Value array(size_t capacity = 0);

    static Value object(size_t capacity = 0);

  public: // Operators
    Value& operator=(const Value& other);
    Value& operator=(Value&& other) noexcept;

    template <typename Type> Value& operator=(const Type& other)
    {
        *this = Value(other);
        return *this;
    }

    operator bool() const; // for quick null/ bool/ positive value checks

  public: // Methods
    // Clears the data of this value and makes it Null.
    void clear();

    // Get the type of this value
    Type get_type() const;

    bool is_null() const;
    bool is_array() const;
    bool is_object() const;
    bool is_string() const;
    bool is_bool() const;
    bool is_number() const;


    JSON::Array& as_array(JSON::Array& dfault);
    const JSON::Array& as_array(const JSON::Array& dfault) const;

    JSON::Object& as_object(JSON::Object& dfault);
    const JSON::Object& as_object(const JSON::Object& dfault) const;

    const char* as_string(const char* dfault) const;

    bool as_bool(bool dfault) const;

    double as_number(double dfault) const;


    JSON::Array& to_array();
    const JSON::Array& to_array() const;

    JSON::Object& to_object();
    const JSON::Object& to_object() const;

    const char* to_string() const;

    bool to_bool() const;

    double to_number() const;

  private:
    Value(Type type, uint64_t value) : type(type), value(value) {};

    void copy_other(const Value& other);

    void move_other(Value&& other);

  private: // Members
    Type type;
    uint64_t value;
};

struct Array
{
    Array(const Array& other);
    Array(Array&& other) noexcept;
    ~Array();

    Value& operator[](size_t index);
    const Value& operator[](size_t index) const;
    Array& operator=(const Array& other);
    Array& operator=(Array&& other) noexcept;

    // Add a value to the end of the array, returns the new size
    size_t append(Value&& value);

    template <typename Type> size_t append(Type value)
    {
        return append(Value(value));
    }

    // Clears the array, retaining capacity and setting size to zero
    void clear();

    // Retrieve the value at a given index
    Value& get(size_t index);
    const Value& get(size_t index) const;

    // Add a value at the specified position, must be <= size, returns the new size
    size_t insert(Value&& value, size_t index);

    template <typename Type> size_t insert(Type value, size_t index)
    {
        return insert(Value(value), index);
    }

    // Add a value to the front of the array, returns the new size
    size_t push(Value&& value);

    template <typename Type> size_t push(Type value)
    {
        return push(Value(value));
    }

    // Remove a value from the end of the array and return it
    Value pop();

    // Remove a value from the front of the array and return it
    Value pull();

    // Remove a value from the given index and return it
    Value remove(size_t index);

    // Resize to hold at least `capacity` elements, grows capacity
    void resize(size_t capacity);

    // Set the value at a given index and return the old value
    Value set(Value&& value, size_t index);

    size_t size() const
    {
        return m_size;
    }

    // Sets array capacity to current array size, freeing memory as needed
    void trim();

  private: // Members
    Value* m_entries;
    size_t m_size;
    size_t m_capacity;

  private: // Methods
    Array(size_t capacity);
    friend Value;

    void copy_other(const Array& other);

    void move_other(Array&& other) noexcept;
};

struct Object
{
    using Key = char*;

    Object(const Object& other);
    Object(Object&& other) noexcept;
    ~Object();

    struct Entry
    {
        Key key = nullptr;
        Value value;

        Entry* next = nullptr;

        Entry() {};
        Entry(const Key& key);
        Entry(const Entry& other);
        Entry(Entry&& other);
        ~Entry();

        bool operator==(const Entry& other);
    };

    Value& operator[](const Key& key);
    const Value& operator[](const Key& key) const;
    Object& operator=(const Object& other);
    Object& operator=(Object&& other) noexcept;

    // Clears the map, retaining capacity and setting size to zero
    void clear();

    // Number of buckets that the map should have given current size
    size_t desired_buckets() const;

    // Retrieve a value with the given key, returns nullptr if it does not exist
    Value* get(const Key& key);
    const Value* get(const Key& key) const;

    // Retrieve a value with the given key, returns default if it does not exist
    Value& get(const Key& key, Value& dfault);
    const Value& get(const Key& key, const Value& dfault) const;

    // Retrieve a value if it exists, or insert a null value and return it
    Value& get_or_put(const Key& key);

    // Test if the map contains the given key
    bool has(const Key& key) const;

    // Put a value at the given key, replacing an existing value if it exists, returns the new map
    // size
    size_t put(const Key& key, const Value& value);
    size_t put(const Key& key, Value&& value);

    // Get the number of mappings
    inline size_t size() const
    {
        return m_size;
    }

  public: // Hash method
    static size_t hash(const Key& key);

  private: // Members
    // Bucket map
    Entry* m_entries;

    // Number of buckets in the map, must be a power of 2
    size_t m_buckets;

    // Number of stored elements
    size_t m_size;

  private: // Constructor for Value
    Object(size_t capacity);
    friend Value;

    void copy_other(const Object& other);

    void move_other(Object&& other);

    // Rehash the entries with at least `buckets` number of buckets
    void rehash(size_t buckets);

    // Resize to hold at least `buckets` number of buckets, frees for size 0
    void resize(size_t buckets);

    // True if the map should rehash given the current size
    bool should_rehash() const;
};

// Returned by parser, evaluates to boolean `false` if there is an error message
struct ParseResult
{
    ParseResult(Value&& value);
    ParseResult(const char* message, size_t line, size_t column)
        : m_message(copy_message(message)), m_line(line), m_column(column) {};

    ~ParseResult();

    operator bool() const
    {
        return m_message == nullptr;
    }

    const char* what() const
    {
        return m_message ? m_message : "No exception";
    }

    Value&& get();

  private:
    Value m_value;
    char* m_message = nullptr;
    size_t m_line = 0, m_column = 0;

  private: // Static methods
    static char* copy_message(const char* message);
};

// Simple wrapper for a char* that deletes the string when it goes out of scope, or can release
// control of the char* if needed.
struct StringResult
{
    StringResult(char* string_to_take) : m_str(string_to_take) {};

    char* string() const
    {
        return m_str;
    }

    char* take_ownership()
    {
        char* temp = m_str;
        m_str = nullptr;
        return temp;
    }

    ~StringResult()
    {
        delete[] m_str;
    }

  private:
    char* m_str;
};

} // namespace Surface::JSON
