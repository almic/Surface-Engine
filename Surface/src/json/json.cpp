#include "json.h"

#include "array.h"
#include "internal/assert.h"
#include "object.h"
#include "value.h"

#include <cstdint>
#include <cstring>
#include <format>

#define STR_LEN SIZE_MAX - 1

#undef NULL

// Value type
namespace Surface::JSON
{

const Value Value::NULL(Type::Null, 0);
const Value& NULL = Value::NULL;

#ifdef DEBUG
int Value::__constructs = 0;
int Value::__copies = 0;
int Value::__moves = 0;

int Array::__constructs = 0;
int Array::__copies = 0;
int Array::__moves = 0;

int Object::__constructs = 0;
int Object::__copies = 0;
int Object::__moves = 0;
#endif

Value _parse(const char* json, bool strict) noexcept;

Value parse(const char* json) noexcept
{
    return _parse(json, false);
}

Value strict_parse(const char* json)
{
    return _parse(json, true);
}

// Helper for building strings, can stack strings with push()/ pop()
struct StringBuilder
{
    StringBuilder(size_t capacity);
    ~StringBuilder();

    // Adds the character to the end of the string
    void append(const char c);

    // Adds text to the end of the string
    void append(const char* text);

    // Adds string of uint to the end of the string
    void append(size_t uint);

    // Returns the current string buffer after inserting a null character, valid for the lifetime of
    // the Reader, invalidated if new characters are appended.
    char* get_string();

    // Resets the internal length to 0 such that future calls to append() and get_string() start
    // over from the beginning. Does not modify the buffer contents.
    void clear();

    // Appends a null, sets the starting point of the next get_string() to the new length
    void push();

    // Moves the internal start point and length back to where they were before the last call to
    // push()
    void pop();

  private:
    inline void resize(size_t new_size);

