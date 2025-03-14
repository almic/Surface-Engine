#pragma once

namespace Surface::JSON
{

// JSON types
enum class Type : unsigned char
{
    Invalid = 0, // Signal used when moving values as opposed to proliferating nulls
    Object,
    Array,
    String,
    Number,
    Bool,
    Null,
};

} // namespace Surface::JSON
