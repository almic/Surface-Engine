#include "json.h"

#include <cmath>   // isnan, isinf
#include <cstdio>  // snprintf
#include <cstdlib> // free, malloc, realloc
#include <limits>  // double precision
#include <utility> // move

namespace Surface::JSON
{

Value array(size_t capacity)
{
    return Value::array(capacity);
}

Value object(size_t capacity)
{
    return Value::object(capacity);
}

// Internal validation method
static ParseResult validate(const char*& json);

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
    // TODO
    return Value();
}

bool is_valid(const char*& json)
{
    return !!validate(json);
}

StringResult to_string(const Value& value)
{
    using Type = Value::Type;
    using Utility::string_builder;

    switch (value.get_type())
    {
    case Type::Boolean:
    {
        char* str = value ? new char[]{"true"} : new char[]{"false"};
        return StringResult::make(str);
    }
    case Type::String:
    {
        auto string = value.to_string();
        size_t length = Utility::str_len(string);
        string_builder str(length + 2);
        str.append('"');
        for (size_t index = 0; index < length; ++index)
        {
            char c = string[index];

            // Escape control characters
            if (c < 0x20)
            {
                str.append('\\');
                switch (c)
                {
                case '\b':
                {
                    str.append('b');
                    break;
                }
                case '\f':
                {
                    str.append('f');
                    break;
                }
                case '\n':
                {
                    str.append('n');
                    break;
                }
                case '\r':
                {
                    str.append('r');
                    break;
                }
                case '\t':
                {
                    str.append('t');
                    break;
                }
                default:
                {
                    str.append('u');

                    if (c < 0x10)
                    {
                        str.append("000");
                    }
                    else
                    {
                        str.append("001");
                        c -= 0x10;
                    }

                    if (c < 0xA)
                    {
                        c = '0' + c;
                    }
                    else
                    {
                        c = 'A' + (c - 0xA);
                    }

                    str.append(c);
                    break;
                }
                }
            }
            // Must be ecaped
            else if (c == '"' || c == '\\')
            {
                str.append('\\');
                str.append(c);
            }
            // Unescaped
            else
            {
                str.append(c);
            }
        }
        str.append('"');
        return str.build();
    }
    case Type::Number:
    {
        double dbl = value.to_number();

        switch (std::fpclassify(dbl))
        {
        case FP_INFINITE:
        {
            // Use max / min value instead
            dbl =
                dbl > 0.0 ? std::numeric_limits<double>::max() : std::numeric_limits<double>::min();

            // format as finite value
            [[fallthrough]];
        }
        case FP_NORMAL:
        case FP_SUBNORMAL:
        {
            // 9 = 1 sign character + 1 leading digit + 1 decimal point + 1 exponent + 1 exponent
            // sign + 3 exponent digits + 1 null character -0.E+XXX\0
            static constexpr int NUMBER_EXTRA = 9;
            static constexpr int NUMBER_PRECISION = std::numeric_limits<double>::digits10;
            static constexpr size_t NUMBER_BUFF_SIZE = NUMBER_PRECISION + 9;

            // Use stl formatting
            char* number = new char[NUMBER_BUFF_SIZE]{0};
            std::snprintf(number, NUMBER_BUFF_SIZE, "%.*G", NUMBER_PRECISION, dbl);
            return StringResult::make(number);
        }
        case FP_ZERO:
        // NAN and unknowns default to 0
        case FP_NAN:
        default:
        {
            break;
        }
        }

        char* str = new char[]{"0"};
        return StringResult::make(str);
    }
    case Type::Array:
    {
        string_builder str;
        str.append('[');
        auto& array = value.to_array();
        size_t size = array.size();
        if (size > 0)
        {
            str.append(to_string(array[0]).string());
            for (size_t i = 1; i < size; ++i)
            {
                str.append(',');
                str.append(to_string(array[i]).string());
            }
        }
        str.append(']');
        return str.build();
    }
    case Type::Object:
    {
        string_builder str;
        str.append('{');
        auto& object = value.to_object();
        if (object.size() > 0)
        {
            bool first = true;
            for (auto& [key, value] : object.entries())
            {
                if (!first)
                {
                    str.append(',');
                }

                str.append('"');
                str.append(key);
                str.append("\":");
                str.append(to_string(value).string());

                if (first)
                {
                    first = false;
                }
            }
        }
        str.append('}');
        return str.build();
    }
    case Type::Null:
    default:
    {
        break;
    }
    }

    char* str = new char[]{"null"};
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
} // namespace Surface::JSON

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