    char* buffer = nullptr;
    size_t start = 0;
    size_t length = 0;
    size_t capacity = 0;
};

// Helper to generate internal parse exceptions
static Exception _parse_exception_internal(StringBuilder& buffer, size_t code, size_t line,
                                           size_t column)
{
    buffer.push();
    buffer.append("Parsing exception occurred on line ");
    buffer.append(line);
    buffer.append(", column ");
    buffer.append(column);
    buffer.append(". Error #");
    buffer.append(code);
    buffer.append('.');

    return Exception(buffer.get_string(), line, column);
}

static Exception _parse_error_complete(StringBuilder& buffer, size_t line, size_t column)
{
    buffer.append(" on line ");
    buffer.append(line);
    buffer.append(", column ");
    buffer.append(column);
    buffer.append('.');

    return Exception(buffer.get_string(), line, column);
}

Value _parse(const char* json, bool strict) noexcept
{
    size_t length = strnlen(json, STR_LEN);

    // No
    if (length == STR_LEN)
    {
        return NULL;
    }

    static enum State
    {
        // Expecting a value
        VALUE,

        // Both VALUE and END
        VALUE_OR_END,

        // Reading a string in double quotes
        STRING,

        // Reading the first part of a number, expecting digits, decimal, or exponent.
        // Have either read a negative sign ('-'), or a digit.
        NUMBER,

        // Reading the fractional part of a number, expecting digits or exponent.
        NUMBER_FRACTION,

        // Reading the exponent, expecting a sign or digits.
        NUMBER_EXP_SIGN,

        // Reading the digits of an exponent
        NUMBER_EXPONENT,

        // Reading escape sequence within a string
        ESCAPE,

        // Expecting a colon ':'
        COLON,

        // Expecting ']' or '}' or ',' to appear.
        END_OR_COMMA,

        // Expecting end of input
        TERMINATE
    };

    // State machine time
    State state = VALUE;

    // Using my own array as a stack type :)
    Value value_stack = Value::array(16);
    auto& stack = value_stack.to_array();

    // I want few allocations while processing, so start with a lot
    StringBuilder buffer(1024);
    size_t index = 0;
    size_t max = length;

    // may not be used but tracked anyway
    size_t pos = 0;
    size_t line = 1;

    while (max)
    {
        --max;

        char next = json[index];
        ++index;
        ++pos;

        if (next == '\n' && state != STRING)
        {
            pos = 0;
            ++line;
            continue;
        }

        switch (state)
        {
        case ESCAPE:
        {
            switch (next)
            {
                // These are the only valid escapes for JSON strings
            case '"':
            case '\\':
            case '/':
            {
                buffer.append(next);
                break;
            }
            case 'b':
            {
                buffer.append('\b');
                break;
            }
            case 'f':
            {
                buffer.append('\f');
                break;
            }
            case 'n':
            {
                buffer.append('\n');
                break;
            }
            case 'r':
            {
                buffer.append('\r');
                break;
            }
            case 't':
            {
                buffer.append('\t');
                break;
            }
            case 'u':
            {
                // Read 4 hex digits
                if (!(index + 3 < length))
                {
                    // Ran out of input while reading unicode escape
                    if (!strict)
                    {
                        return NULL;
                    }
                    buffer.push();
                    buffer.append("Unexpected end of input while reading unicode escape");
                    throw _parse_error_complete(buffer, line, pos);
                }

                unsigned int val = 0;
                for (char i = 0; i < 4; ++i)
                {
                    val *= 16;

                    next = json[index];
                    ++index;
                    ++pos;
                    if (next >= '0' && next <= '9')
                    {
                        val += next - '0';
                    }
                    else if (next >= 'a' && next <= 'f')
                    {
                        val += 10 + (next - 'a');
                    }
                    else if (next >= 'A' && next <= 'F')
                    {
                        val += 10 + (next - 'A');
                    }
                    else
                    {
                        // Invalid hex digit in unicode escape
                        if (!strict)
                        {
                            return NULL;
                        }
                        buffer.push();
                        buffer.append("Invalid ");
                        if (next >= 32 && next < 127)
                        {
                            buffer.append("hex digit '");
                            buffer.append(next);
                            buffer.append('\'');
                        }
                        else if (next >= 0)
                        {
                            buffer.append("codepoint ");
                            buffer.append((size_t) next);
                        }
                        else
                        {
                            buffer.append("character");
                        }

                        buffer.append(" while reading unicode escape");
                        throw _parse_error_complete(buffer, line, pos);
                    }
                }

                if (val == 0)
                {
                    // Encode a null character with two bytes
                    // This is generally how nulls should be encoded into utf8 codepoints
                    buffer.append((char) 0xC0);
                    buffer.append((char) 0x80);
                }
                else if (val < 0x80)
                {
                    // ascii encoding
                    buffer.append((char) val);
                }
                else if (val < 0x800)
                {
                    // two bytes
                    char a = 0xC0 | (val >> 6);
                    char b = 0x80 | (val & 0x3F);
                    buffer.append(a);
                    buffer.append(b);
                }
                else
                {
                    // three bytes
                    char a = 0xE0 | (val >> 12);
                    char b = 0x80 | ((val >> 6) & 0x3F);
                    char c = 0x80 | (val & 0x3F);
                    buffer.append(a);
                    buffer.append(b);
                    buffer.append(c);
                }
                break;
            }
            default:
            {
                if (!strict)
                {
                    return NULL;
                }

                // Invalid escaped character
                buffer.push();
                buffer.append("Invalid escape sequence ");
                if (next >= 32 && next < 127)
                {
                    buffer.append("\"\\");
                    buffer.append(next);
                    buffer.append('"');
                }
                else if (next == 0)
                {
                    buffer.append("\"\\0\"");
                }
                throw _parse_error_complete(buffer, line, pos);
            }
            }

            state = STRING;
            break;
        }
        case STRING:
        {
            switch (next)
            {
            case '"':
            {
                char* string = buffer.get_string();
                buffer.pop();

                // Single string value
                if (stack.is_empty())
                {
                    stack.push(Value(string));
                    state = TERMINATE;
                    break;
                }

                auto top = stack.last();

                if (top->is_object())
                {
                    // save key to be used later once the value is parsed
                    stack.push(Value(string));
                    state = COLON;
                    break;
                }

                if (top->is_array())
                {
                    top->to_array().push(Value(string));
                    state = END_OR_COMMA;
                    break;
                }

                if (top->is_string())
                {
                    auto key = stack.pop().to_string();
                    // Expect to get an object

                    if (stack.is_empty())
                    {
                        assert(("You messed up, read a string value with a string value on the "
                                "stack, expected object but stack was empty",
                                false));

                        if (!strict)
                        {
                            return NULL;
                        }

                        throw _parse_exception_internal(buffer, 0x11, line, pos);
                    }

                    auto value_obj = stack.last();
                    if (!value_obj->is_object())
                    {
                        assert(("You messed up, read a string value and popped a string key, but "
                                "top value is not an object",
                                false));

                        if (!strict)
                        {

                            return NULL;
                        }

                        throw _parse_exception_internal(buffer, 0x12, line, pos);
                    }

                    value_obj->to_object().set(key, Value(string));
                    state = END_OR_COMMA;
                    break;
                }

                assert(("You messed up, read a string value but top of stack is neither "
                        "object, array, nor string",
                        false));

                if (!strict)
                {
                    return NULL;
                }

                throw _parse_exception_internal(buffer, 0x1F, line, pos);
            }
            case '\\':
            {
                state = ESCAPE;
                break;
            }
            default:
            {
                if (next == 0x7F || (next >= 0 && next < 0x20))
                {
                    // Control characters are not allowed to appear in strings
                    if (!strict)
                    {
                        return NULL;
                    }

                    buffer.push();
                    buffer.append("Invalid string, control characters are not valid in JSON "
                                  "strings. Found control character ");
                    buffer.append((size_t) next);
                    throw _parse_error_complete(buffer, line, pos);
                }

                if (next >= 0x20 && next < 0x7F)
                {
                    buffer.append(next);
                    break;
                }

                // TODO: parse UTF-8 codepoints...
                assert(("UTF-8 not implemented. Do it now!!", false));
                return NULL;
            }
            }
        }
        }
    }
}

// Helper for writing to string buffer, and being conservative about memory use. This is meant to be
// used when you have a very good idea on exactly how much memory you will be needing, and don't
// need all the fancy <string> stuff.
struct Writer
{
    Writer(size_t size);
    ~Writer();

