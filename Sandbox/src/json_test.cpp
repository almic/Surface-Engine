#include "json_test.h"

#include "json/json.h"
#include <filesystem>
#include <fstream>
#include <future> // this_thread

namespace JSON = Surface::JSON;
using Surface::JSON::json;

namespace Sandbox
{

void run_tests(Surface::Console& console)
{
    // Random testing
    /*
    auto result = json::parse("-0");
    if (!result)
    {
        console.writeln(result.what());
    }
    else
    {
        auto number = result.get().to_number();
        console.writeln(json::to_string(number).string());
    }

    json var = "hello world!";
    console.writeln(json::to_string(var).string());

    var = nullptr;
    console.writeln(json::to_string(var).string());

    var = json::object();
    var["test"] = 42;
    var["hello"] = json::object();
    var["hello"]["world"] = nullptr;

    var["array"] = json::array();
    var["array"][0] = 69;
    var["array"][1] = 420;

    auto& arr = var["array"];
    arr[2] = json::object();
    arr[2]["deep"] = true;

    auto string = json::to_string(var);
    console.writeln(string.string());

    console.writeln(JSON::is_valid(string.string()) ? "valid!" : "not valid!!!");
    */

    static const std::filesystem::path test_dir("test/json");

    for (auto const& entry : std::filesystem::directory_iterator(test_dir))
    {
        // Uncomment to test single filename
        /*
        if (!JSON::Utility::str_equal("y_string_nonCharacterInUTF-8_U+10FFFF.json",
                                      entry.path().filename().generic_string().c_str()))
        {
            continue;
        }
        */

        // Wait for console buffer to flush
        uint16_t max = -1;
        while (max)
        {
            --max;
            if (console.is_buffered())
            {
                console.flush();
                std::this_thread::sleep_for(std::chrono::microseconds(500));
                continue;
            }
            break;
        }

        if (!entry.is_regular_file())
        {
            continue;
        }

        auto& path = entry.path();
        if (!path.has_extension() || path.extension() != ".json")
        {
            continue;
        }

        // Test that the filename follows the "i_", "n_", "y_" format
        auto filename_str = path.filename().generic_string();
        auto filename = filename_str.c_str();
        size_t length = strnlen(filename, 2);
        if (length < 2)
        {
            continue;
        }

        const char type = filename[0];
        if (filename[1] != '_' || (type != 'i' && type != 'n' && type != 'y'))
        {
            continue;
        }

        // .json extension, starts with proper format
        auto file = std::ifstream(path, std::ios::in | std::ios::binary | std::ios::ate);
        if (!file)
        {
            console.write("Failed to open file \"");
            console.write(filename);
            console.writeln("\"!");
            continue;
        }

        auto end = file.tellg();
        file.seekg(0);

        // this is probably the right way to do this
        std::unique_ptr<char[]> json_text = std::make_unique<char[]>((size_t) end + 1);
        file.read(json_text.get(), end);
        file.close();
        json_text[end] = 0;

        auto result = json::parse(json_text.get());
        if (result)
        {
            if (type == 'y')
            {
                console.write("OK: ");
                console.writeln(filename);
                continue;
            }

            if (type == 'i')
            {
                console.write("OK/i: ");
                console.writeln(filename);
                continue;
            }

            console.writeln("\n---");
            console.write("FAIL: ");
            console.writeln(filename);
            console.writeln("    Should have rejected but input was accepted.");
            console.writeln("");
            auto as_string = json::to_string(result.get());
            console.writeln(as_string.string());
            console.writeln("---\n");

            continue;
        }

        if (type == 'n')
        {
            console.write("OK: ");
            console.writeln(filename);
            continue;
        }

        if (type == 'i') // I would like my parser to accept most ambiguous input
        {
            console.write("OK/i: ");
            console.writeln(filename);
            continue;
        }

        console.writeln("\n---");
        console.write("FAIL: ");
        console.writeln(filename);
        console.writeln("    Failed to parse input.");
        console.writeln("    ");
        console.writeln(result.what());

        JSON::Utility::string_builder msg;
        msg.append("Line: ");
        msg.append(json::to_string(json(result.line())).string());
        msg.append(", Column: ");
        msg.append(json::to_string(json(result.column())).string());

        console.writeln(msg.build().string());
        console.writeln("---\n");
    }

    console.writeln("\nDone with tests!");
}

} // namespace Sandbox
