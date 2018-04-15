#include "Decoder.hpp"

#include <iostream>

namespace lzss
{
    /*
     *  private methods
     */

    void
    Decoder::codeFile(std::ifstream && ifs, std::ofstream && ofs)
    {
        this->setup(std::move(ifs), std::move(ofs));

        while (0b101010)
        {
            uint8_t unread = 0;

            while (m_idx + 1 < m_encBufCap)
            {
                uint8_t const bitFlag = (m_encBuf[m_idx] >> m_nBits++) & 1;
                if (m_nBits == 8)
                {
                    m_nBits = 0;
                    ++m_idx;
                }

                if (bitFlag == 1)
                {
                    uint32_t const rawMatch = this->extract<uint32_t>();
                    SlidingWindow::MatchInfo const match =
                    {
                        static_cast<uint16_t>(rawMatch),
                        static_cast<uint16_t>(rawMatch >> sizeof(uint16_t) * 8)
                    };

                    if (!m_sliWin.canSlide(match.length()))
                        m_sliWin.digestTo(m_ofs);
                    m_sliWin.feed(match);
                }
                else
                {
                    if (!m_sliWin.canSlide(1))
                        m_sliWin.digestTo(m_ofs);
                    m_sliWin.feed(this->extract<uint8_t>());
                }
            }

            if (!m_ifs.eof())
                this->feed(unread);
            else
                break;
        }
        m_sliWin.digestTo(m_ofs, true);
    }

    void
    Decoder::setup(std::ifstream && ifs, std::ofstream && ofs)
    {
        m_ifs = std::move(ifs);
        m_ofs = std::move(ofs);

        uint16_t size;
        if (!m_ifs.read(reinterpret_cast<char *>(&size), sizeof(size)))
            throw std::exception();
        m_sliWin.reset(size);

        m_encBufCap = 0x10000;
        m_encBuf = std::make_unique<std::uint8_t[]>(m_encBufCap);

        this->feed(0);
    }

    template<typename T>
    T
    Decoder::extract(void)
    {
        T r = this->readEncBuf<T>(true);
        if (m_nBits)
        {
            r >>= m_nBits;
            r |= this->readEncBuf<T>(false) << (sizeof(T) * 8 - m_nBits);
        }
        return r;
    }

    template<typename T>
    T
    Decoder::readEncBuf(bool consume)
    {
        if (m_idx + sizeof(T) > m_encBufCap)
            this->feed(m_encBufCap - m_idx);

        T r = *reinterpret_cast<T *>(m_encBuf.get() + m_idx);
        if (consume)
            m_idx += sizeof(T);
        return r;
    }

    void
    Decoder::feed(uint8_t unread)
    {
        auto d_first = m_encBuf.get();
        auto last = d_first + m_encBufCap;
        auto first = last - unread;
        std::move(first, last, d_first);

        if (!m_ifs.read(reinterpret_cast<char *>(d_first + unread), m_encBufCap))
        {
            if (!m_ifs.eof())
                throw std::exception();
            else
                m_encBufCap = m_ifs.gcount();
        }
    }
}
