#include "json.h"

#include <cstdlib> // free, malloc, realloc
#include <utility> // move

namespace Surface::JSON
{

ParseResult parse(const char*& json)
{
    ParseResult valid = validate(json);
    if (!valid)
    {
        return valid;
    }

    return ParseResult::value(parse_no_validate(json));
}

Value parse_no_validate(const char*& json)
{
    return Value();
}

bool is_valid(const char*& json)
{
    return !!validate(json);
}

StringResult to_string(const Value& value)
{
    char str[] = "null";
    return StringResult::make(str);
}

// Internal validation method, returns an error result or an empty null result
static ParseResult validate(const char*& json)
{
    using Utility::stack;

    stack<Value::Type> stack();
}

namespace Utility
{
bool str_equal(const char*& a, const char*& b)
{
    for (size_t index = 0; index < MAX_SIZE; ++index)
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

char* str_copy(const char*& string)
{
    size_t length = 0;
    while (length < MAX_SIZE - 1)
    {
        if (string[length] == 0)
        {
            break;
        }
        ++length;
    }

    char* copy = new char[length + 1];
    for (size_t i = 0; i < length; ++i)
    {
        copy[i] = string[i];
    }
    copy[length] = 0;
    return copy;
}

template <typename type> stack<type>::stack()
{
    m_capacity = 4;
    m_elements = std::malloc(m_capacity * sizeof(type));
    m_size = 0;
}

template <typename type> stack<type>::~stack()
{
    for (size_t i = 0; i < m_size; ++i)
    {
        m_elements[i].~type();
    }

    std::free(m_elements);
    m_elements = nullptr;
    m_capacity = 0;
    m_size = 0;
}

template <typename type> bool stack<type>::empty() const
{
    return m_size == 0;
}

template <typename type> type stack<type>::pop()
{
    if (!(m_size > 0))
    {
        return type();
    }

    --m_size;
    type value = std::move(m_elements[m_size]);
    return value;
}

template <typename type> size_t stack<type>::push(type&& value)
{
    if (m_size == m_capacity)
    {
        m_capacity = m_capacity * 2;
        m_elements = std::realloc(m_elements, m_capacity * sizeof(type));
    }

    new (m_elements + m_size) type(std::move(value));
    ++m_size;
}

template <typename type> size_t stack<type>::size() const
{
    return m_size;
}

template <typename type> type& stack<type>::top()
{
    if (!(m_size > 0))
    {
        type fake{};
        return fake;
    }

    return m_elements[m_size - 1];
}

template <typename type> const type& stack<type>::top() const
{
    if (!(m_size > 0))
    {
        type fake{};
        return fake;
    }

    return m_elements[m_size - 1];
}

} // namespace Utility

ParseResult ParseResult::value(Value&& value)
{
    return ParseResult(std::move(value));
}

ParseResult ParseResult::error(const char* messege, size_t line, size_t column)
{
    return ParseResult(messege, line, column);
}

ParseResult::ParseResult(Value&& value)
{
    m_value = std::move(value);
}

ParseResult::~ParseResult()
{
    delete[] m_message;
}

ParseResult::operator bool() const
{
    return m_message == nullptr;
}

const char* ParseResult::what() const
{
    return m_message ? m_message : "No exception";
}

Value&& ParseResult::get()
{
    return std::move(m_value);
}

StringResult StringResult::make(char* string)
{
    return StringResult(string);
}

StringResult::~StringResult()
{
    delete[] m_str;
}

char* StringResult::string() const
{
    return m_str;
}

char* StringResult::take_ownership()
{
    char* temp = m_str;
    m_str = nullptr;
    return temp;
}

} // namespace Surface::JSON
