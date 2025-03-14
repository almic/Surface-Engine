#pragma once

#include "array.h"
#include "object.h"
#include "value.h"

// This macro is STUPID and I did NOT ask for it
#pragma push_macro("NULL")
#undef NULL

/*

I want to, in the future, add a binary representation of this JSON. However, this implementation
must keep parity with what JSON allows.

So, this implementation must support a few things:

1. Reading a string into the json data type.
2. Writing a string from a json data type.
3. Modifying the json data type.

To stay minimal, the following will be explicitly unimplemented:

1. NO file reading or writing, only string encoding/ decoding
2. NO generic dynamic type, it just handles the values that json defines
3. NO type coersion. If we have a string, it must be retrieved as a string
4. NO integer type selection. We only store longs and doubles
5. NO advanced arrays. Just inserting, erasing, and iteration.
6. NO advanced dynamic maps. Just inserting, erasing, and iteration.
7. NO crazy operator overloading, just get and set and maybe a bracket operator for easier use.

*/

namespace Surface::JSON
{

/**
 * @brief The JSON `null` value.
 *
 * If you have the NULL macro: blame Windows, #undef NULL, type `0` instead, blame Windows again.
 */
extern const Value& NULL;

/**
 * @brief Parse a string into a JSON::Value. If parsing fails for any reason, JSON::NULL is returned
 * instead of throwing an exception. This makes parsing a little bit faster, especially if you don't
 * intend to display parsing errors to users.
 *
 * To get detailed parsing errors, use JSON::strict_parse().
 *
 * @param json json text to parse
 * @return a JSON::Value
 */
Value parse(const char* json) noexcept;

/**
 * @brief Parse a string into a JSON::Value. If parsing fails, an exception is thrown describing the
 * exact reason, including line and character values. Because this method must keep track of more
 * information to produce an exception, it is not recommended unless you intend to display the error
 * to a user.
 *
 * @param json json text to parse with exceptions
 * @return a JSON::Value
 */
Value strict_parse(const char* json);

/**
 * @brief Encode a JSON::Value into a string, optionally with spacing and array line length. When
 * spacing is enabled, newlines will be added for each key:value pair in objects, and each value in
 * arrays. You can set `array_line_length` to a positive value to let values in arrays run on the
 * same line up to a max length. Only strings, numbers, and bools in arrays can share lines.
 *
 * @param value value to encode to a JSON string
 * @param space 0 = newline per value, N = space indentation, -1 = no whitespace
 * @param array_line_length use a positive value to let array values share lines
 * @return a string
 */
char* to_string(const Value& value, unsigned int space = -1,
                unsigned int array_line_length = 0) noexcept;

} // namespace Surface::JSON

#pragma pop_macro("NULL")
