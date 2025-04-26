#pragma once

#include "error_codes.h"

// Generic error reporting across APIs

namespace Surface::Graphics
{

struct Error
{
  public:
    inline const char* get_message() const
    {
        return m_message;
    }

    inline int get_code() const
    {
        return m_code;
    }

    inline bool is_error() const
    {
        return m_code > 0;
    }

    static inline Error none()
    {
        return Error();
    }

    static inline Error create(const char* message, ERROR_CODE code)
    {
        return Error(message, code);
    }

    ~Error()
    {
        delete[] m_message;
        m_message = nullptr;
        m_code = Error_None;
    }

  public: // Operators
    inline operator bool() const
    {
        return is_error();
    }

    Error& operator=(Error&& other) noexcept
    {
        delete[] m_message;

        m_code = other.m_code;
        m_message = other.m_message;

        other.m_code = Error_None;
        other.m_message = nullptr;

        return *this;
    }

    Error& operator=(const Error& other)
    {
        delete[] m_message;
        m_message = nullptr;

        m_code = other.m_code;

        if (other.m_message)
        {
            copy_cstr(other.m_message, &m_message);
        }
    }

  public: // Move/ Copy constructors
    Error(Error&& other) noexcept
    {
        m_code = other.m_code;
        m_message = other.m_message;

        other.m_code = Error_None;
        other.m_message = nullptr;
    }

    Error(const Error& other)
    {
        m_code = other.m_code;

        if (other.m_message)
        {
            copy_cstr(other.m_message, &m_message);
        }
        else
        {
            m_message = nullptr;
        }
    }

  private:
    Error() : m_message(nullptr), m_code(Error_None)
    {
    }

    Error(const char* message, ERROR_CODE code) : m_code(code)
    {
        copy_cstr(message, &m_message);
    }

    static void copy_cstr(const char* source, char** dest)
    {
        size_t length = 0;
        for (/* empty */; length < static_cast<size_t>(-2); ++length)
        {
            if (source[length] == '\0')
            {
                break;
            }
        }

        char* copy = new char[length + 1];
        for (size_t index = 0; index < length; ++index)
        {
            copy[index] = source[index];
        }
        copy[length] = '\0';

        *dest = copy;
    }

    char* m_message;
    ERROR_CODE m_code;
};

} // namespace Surface::Graphics
