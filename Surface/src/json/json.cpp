#include "json.h"

namespace Surface::JSON
{

ParseResult parse(const char* json)
{
    return ParseResult("TODO", 1, 1);
}

StringResult to_string(const Value& value)
{
    char str[] = "Hi";
    return StringResult(str);
}

bool Utility::str_equal(const char*& a, const char*& b)
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

char* Utility::str_copy(const char*& string)
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


} // namespace Surface::JSON
