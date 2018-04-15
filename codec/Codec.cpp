#include <exception>
#include "Codec.hpp"

namespace lzss
{
    bool
    Codec::addFile(std::string const & filename)
    {
        std::ifstream ifs(filename);
        if (!ifs.is_open())
            return false;
        m_fileQ.push(std::make_pair(std::move(ifs), std::ofstream(filename + ".lzss"))); // TODO: handle invalid output file
        return true;
    }

    Codec &
    Codec::operator<<(std::string const & filename)
    {
        if (!this->addFile(filename))
            throw std::exception();
        return *this;
    }

    bool
    Codec::hasFile(void) const
    {
        return !m_fileQ.empty();
    }

    void
    Codec::run(void)
    {
        while (this->hasFile())
            this->codeFile();
    }

    void
    Codec::operator()(void)
    {
        this->run();
    }

    void
    Codec::codeFile(void)
    {
        FileQueue::value_type files(std::move(m_fileQ.front()));
        m_fileQ.pop();
        this->codeFile(std::move(files.first), std::move(files.second));
    }
}
