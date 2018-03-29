#include <cstdlib>
#include <lzss_codec.hpp>
#include <memory>
#include "Application.hpp"

Application::Application(std::string && name) :
    m_name(name),
    m_encode(name.find("encode") != std::string::npos)
{
}

int
Application::run(std::vector<std::string> const & filenames) const
{
    std::unique_ptr<lzss::Codec> codec(m_encode
        ? static_cast<lzss::Codec *>(new lzss::Encoder())
        : static_cast<lzss::Codec *>(new lzss::Decoder()));

    for (auto const & filename : filenames)
        *codec << filename;
    codec->run();

    return EXIT_SUCCESS;
}
