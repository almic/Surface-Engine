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

// State machine parser, only states that may traverse whitespace are allowed.
// Strings, numbers, and primitives are parsed in a single step.
enum State : unsigned char
{
    // Initial state, expecting the first value
    VALUE,

    // Start of an array, expecting a value or end
    ARRAY_START,

    // Parsing an array, expecting a value
    ARRAY_VALUE,

    // Parsing an array, expecting a ',' or ']'
    ARRAY_END,

    // Start of an object, expecting a key or end
    OBJECT_START,

    // Expecting a string key for an object
    OBJECT_KEY,

    // Expecting a ':' between a string key and a value
    OBJECT_COLON,

    // Parsing an object, expecting a value
    OBJECT_VALUE,

    // Parsing an object, expecting a ',' or '}'
    OBJECT_END,

    // First value parsed, expecting end of input
    END,
};

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

// Validate a string for the validate method
static bool validate_string(const char*& json, size_t& next, const size_t& line, size_t& column,
                            ParseResult& error);

// Internal validation method, returns an error result or an empty null result
static ParseResult validate(const char*& json)
{
    using Type = Value::Type;

    Utility::stack<Type> stack;
    ParseResult str_error = ParseResult::value(Value());
    State state = VALUE;
    size_t line = 1;
    size_t column = 0;
    size_t next = 0;

    // Skip UTF-8 Byte Order Mark
    if (json[0] == 0xEF && json[1] == 0xBB && json[2] == 0xBF)
    {
        next = 3;
    }

// Common code when ending an array/ object
#define POP_STACK                                                                                  \
    stack.pop();                                                                                   \
    switch (stack.top())                                                                           \
    {                                                                                              \
    case Type::Null:                                                                               \
    {                                                                                              \
        state = END;                                                                               \
        break;                                                                                     \
    }                                                                                              \
    case Type::Array:                                                                              \
    {                                                                                              \
        state = ARRAY_END;                                                                         \
        break;                                                                                     \
    }                                                                                              \
    case Type::Object:                                                                             \
    {                                                                                              \
        state = OBJECT_END;                                                                        \
        break;                                                                                     \
    }                                                                                              \
    }                                                                                              \
    0

// Common code after parsing a value
#define NEXT_VALUE                                                                                 \
    switch (state)                                                                                 \
    {                                                                                              \
    case ARRAY_START:                                                                              \
    case ARRAY_VALUE:                                                                              \
    {                                                                                              \
        state = ARRAY_END;                                                                         \
        break;                                                                                     \
    }                                                                                              \
    case OBJECT_VALUE:                                                                             \
    {                                                                                              \
        state = OBJECT_END;                                                                        \
        break;                                                                                     \
    }                                                                                              \
    case VALUE:                                                                                    \
    {                                                                                              \
        state = END;                                                                               \
        break;                                                                                     \
    }                                                                                              \
    }                                                                                              \
    0

    char c;

    // Validation loop
    do
    {
        // Whitespace loop
        do
        {
            c = json[next];
            ++next;

            if (c == '\n')
            {
                ++line;
                column = 0;
            }

            ++column;
        }
        while (next < Utility::MAX_SIZE && (c == ' ' || c == '\n' || c == '\r' || c == '\t'));

        switch (state)
        {
        case ARRAY_START:
        {
            if (c == ']')
            {
                POP_STACK;
                break;
            }
            [[fallthrough]];
        }
        case VALUE:
        case ARRAY_VALUE:
        case OBJECT_VALUE:
        {
            switch (c)
            {
            case '[':
            {
                state = ARRAY_START;
                stack.push(Type::Array);
                break;
            }
            case '{':
            {
                state = OBJECT_START;
                stack.push(Type::Object);
                break;
            }
            case 't':
            {
                // NOTE: since none of symbols "true", "false", and "null" contain the starting
                // letter of another symbol, there is no need to check for `next` overflowing to
                // zero and potentially looping forever. But, also, that's insane!

                if (json[next] != 'r' || json[++next] != 'u' || json[++next] != 'e')
                {
                    return ParseResult::error("Invalid symbol", line, column);
                }

                column += 3;
                NEXT_VALUE;
                break;
            }
            case 'f':
            {
                if (json[next] != 'a' || json[++next] != 'l' || json[++next] != 's' ||
                    json[++next] != 'e')
                {
                    return ParseResult::error("Invalid symbol", line, column);
                }

                column += 4;
                NEXT_VALUE;
                break;
            }
            case 'n':
            {
                if (json[next] != 'u' || json[++next] != 'l' || json[++next] != 'l')
                {
                    // TODO: problem
                    return ParseResult::error("Invalid symbol", line, column);
                }

                column += 3;
                NEXT_VALUE;
                break;
            }
            case '"':
            {
                if (!validate_string(json, next, line, column, str_error))
                {
                    return str_error;
                }

                NEXT_VALUE;
                break;
            }
            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                if (c == '-')
                {
                    c = json[next];
                    ++next;
                    ++column;
                }

                if (c == '0')
                {
                    c = json[next];
                    ++next;
                    ++column;
                }
                else
                {
                    if (c < '1' || c > '9')
                    {
                        return ParseResult::error("Invalid number after `-` character", line,
                                                  column);
                    }

                    c = json[next];
                    while (c >= '0' && c <= '9' && next < Utility::MAX_SIZE)
                    {
                        ++next;
                        ++column;
                        c = json[next];
                    }
                }

                if (c == '.')
                {
                    c = json[next];
                    ++next;
                    ++column;

                    if (c < '0' || c > '9')
                    {
                        return ParseResult::error("Invalid number after `.` character", line,
                                                  column);
                    }

                    c = json[next];
                    while (c >= '0' && c <= '9' && next < Utility::MAX_SIZE)
                    {
                        ++next;
                        ++column;
                        c = json[next];
                    }
                }

                if (c == 'e' || c == 'E')
                {
                    c = json[next];
                    ++next;
                    ++column;

                    if (c == '-' || c == '+')
                    {
                        c = json[next];
                        ++next;
                        ++column;
                    }

                    if (c < '0' || c > '9')
                    {
                        return ParseResult::error("Invalid number in exponent", line, column);
                    }

                    c = json[next];
                    while (c >= '0' && c <= '9' && next < Utility::MAX_SIZE)
                    {
                        ++next;
                        ++column;
                        c = json[next];
                    }
                }

                NEXT_VALUE;
                break;
            }
            default:
            {
                return ParseResult::error("Unknown symbol, expecting a value", line, column);
            }
            }
        }
        case ARRAY_END:
        {
            switch (c)
            {
            case ']':
            {
                POP_STACK;
                break;
            }
            case ',':
            {
                state = ARRAY_VALUE;
                break;
            }
            default:
            {
                return ParseResult::error("Unknown symbol, expecting `]` or `,` in array", line,
                                          column);
            }
            }
        }
        case OBJECT_START:
        {
            switch (c)
            {
            case '}':
            {
                POP_STACK;
                break;
            }
            case '"':
            {
                if (!validate_string(json, next, line, column, str_error))
                {
                    return str_error;
                }

                state = OBJECT_COLON;
                break;
            }
            default:
            {
                return ParseResult::error("Unknown symbol, expecting `}` or `\"` in object", line,
                                          column);
            }
            }
            break;
        }
        case OBJECT_KEY:
        {
            if (c != '"')
            {
                return ParseResult::error("Unknown symbol, expecting `\"` for object key", line,
                                          column);
            }

            if (!validate_string(json, next, line, column, str_error))
            {
                return str_error;
            }

            state = OBJECT_COLON;
            break;
        }
        case OBJECT_COLON:
        {
            if (c == ':')
            {
                state = OBJECT_VALUE;
                break;
            }

            return ParseResult::error("Unknown symbol, expecting `:` following object key", line,
                                      column);
        }
        case OBJECT_END:
        {
            switch (c)
            {
            case '}':
            {
                POP_STACK;
                break;
            }
            case ',':
            {
                state = OBJECT_KEY;
                break;
            }
            default:
            {
                return ParseResult::error(
                    "Unknown symbol, expecting `}` or `,` following object key", line, column);
            }
            }
        }
        case END:
        {
            if (c != '\0')
            {
                return ParseResult::error("Unexpected character, no more values are expected", line,
                                          column);
            }

            // This will terminate the loop
            next = Utility::MAX_SIZE;
            break;
        }
        }
    }
    while (next < Utility::MAX_SIZE);

    return ParseResult::value(Value());
}

