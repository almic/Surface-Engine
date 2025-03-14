#include "json.h"

#include "array.h"
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

Value parse(const char* json) noexcept
{
    return NULL;
}

Value strict_parse(const char* json)
{
    return NULL;
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

} // namespace Surface::JSON
