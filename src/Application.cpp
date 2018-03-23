#include <cstdlib>
#include "Application.hpp"

Application::Application(std::string && name) :
    m_name(name),
    m_encode(name.find("encode") != std::string::npos)
{
}

int
Application::run(std::vector<std::string> const & filenames) const
{
    return EXIT_SUCCESS;
}
