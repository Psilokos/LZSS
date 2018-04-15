#include <exception>
#include "Encoder.hpp"

namespace lzss
{
    /*
     * private methods
     */

    template<> void Encoder::writeToBuffer<bool>(bool flag);

    void
    Encoder::codeFile(std::ifstream && ifs, std::ofstream && ofs)
    {
#ifdef DEBUG
        std::ofstream decoded("decoded");
#endif
        m_ofs = std::move(ofs);
        m_sliWin.reset(std::move(ifs));
        m_encBuf = std::vector<std::uint8_t>(1);
        m_nBits = 0;

        auto sliWinSize = m_sliWin.size();
        m_ofs.write(reinterpret_cast<char *>(&sliWinSize), sizeof(sliWinSize));
        while (m_sliWin)
        {
            auto const match = m_sliWin.match(2 * sizeof(std::uint16_t));

            this->writeToBuffer(match.has_value());
            if (match)
            {
                this->writeToBuffer(match->index() | static_cast<std::uint32_t>(match->length()) << 16);
#ifdef DEBUG
                auto p_addr = m_sliWin.searchBuffer().data() + m_sliWin.searchBuffer().size() - 1 - match->index();
                decoded << std::string_view(reinterpret_cast<char const *>(p_addr), match->length());
#endif
                m_sliWin.consume(match->length());
            }
            else
            {
                auto const unmatched = m_sliWin.extract();
#ifdef DEBUG
                decoded << unmatched;
#endif
                this->writeToBuffer(unmatched);
            }

            if (m_nBits == 8)
            {
                this->writeChunkToFile();
                m_encBuf.push_back(0);
                m_nBits = 0;
            }
        }
        this->writeChunkToFile(true);
    }

    template<typename T>
    void
    Encoder::writeToBuffer(T code)
    {
        m_encBuf.resize(m_encBuf.size() + sizeof(T));
        T * buf = (T *)(m_encBuf.data() + m_encBuf.size() - sizeof(T) - 1);
        if (m_nBits == 0)
            *buf = code;
        else
        {
            *buf++ |= code << m_nBits;
            *reinterpret_cast<std::uint8_t *>(buf) = code >> (sizeof(T) * 8 - m_nBits);
        }
    }

    template<>
    void
    Encoder::writeToBuffer<bool>(bool flag)
    {
        if (m_nBits & 8)
        {
            m_encBuf.resize(m_encBuf.size() + 1);
            m_nBits = 0;
        }
        if (flag)
            m_encBuf.back() |= 1 << m_nBits;
        ++m_nBits;
    }

    void
    Encoder::writeChunkToFile(bool force)
    {
        if (m_encBuf.size() < 0x1000 && !force)
            return;

        std::streamsize const size = force ? m_encBuf.size() : 0x1000;
        auto chunkStart = m_encBuf.begin();
        auto chunkEnd = chunkStart + size;

        m_ofs.write(reinterpret_cast<char *>(m_encBuf.data()), size);
        m_encBuf.erase(chunkStart, chunkEnd);
    }
}