bool str_equal(const char* a, const char* b)
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

char* str_copy(const char* string)
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

size_t str_len(const char* string)
{
    size_t length = 0;

    while (length < MAX_SIZE)
    {
        if (string[length] == 0)
        {
            break;
        }
        ++length;
    }

    return length;
}

template <typename type> stack<type>::stack()
{
    m_capacity = 4;
    m_elements = (type*) std::malloc(m_capacity * sizeof(type));
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
        m_elements = (type*) std::realloc(m_elements, m_capacity * sizeof(type));
    }

    new (m_elements + m_size) type(std::move(value));
    return ++m_size;
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

string_builder::string_builder()
{
    resize(8);
}

string_builder::string_builder(size_t capacity)
{
    resize(capacity);
}

string_builder::~string_builder()
{
    delete[] c_str;
    m_capacity = 0;
    m_size = 0;
}

void string_builder::append(char c)
{
    resize(m_size + 1);
    c_str[m_size - 1] = c;
}

void string_builder::append(const char* string)
{
    size_t length = 0;
    while (string[length] > 0)
    {
        ++length;
    }

    size_t old_size = m_size;
    resize(m_size + length);

    for (size_t i = 0; i < length; ++i)
    {
        c_str[old_size + i] = string[i];
    }
}

void string_builder::append(unsigned int codepoint)
{
    if (codepoint < 0x80)
    {
        resize(m_size + 1);
        c_str[m_size - 1] = codepoint;
    }
    else if (codepoint < 0x800)
    {
        resize(m_size + 2);
        c_str[m_size - 2] = 0xC0 | (codepoint >> 6);
        c_str[m_size - 1] = 0x80 | (codepoint & 0x3F);
    }
    else if (codepoint < 0x10000)
    {
        resize(m_size + 3);
        c_str[m_size - 3] = 0xE0 | (codepoint >> 12);
        c_str[m_size - 2] = 0x80 | ((codepoint >> 6) & 0x3F);
        c_str[m_size - 1] = 0x80 | (codepoint & 0x3F);
    }
    else if (codepoint < 0x110000)
    {
        resize(m_size + 4);
        c_str[m_size - 4] = 0xF0 | (codepoint >> 18);
        c_str[m_size - 3] = 0x80 | ((codepoint >> 12) & 0x3F);
        c_str[m_size - 2] = 0x80 | ((codepoint >> 6) & 0x3F);
        c_str[m_size - 1] = 0x80 | (codepoint & 0x3F);
    }
}

StringResult string_builder::build()
{
    // trim to size first
    if (m_size < m_capacity)
    {
        c_str = (char*) std::realloc(c_str, m_size);
    }

    StringResult result = StringResult::make(c_str);

    c_str = nullptr;
    m_capacity = 0;
    m_size = 0;

    return result;
}

void string_builder::resize(size_t size)
{
    if (size <= m_capacity)
    {
        m_size = size;
        return;
    }

    size_t old_cap = m_capacity;
    m_capacity *= 2;
    if (m_capacity < size)
    {
        m_capacity = size;
    }

    c_str = (char*) std::realloc(c_str, m_capacity + 1);
    for (size_t i = old_cap + 1; i < m_capacity + 1; ++i)
    {
        c_str[i] = '\0';
    }

    m_size = size;

    return;
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

const char* StringResult::string() const
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
