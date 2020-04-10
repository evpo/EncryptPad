#include <iostream>
#include <sstream>
#include <string>
#include "version.h"
#include "cli_parser.hpp"
#include "logger_init.h"
#include "plog/Log.h"
#include "commands.h"

using namespace std;
using namespace stlplus;
using namespace EncryptMsg::Cli;
using namespace EncryptMsg;

void PrintUsage()
{
    const char *usage =
        VER_PRODUCTNAME_STR " " VER_PRODUCTVERSION_STR "\n"
        "\n"
        "Usage: encrypt-msg --help | -e|-d <file-name>\n";

    std::cout << usage << std::endl;
}

enum class Action
{
    None,
    Encrypt,
    Decrypt
};

int main(int, char *argv[])
{
    cli_definitions_t cli_defs = {
        {
            "help",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "help",
            ""
        },
        {
            "log",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "log",
            ""
        },
        {
            "e",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "encrypt",
            ""
        },
        {
            "d",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "decrypt",
            ""
        },
        {
            "pwd-file",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "pwd-file",
            ""
        },
        {
            "armor",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "armor",
            ""
        },
        {
            "o",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "output",
            ""
        },
        {
            "",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "file-name",
            ""
        },
        END_CLI_DEFINITIONS,
    };

    message_handler messages(std::cerr);
    cli_parser parser(&cli_defs[0], messages);
    if(!parser.parse(argv))
    {
        PrintUsage();
        exit(1);
    }

    string log_file;
    string file_name;
    string pwd_file;
    string output;
    bool armor = false;
    Action action = Action::None;

    for(unsigned i = 0; i < parser.size(); i++)
    {
        if(parser.name(i) == "help")
        {
            PrintUsage();
            exit(0);
        }
        else if(parser.name(i) == "log")
        {
            log_file = parser.string_value(i);
        }
        else if(parser.name(i) == "pwd-file")
        {
            pwd_file = parser.string_value(i);
        }
        else if(parser.name(i) == "armor")
        {
            armor = true;
        }
        else if(parser.name(i) == "o")
        {
            output = parser.string_value(i);
        }
        else if(parser.name(i) == "e")
        {
            action = Action::Encrypt;
        }
        else if(parser.name(i) == "d")
        {
            action = Action::Decrypt;
        }
        else if(parser.name(i) == "")
        {
            file_name = parser.string_value(i);
        }
    }

    if(log_file.empty())
    {
        log_file = "debug.log";
    }

    InitLogger(log_file);

    LOG_DEBUG << "Action: " << static_cast<int>(action);
    LOG_DEBUG << file_name << " " << output << " " << pwd_file;
    if(file_name.empty() || output.empty() || pwd_file.empty())
    {
        PrintUsage();
        return -1;
    }

    if(action == Action::Decrypt)
    {
        Decrypt(file_name, output, pwd_file);
    }
    else if(action == Action::Encrypt)
    {
        Encrypt(file_name, output, pwd_file, armor);
    }

    return 0;
}
