#pragma once

namespace Surface::Graphics
{


enum ERROR_CODE : unsigned long
{
    // No error
    Error_None = 0,

    // Generic errors
    Error_Generic = 1,

    // Initialization errors
    Error_Init_Failed_Generic = 0x100,

    // Binding errors
    Error_Bind_Window = 0x200,
};

} // namespace Surface::Graphics
