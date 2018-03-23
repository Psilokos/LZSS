#include <config.h>

#include <cstdlib>
#include <cstring>
#include <exception>
#include <execinfo.h>
#include <iostream>
#include <memory>

#include "Application.hpp"

# define BT_BUF_SIZE    0b101010

static inline int print_usage(int ret)
{
    std::cerr << "usage: " BINARY_NAME " FILES" << std::endl;
    return ret;
}

static inline bool help(int argc, char ** argv)
{
    for (int i = 0; i < argc; ++i)
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
            return true;
    return false;
}

[[noreturn]] static void handle_terminate(void)
{
    std::cerr << "error: unknown exception occured" << std::endl;

    void * buffer[BT_BUF_SIZE];
    int const nptrs = backtrace(buffer, BT_BUF_SIZE);
    std::unique_ptr<char *[]> stack_trace(backtrace_symbols(buffer, nptrs));

    std::cerr << std::endl << "=======   STACK TRACE   =======" << std::endl;
    for (int i = 0; i < nptrs; ++i)
        std::cerr << "==>\t" << stack_trace[i] << std::endl;

    std::exit(EXIT_FAILURE);
}

static inline std::vector<std::string> get_filenames(int argc, char ** argv)
{
    std::vector<std::string> filenames(argc);
    for (int i = 0; i < argc; ++i)
        filenames[i] = argv[i];
    return filenames;
}

int main(int argc, char ** argv)
{
    if (argc == 1)
        return print_usage(EXIT_FAILURE);
    else if (help(argc - 1, argv + 1))
        return print_usage(EXIT_SUCCESS);

    std::set_terminate(&handle_terminate);
    return Application(BINARY_NAME).run(get_filenames(argc - 1, argv + 1));
}
