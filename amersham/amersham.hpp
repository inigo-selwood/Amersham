#pragma once

#include <map>
#include <string>
#include <vector>

namespace Amersham {

namespace Argument {

class Parser {

public:

    typedef unsigned int FlagHandle;

    struct Result {

    public:

        std::vector<std::string> error_messages;

        std::vector<std::string> keywords;

        std::map<FlagHandle, std::string> flags;

    };

    struct Flag {

    public:

        std::string token;

        char code;

        bool takes_parameter;

        Flag(const std::string &token,
                const char &code,
                const bool takes_parameter);

    };

    struct DuplicateFlagCode {};
    struct DuplicateFlagToken {};
    struct EmptyArgument {};
    struct InvalidKeywordCount {};

    unsigned int minimum_keyword_count;
    unsigned int maximum_keyword_count;

    FlagHandle add_flag(const Flag &flag);

    Result parse(const std::vector<std::string> &arguments);

private:

    std::vector<Flag> flags;

};

Parser::Flag::Flag(const std::string &token,
        const char &code,
        const bool takes_parameter = false) {

    this->token = token;
    this->code = code;
    this->takes_parameter = takes_parameter;
}

Parser::FlagHandle Parser::add_flag(const Flag &flag) {
    for(const Flag &_flag : flags) {
        if(_flag.token == flag.token)
            throw DuplicateFlagToken();
        else if(_flag.code == flag.code)
            throw DuplicateFlagCode();
    }

    flags.push_back(flag);
    return flags.size() - 1;
}

Parser::Result Parser::parse(const std::vector<std::string> &arguments) {
    Result result;

    if(maximum_keyword_count < minimum_keyword_count)
        throw InvalidKeywordCount();

    for(unsigned int index = 0; index < arguments.size(); index += 1) {
        const std::string &argument = arguments[index];
        const unsigned int length = argument.length();
        if(length == 0)
            throw EmptyArgument();

        // Handle flags
        if(argument[0] == '-') {

            if(length == 1 || (length == 2 && argument[1] == '-')) {
                result.error_messages.push_back("empty flag");
                continue;
            }

            // Handle code flags, ie: '-c', or '-xZyf'
            if(argument[1] != '-') {
                bool error_encountered = false;
                unsigned int parameter_offset = 0;
                std::vector<char> unrecognized_flags;

                for(unsigned int index = 1;
                            index < argument.length();
                            index += 1) {

                    const char &code = argument[index];
                    if((code < 'a' || code > 'z')
                            && (code < 'A' || code > 'Z')) {
                        std::string error_message = "invalid flag '-";
                        error_message += code;
                        error_message += "'";

                        result.error_messages.push_back(error_message);
                        error_encountered = true;
                        continue;
                    }

                    // Find corresponding flag
                    unsigned int flag_handle = 0;
                    bool flag_found = false;
                    bool takes_parameter = false;
                    for(flag_handle = 0;
                            flag_handle < flags.size();
                            flag_handle += 1) {

                        if(flags[flag_handle].code == code) {
                            flag_found = true;
                            takes_parameter =
                                    flags[flag_handle].takes_parameter;
                            break;
                        }
                    }

                    if(flag_found == false) {
                        std::string error_message = "unrecognized flag '-";
                        error_message += code;
                        error_message += "'";

                        result.error_messages.push_back(error_message);
                        error_encountered = true;
                        continue;
                    }

                    // Handle parameters
                    std::string parameter;
                    if(takes_parameter) {
                        if(index + parameter_offset + 1 >= arguments.size()) {
                            std::string error_message =
                                    "parameter expected for flag '-";
                            error_message += code;
                            error_message += "'";

                            result.error_messages.push_back(error_message);
                            continue;
                        }

                        parameter = arguments[index + parameter_offset + 1];

                        if(parameter.empty())
                            throw EmptyArgument();
                        else if(parameter[0] == '-') {
                            std::string error_message =
                                    "parameter expected for flag '-";
                            error_message += code;
                            error_message += "'";

                            result.error_messages.push_back(error_message);
                            continue;
                        }

                        parameter_offset += 1;
                    }

                    result.flags[flag_handle] = parameter;
                }

                index += parameter_offset;

                if(error_encountered) {
                    const std::string token =
                            argument.substr(1, std::string::npos);
                    for(const auto &flag : flags) {
                        if(flag.token == token) {
                            const std::string error_message =
                                    "did you mean '--" + token + "'?";
                            result.error_messages.push_back(error_message);
                            continue;
                        }
                    }
                }
            }

            // Handle token flags, ie: '--output', '--help'
            else {
                const std::string token =
                        argument.substr(2, std::string::npos);

                unsigned int flag_handle = 0;
                bool flag_found = false;
                bool takes_parameter = false;
                for(flag_handle = 0;
                        flag_handle < flags.size();
                        flag_handle += 1) {

                    if(flags[flag_handle].token == token) {
                        flag_found = true;
                        takes_parameter = flags[flag_handle].takes_parameter;
                        break;
                    }
                }

                if(flag_found == false) {
                    const std::string error_message =
                            "unrecognized flag '--" + token + "'";
                    result.error_messages.push_back(error_message);
                    continue;
                }

                // Handle parameters
                std::string parameter;
                if(takes_parameter) {
                    index += 1;

                    if(index == arguments.size()) {
                        const std::string error_message =
                                "flag '--" + token + "' expects a parameter";
                        result.error_messages.push_back(error_message);
                        continue;
                    }

                    parameter = arguments[index];

                    if(parameter.empty())
                        throw EmptyArgument();
                    else if(parameter[0] == '-') {
                        const std::string error_message =
                                "flag '--" + token + "' expects a parameter";
                        result.error_messages.push_back(error_message);
                        continue;
                    }
                }

                result.flags[flag_handle] = parameter;
            }
        }

        // Handle keywords
        else {
            if(result.keywords.size() == maximum_keyword_count) {
                const std::string error_message =
                        "unexpected keyword '" + argument + "'";
                result.error_messages.push_back(error_message);
                continue;
            }

            result.keywords.push_back(argument);
        }
    }

    const unsigned int keyword_count = result.keywords.size();
    if(keyword_count < minimum_keyword_count) {
        const std::string error_message =
                std::to_string(minimum_keyword_count) +
                " keyword(s) expected, but found " +
                std::to_string(keyword_count);
        result.error_messages.push_back(error_message);
    }

    return result;
}

};

};
