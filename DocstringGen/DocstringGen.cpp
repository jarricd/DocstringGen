#include <iostream>
#include <llama.h>
#include <string>
#include <vector>
#include "Tools.h"
#include "args.hxx"
#include "Generator.h"

int main(int argc, char **argv)
{
    args::ArgumentParser parser("This program will generate docstrings for your Python code.", "This program does not have supercow powers.");
    args::HelpFlag help(parser, "help", "Display this help menu", { 'h', "help" });
    args::Group args_group(parser, "");
    args::ValueFlag<std::string> file_flag(args_group, "target_path", "Target file to process", { 'f', "file" });
    args::ValueFlag<std::string> directory_flag(args_group, "target_directory", "Target directory to process", { 'd', "target_dir" });
    std::string script_result = "";
    tools::execute_python_file("test.py", argv, &script_result);


    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (args::Help)
    {
        std::cout << parser;
        return 0;
    }
    catch (args::ParseError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    catch (args::ValidationError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 2;
    }
    if (!file_flag)
    {
        std::cerr << "File not provided." << std::endl;
        return 3;
    }

    std::string generated_response = generator::generate_docstring(args::get(file_flag));
    if (generated_response.empty())
    {
        std::cerr << "No response has been generated, read previous messages." << std::endl;
    }
    return 0;
 }