static bool validate_string(const char*& json, size_t& next, const size_t& line, size_t& column,
                            ParseResult& error)
{
    char c;
    do
    {
        c = json[next];
        ++next;
        ++column;

        switch (c)
        {
        case '"':
        {
            break;
        }
        case '\\':
        {
            c = json[next];
            ++next;
            ++column;

            switch (c)
            {
            case '\\':
            case '/':
            case 'b':
            case 'f':
            case 'n':
            case 'r':
            case 't':
            {
                break;
            }
            case 'u':
            {
                uint16_t unicode = 0;
                if (!Utility::parse_4hex(json + next, unicode))
                {
                    error = ParseResult::error("Expecting 4 hex digits following `\\u` in string",
                                               line, column);
                    return false;
                }

                next += 4;
                column += 4;

                if (unicode >= 0xD800 && unicode <= 0xDBFF)
                {
                    if (json[next] != '\\' || json[++next] != 'u')
                    {
                        error =
                            ParseResult::error("Expecting a second unicode escape after reading a "
                                               "leading surrogate value (0xD800 to 0xDBFF)",
                                               line, column);
                        return false;
                    }

                    ++next;
                    if (!Utility::parse_4hex(json + next, unicode))
                    {
                        error = ParseResult::error(
                            "Expecting 4 hex digits following `\\u` in string", line, column);
                        return false;
                    }

                    next += 4;
                    column += 4;
                }

                break;
            }
            default:
            {
                error = ParseResult::error("Invalid escape sequence in string", line, column);
                return false;
            }
            }
        }
        default:
        {
            if (c < 0x20)
            {
                error = ParseResult::error(
                    "Invalid character in string, control codes must be escaped", line, column);
                return false;
            }
        }
        }
    }
    while (next < Utility::MAX_SIZE && c != '"');

    return true;
}

namespace Utility
{

bool parse_4hex(const char* hex, uint16_t& result)
{
    for (size_t i = 0; i < 4; ++i)
    {
        result *= 16;
        // clang-format off
        switch (hex[i])
        {
        case '0':
        {
            break;
        }
        case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        {
            result += hex[i] - '0';
            break;
        }
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        {
            result += 10 + (hex[i] - 'a');
            break;
        }
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        {
            result += 10 + (hex[i] - 'A');
            break;
        }
        default:
        {
            return false;
        }
        }
        // clang-format on
    }

    return true;
}

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

void ParseResult::move_other(ParseResult&& other)
{
    m_message = other.m_message;
    m_line = other.m_line;
    m_column = other.m_column;
    m_value = std::move(other.m_value);

    other.m_message = nullptr;
    other.m_line = 0;
    other.m_column = 0;
}

ParseResult::ParseResult(ParseResult&& other)
{
    move_other(std::move(other));
}

ParseResult::~ParseResult()
{
    delete[] m_message;
}

ParseResult::operator bool() const
{
    return m_message == nullptr;
}

ParseResult& ParseResult::operator=(ParseResult&& other)
{
    delete[] m_message;

    move_other(std::move(other));

    return *this;
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
