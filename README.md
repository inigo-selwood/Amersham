# Amersham

A header-only library for parsing command-line arguments.

## Overview

Amersham neatly abstracts the parsing of command-line arguments, providing verbose and helpful error messages for the user.

There are two types of arguments: flags, and keywords. The inputs to the program are termed keywords, while optional modifiers for the program's behaviour are termed flags. Flags have two forms, either tokens or codes. Tokens are strings, and are prefixed with a double hyphen, while codes are prefixed with a single hyphen.

For example, a program might have the following usage:

`program input_file [(--output | -o) <output_file_name>]`

For which the following invocations would be valid:

```
> ./program input.txt
> ./program input.txt --output output.txt
> ./program input.txt -o output.txt
```

In the above example, `input_file` is a keyword, while `--output` or `-o` is a flag, taking a parameter `output_file_name`. In both cases, the value `output.txt` is provided.

## Constraints

- Keywords and flag parameters cannot begin with a hyphen.
- Multiple flag codes can be concatenated into a string following a single hyphen, but multiple token flags must appear separately.
- Flag tokens can contain any characters, but flag codes must be uppercase or lowercase letters.
- Flags can take a single optional parameter, which should appear directly after it. In the case of a concatenated string of flag codes, their parameters (if any) should appear in order afterwards.

For example, a program might have the following usage:

`program [(--output | -o) <output_file_name>] [--verbose | -v] [(--profiled | -p) <time_base>]`

For which the two following invocations are all equivalent:

```
./program --output output.txt -v --profiled milliseconds
./program -o output.txt -v -p milliseconds
./program -ovp output.txt milliseconds
```

## Usage

This example program has the following usage:

`test input_file_name [(--output | -o) <output_file_name>]`

``` c++
#include <iostream>

#include "amersham.hpp"
using namespace Amersham;

int main(int argument_count, char *argument_vector[]) {

    // Place arguments in vector
    std::vector<std::string> arguments(argument_count - 1);
    for(unsigned int index = 1; index < argument_count; index += 1)
        arguments[index - 1] = argument_vector[index];

    // Create parser
    Argument::Parser parser;

    // Set expected range of no. keywords
    parser.minimum_keyword_count = 1;
    parser.maximum_keyword_count = 1;

    // Add flag '--output' or '-o', which takes a parameter
    const Argument::Parser::Flag output_flag("output", 'o', true);
    const Argument::Parser::FlagHandle output_flag_handle =
            parser.add_flag(output_flag);

    // Parse arguments vector, reporting errors
    Argument::Parser::Result result = parser.parse(arguments);
    if(result.error_messages.empty() == false) {
        for(const std::string &error_message : result.error_messages)
            std::cerr << error_message << "\n";
        return -1;
    }

    // Get input file keyword
    const std::string input_file = result.keywords[0];

    // Get output file flag
    std::string output_file = "default_output.txt";
    if(result.flags.find(output_flag_handle) != result.flags.end())
        output_file = result.flags[output_flag_handle];

    // Print input/output file names
    std::cout << "input file: " << input_file << "\n";
    std::cout << "output file: " << output_file << "\n";

    return 0;
}
```

Note that the function `Argument::Parser::add_flag` returns an `Argument::Parser::FlagHandle` object which is then used as an alias. It is used as a key in the result object's flag map.

### Exceptions

Exception               | Thrown by                     | Thrown when
------------------------|-------------------------------|---
`DuplicateFlagCode`     | `Argument::Parser::add_flag`  | Multiple flags added with the same code
`DuplicateFlagToken`    | `Argument::Parser::add_flag`  | Multiple flags added with the same token
`EmptyArgument`         | `Argument::Parser::parse`     | One of the strings in `arguments` is empty. This should never be the case unless the programmer has altered the arguments as passed to the program
`InvalidKeywordCount`   | `Argument::Parser::parse`     | `maximum_keyword_count` is less than `minimum_keyword_count`