    // Write a single character
    void write_char(char chr);
    // Write a character N times
    void write_char_n(char chr, size_t n);
    // Write text
    void write_text(const char* text, size_t count);

    // Resets the buffer and internal index
    void clear();

    // Releases the internal buffer to the caller
    char* take_buffer();

    // Resizes the buffer, if needed, to the current index. This always leaves at least one
    // additional spot for a null terminator character. Returns the current index, which would be
    // pointing to the final element in the buffer.
    size_t trim();

    // Get the number of bytes currently written to this Writer
    inline size_t bytes_written() const
    {
        return index;
    }

  public: // For back insert iterator
    using value_type = char;

    inline void push_back(char chr)
    {
        write_char(chr);
    }

    inline void push_back(char&& chr)
    {
        write_char(chr);
    }

    inline std::back_insert_iterator<Writer>& back_itr()
    {
        return _back_itr;
    }

  private:
    char* buffer;
    size_t size;
    size_t index = 0;
    std::back_insert_iterator<Writer> _back_itr = std::back_inserter(*this);

  private:
    void resize(size_t new_size);
};

// Declare methods
static size_t _compute_size(const Value& value, Writer& out, unsigned int& indent,
                            unsigned int space, unsigned int line_length);

static bool _to_string(const Value& value, Writer& out, unsigned int& indent, unsigned int space,
                       unsigned int line_length);

char* to_string(const Value& value, unsigned int space, unsigned int array_line_length) noexcept
{

    size_t size;
    unsigned int indent = 0;
    {
        Writer wr(25); // Pre-initialized for number length calculations
        size = _compute_size(value, wr, indent, space, array_line_length);
    }

    // Now write for real
    Writer wr(size + 1);
    indent = 0;
    if (_to_string(value, wr, indent, space, array_line_length))
    {
        size_t length = wr.trim();
        char* out = wr.take_buffer();
        out[length] = 0;
        return out;
    }

    return nullptr;
}

// Declare methods
static void _to_string(const Object& object, Writer& out, unsigned int& indent, unsigned int space,
                       unsigned int line_length);
static void _to_string(const Array& array, Writer& out, unsigned int& indent, unsigned int space,
                       unsigned int line_length);

// Implementations
template <class Number> void _format_number(Writer& out, Number&& number)
{
    std::format_to(out.back_itr(), "{}", number);
}

static bool _to_string(const Value& value, Writer& out, unsigned int& indent, unsigned int space,
                       unsigned int line_length)
{
    switch (value.get_type())
    {

    case Type::Object:
    {
        _to_string(value.to_object(), out, indent, space, line_length);
        return true;
    }

    case Type::Array:
    {
        _to_string(value.to_array(), out, indent, space, line_length);
        return true;
    }

    case Type::String:
    {
        const char* str = value.to_string();
        size_t length = strnlen(str, STR_LEN);

        // Output: "str"
        out.write_char('"');
        out.write_text(str, length);
        out.write_char('"');

        return true;
    }

    case Type::Number:
    {
        // Output formatted number
        if (value.is_double())
        {
            _format_number(out, value.to_double());
        }
        else if (value.is_int())
        {
            _format_number(out, value.to_int());
        }
        else
        {
            _format_number(out, value.to_uint());
        }

        return true;
    }

    case Type::Bool:
    {
        if (value.to_bool())
        {
            // Output: true
            out.write_text("true", 4);
        }
        else
        {
            // Output: false
            out.write_text("false", 5);
        }

        return true;
    }

    case Type::Null:
    {
        // Output: null
        out.write_text("null", 4);
        return true;
    }

    case Type::Invalid:
    default:
        return false;
    }
    return true;
}

static void _to_string(const Object& object, Writer& out, unsigned int& indent, unsigned int space,
                       unsigned int line_length)
{
    out.write_char('{');
    const size_t size = object.get_size();

    if (size == 0)
    {
        out.write_char('}');
        return;
    }

    if (space == 0) // Newlines only
    {
        out.write_char('\n');

        bool do_comma = false;
        size_t keylen;

        for (size_t i = 0; i < size; ++i)
        {
            auto& [key, value] = object.entry_at(i);

            // Don't attempt to write invalid values
            if (!value.is_valid())
            {
                continue;
            }

            keylen = strnlen(key, STR_LEN);

            if (do_comma)
            {
                out.write_text(",\n", 2);
            }
            else
            {
                do_comma = true;
            }

            out.write_char('"');
            out.write_text(key, keylen);
            out.write_text("\":", 2); // No space after colon

            // output value inline
            _to_string(value, out, indent, space, line_length);
        }

        // newline for final '}'
        out.write_char('\n');
    }
    else if (space != -1) // Newlines and indentation
    {
        ++indent;
        const size_t num_spaces = indent * static_cast<size_t>(space);

        out.write_char('\n');
        out.write_char_n(' ', num_spaces);

        bool do_comma = false;
        size_t keylen;

        for (size_t i = 0; i < size; ++i)
        {
            auto& [key, value] = object.entry_at(i);

            // Don't attempt to write invalid values
            if (!value.is_valid())
            {
                continue;
            }

            keylen = strnlen(key, STR_LEN);

            if (do_comma)
            {
                out.write_text(",\n", 2);
                out.write_char_n(' ', num_spaces);
            }
            else
            {
                do_comma = true;
            }

            out.write_char('"');
            out.write_text(key, keylen);
            out.write_text("\": ", 3); // Space between value and colon

            // output value inline
            _to_string(value, out, indent, space, line_length);
        }

        // Add newline and space for final '}'
        out.write_char('\n');
        out.write_char_n(' ', num_spaces - space);
        --indent;
    }
    else // No whitespace
    {
        bool do_comma = false;
        size_t keylen;

        for (size_t i = 0; i < size; ++i)
        {
            auto& [key, value] = object.entry_at(i);

            // Don't attempt to write invalid values
            if (!value.is_valid())
            {
                continue;
            }

            keylen = strnlen(key, STR_LEN);

            if (do_comma)
            {
                out.write_char(',');
            }
            else
            {
                do_comma = true;
            }

            out.write_char('"');
            out.write_text(key, keylen);
            out.write_text("\":", 2); // no space after colon

            // output value
            _to_string(value, out, indent, space, line_length);
        }
    }

    out.write_char('}');
}

static void _to_string(const Array& array, Writer& out, unsigned int& indent, unsigned int space,
                       unsigned int line_length)
{
    // TODO: Implement line_length restrictions so numbers, strings, bools, and nulls can stay on
    // the same line
    out.write_char('[');

    if (array.get_size() == 0)
    {
        out.write_char(']');
        return;
    }

    if (space == 0) // Newlines only
    {
        out.write_char('\n');

        bool do_comma = false;

        for (auto& value : array)
        {
            // Don't attempt to write invalid values
            if (!value.is_valid())
            {
                continue;
            }

            if (do_comma)
            {
                out.write_text(",\n", 2);
            }
            else
            {
                do_comma = true;
            }

            // output value
            _to_string(value, out, indent, space, line_length);
        }

        // newline for final ']'
        out.write_char('\n');
    }
    else if (space != -1) // Newlines and indentation
    {
        ++indent;
        const size_t num_spaces = indent * static_cast<size_t>(space);

        out.write_char('\n');
        out.write_char_n(' ', num_spaces);

        bool do_comma = false;

        for (auto& value : array)
        {
            // Don't attempt to write invalid values
            if (!value.is_valid())
            {
                continue;
            }

            if (do_comma)
            {
                out.write_text(",\n", 2);
                out.write_char_n(' ', num_spaces);
            }
            else
            {
                do_comma = true;
            }

            // output value
            _to_string(value, out, indent, space, line_length);
        }

        // Add newline and space for final ']'
        out.write_char('\n');
        out.write_char_n(' ', num_spaces - space);
        --indent;
    }
    else // No whitespace
    {
        bool do_comma = false;

        for (auto& value : array)
        {
            // Don't attempt to write invalid values
            if (!value.is_valid())
            {
                continue;
            }

            if (do_comma)
            {
                out.write_char(',');
            }
            else
            {
                do_comma = true;
            }

            // output value
            _to_string(value, out, indent, space, line_length);
        }
    }

    out.write_char(']');
}

// Declare methods
static size_t _compute_size(const Object& object, Writer& out, unsigned int& indent,
                            unsigned int space, unsigned int line_length);
static size_t _compute_size(const Array& array, Writer& out, unsigned int& indent,
                            unsigned int space, unsigned int line_length);

// Implementations
static size_t _compute_size(const Value& value, Writer& out, unsigned int& indent,
                            unsigned int space, unsigned int line_length)
{

    switch (value.get_type())
    {

    case Type::Object:
    {
        return _compute_size(value.to_object(), out, indent, space, line_length);
    }

    case Type::Array:
    {
        return _compute_size(value.to_array(), out, indent, space, line_length);
    }

    case Type::String:
    {
        // Just string length, plus two for the quotes
        return strnlen(value.to_string(), STR_LEN) + 2;
    }

    case Type::Number:
    {
        // Unfortunately, we must format the number to know its size.
        // TODO: investigate if there is a faster way to compute this.
        out.clear();
        if (value.is_double())
        {
            _format_number(out, value.to_double());
        }
        else if (value.is_int())
        {
            _format_number(out, value.to_int());
        }
        else
        {
            _format_number(out, value.to_uint());
        }
        return out.bytes_written();
    }

    case Type::Bool:
    {
        return value.to_bool() ? 4 /* true */ : 5 /* false */;
    }

    case Type::Null:
    {
        return 4 /* null */;
    }

    case Type::Invalid:
    default:
        return 0;
    }
    return 0;
}

static size_t _compute_size(const Object& object, Writer& out, unsigned int& indent,
                            unsigned int space, unsigned int line_length)
{
    size_t result = 2; // at least '{' and '}'
    const size_t size = object.get_size();

    if (size == 0)
    {
        return result;
    }

    if (space == 0) // Newlines only
    {
        ++result; // newline after '{'

        bool do_comma = false;
        size_t keylen;

        for (size_t i = 0; i < size; ++i)
        {
            auto& [key, value] = object.entry_at(i);

            // Ignore invalid values
            if (!value.is_valid())
            {
                continue;
            }

            keylen = strnlen(key, STR_LEN);

            if (do_comma)
            {
                result += 2; // comma and newline
            }
            else
            {
                do_comma = true;
            }

            // key, two quotes, a colon, NO SPACE, and value
            result += keylen + 3 + _compute_size(value, out, indent, space, line_length);
        }

        ++result; // newline for final '}'
    }
    else if (space != -1) // Newlines and indentation
    {
        ++indent;
        const size_t num_spaces = indent * static_cast<size_t>(space);

        result += 1 + num_spaces; // newline after '{' plus indentation

        bool do_comma = false;
        size_t keylen;

        for (size_t i = 0; i < size; ++i)
        {
            auto& [key, value] = object.entry_at(i);

            // Ignore invalid values
            if (!value.is_valid())
            {
                continue;
            }

            keylen = strnlen(key, STR_LEN);

            if (do_comma)
            {
                result += 2 + num_spaces; // comma, newline, and indentation
            }
            else
            {
                do_comma = true;
            }

            // key, two quotes, a colon, a space, and value
            result += keylen + 4 + _compute_size(value, out, indent, space, line_length);
        }

        result += 1 + (num_spaces - space); // newline and indentation for final '}'
        --indent;
    }
    else // No whitespace
    {
        bool do_comma = false;
        size_t keylen;

        for (size_t i = 0; i < size; ++i)
        {
            auto& [key, value] = object.entry_at(i);

            // Ignore invalid values
            if (!value.is_valid())
            {
                continue;
            }

            keylen = strnlen(key, STR_LEN);

            if (do_comma)
            {
                ++result; // comma only
            }
            else
            {
                do_comma = true;
            }

            // key, two quotes, a colon, NO SPACE, and value
            result += keylen + 3 + _compute_size(value, out, indent, space, line_length);
        }
    }

    return result;
}

static size_t _compute_size(const Array& array, Writer& out, unsigned int& indent,
                            unsigned int space, unsigned int line_length)
{
    // TODO: Implement line_length restrictions so numbers, strings, bools, and nulls can stay on
    // the same line
    size_t result = 2; // at least '[' and ']'

    if (array.get_size() == 0)
    {
        return result;
    }

    if (space == 0) // Newlines only
    {
        ++result; // newline after '['

        bool do_comma = false;

        for (auto& value : array)
        {
            // Ignore invalid values
            if (!value.is_valid())
            {
                continue;
            }

            if (do_comma)
            {
                result += 2; // comma and newline
            }
            else
            {
                do_comma = true;
            }

            // value
            result += _compute_size(value, out, indent, space, line_length);
        }

        ++result; // newline for final ']'
    }
    else if (space != -1) // Newlines and indentation
    {
        ++indent;
        const size_t num_spaces = indent * static_cast<size_t>(space);

        result += 1 + num_spaces; // newline and indentation

        bool do_comma = false;

        for (auto& value : array)
        {
            // Ignore invalid values
            if (!value.is_valid())
            {
                continue;
            }

            if (do_comma)
            {
                result += 2 + num_spaces; // comma, newline, indentation
            }
            else
            {
                do_comma = true;
            }

            // value
            result += _compute_size(value, out, indent, space, line_length);
        }

        result += 1 + (num_spaces - space); // Newline and indentation for final ']'
        --indent;
    }
    else // No whitespace
    {
        bool do_comma = false;

        for (auto& value : array)
        {
            // Ignore invalid values
            if (!value.is_valid())
            {
                continue;
            }

            if (do_comma)
            {
                ++result; // comma only
            }
            else
            {
                do_comma = true;
            }

            // value
            result += _compute_size(value, out, indent, space, line_length);
        }
    }

    return result;
}

Writer::Writer(size_t size)
{
    this->size = size;
    buffer = new char[this->size];
}

Writer::~Writer()
{
    delete[] buffer;
}

void Writer::write_char(char chr)
{
    if (index == size)
    {
        resize(size + 1);
    }

    buffer[index] = chr;
    ++index;
}

void Writer::write_char_n(char chr, size_t n)
{
    if (n == 0)
    {
        return;
    }

    if (index + n > size)
    {
        resize(size + n);
    }

    memset(buffer + index, chr, n);
    index += n;
}

void Writer::write_text(const char* text, size_t count)
{
    if (index + count > size)
    {
        resize(size + count);
    }

    memcpy(buffer + index, text, count);
    index += count;
}

void Writer::clear()
{
    index = 0;
    memset(buffer, 0, size);
}

char* Writer::take_buffer()
{
    char* result = buffer;
    buffer = nullptr;
    return result;
}

size_t Writer::trim()
{
    if (size != index + 1)
    {
        // Must resize to index + 1
        resize(index + 1);
    }

    return index;
}

void Writer::resize(size_t new_size)
{
    char* old_buffer = buffer;
    buffer = new char[new_size];

    if (size < new_size)
    {
        memcpy(buffer, old_buffer, size);
        memset(buffer + size, 0, new_size - size);
    }
    else
    {
        memcpy(buffer, old_buffer, new_size);
    }

    delete[] old_buffer;
    size = new_size;
}

Exception::Exception(const char* message, size_t line, size_t column) : _line(line), _column(column)
{
    size_t length = strnlen(message, STR_LEN);
    this->message = new char[length + 1];
    strncpy_s(this->message, length + 1, message, length);
}

Exception::~Exception()
{
    delete[] message;
    message = nullptr;
}

size_t Exception::line() const noexcept
{
    return _line;
}

size_t Exception::column() const noexcept
{
    return _column;
}

const char* Exception::what() const noexcept
{
    return message;
}

StringBuilder::StringBuilder(size_t capacity)
{
    resize(capacity);
}

StringBuilder::~StringBuilder()
{
    delete[] buffer;
}

void StringBuilder::append(const char c)
{
    resize(length + 1);
    buffer[length] = c;
    ++length;
}

void StringBuilder::append(const char* text)
{
    size_t text_length = strnlen(text, STR_LEN);
    if (text_length == 0)
    {
        return;
    }

    resize(length + text_length);
    memcpy(buffer + length, text, text_length);
    length += text_length;
}

void StringBuilder::append(size_t uint)
{
    // Simple but eh... feels primitive
    // Reserve 20 characters to work with
    static size_t max = 20;
    size_t pos = max;
    resize(length + pos);

    do
    {
        --pos;
        // is there not a way to divide and get the result and the remainder together?
        // maybe this is optimized to something I don't know about?
        char c = '0' + (uint % 10);
        buffer[length + pos] = c;
        uint /= 10;

        if (pos == 0)
        {
            break;
        }
    }
    while (uint > 0);

    size_t offset = pos;
    if (offset == 0)
    {
        length += max;
        return;
    }

    do
    {
        buffer[length] = buffer[length + offset];
        ++length;
        ++pos;
    }
    while (pos < max);
}

char* StringBuilder::get_string()
{
    resize(length + 1);
    buffer[length] = 0;
    return buffer + start;
}

void StringBuilder::clear()
{
    length = 0;
}

void StringBuilder::push()
{
    append('\0');
    start = length;
}

void StringBuilder::pop()
{
    if (start == 0)
    {
        return;
    }

    length = start - 1;

    // move start back to beginning or after first null
    do
    {
        --start;
    }
    while (start > 0 && buffer[start - 1] != 0);
}

inline void StringBuilder::resize(size_t new_size)
{
    if (!(capacity < new_size))
    {
        return;
    }

    char* old = buffer;
    size_t old_cap = capacity;
    capacity = capacity * 2;
    if (capacity < new_size)
    {
        capacity = new_size;
    }

    if (capacity == 0)
    {
        buffer = nullptr;
        if (old != nullptr)
        {
            delete[] old;
        }
        return;
    }

    buffer = new char[capacity];
    if (old != nullptr)
    {
        memcpy(buffer, old, old_cap);
#ifdef DEBUG
        memset(buffer + old_cap, 0, capacity - old_cap);
#endif
        delete[] old;
    }
#ifdef DEBUG
    else
    {
        memset(buffer, 0, capacity);
    }
#endif
}

} // namespace Surface::JSON
