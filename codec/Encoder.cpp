#include <exception>
#include "Encoder.hpp"

namespace lzss
{
    /*
     * private methods
     */

    template<> void Encoder::writeToBuffer<bool>(bool flag);

    void
    Encoder::codeFilePriv(std::ifstream & ifs, std::ofstream & ofs)
    {
        m_ofs = std::move(ofs);
        m_sliWin.reset(std::move(ifs));
        m_encBuf = std::vector<std::uint8_t>(1);
        m_nBits = 0;

        while (!m_sliWin.isViewEmpty())
        {
            std::any const encodedView = this->encodeView();

            if (encodedView.type() == typeid(std::uint8_t))
            {
                this->writeToBuffer(true);
                this->writeToBuffer(std::any_cast<std::uint8_t>(encodedView));
                m_sliWin.consume(1);
            }
            else if (encodedView.type() == typeid(std::pair<std::uint16_t, std::uint16_t>))
            {
                auto const pair = std::any_cast<std::pair<std::uint16_t, std::uint16_t>>(encodedView);
                this->writeToBuffer(false);
                this->writeToBuffer(pair.first | (std::uint32_t)pair.second << 16);
                m_sliWin.consume(pair.second);
            }
            else
                throw std::exception();

            if (m_nBits == 8)
            {
                this->writeChunkToFile();
                m_encBuf.push_back(0);
                m_nBits = 0;
            }
        }
        this->writeChunkToFile(true);
    }

    std::any
    Encoder::encodeView(void)
    {
        SlidingWindow::Buffer const & searchBuf = m_sliWin.searchBuffer();
        SlidingWindow::Buffer viewBuf = m_sliWin.viewBuffer();

        while (viewBuf.size() >= 2 * sizeof(std::uint16_t))
        {
            SlidingWindow::Buffer::size_type pos = searchBuf.rfind(viewBuf);
            SlidingWindow::Buffer::size_type size = viewBuf.size();
            if (pos != SlidingWindow::Buffer::npos)
                return std::any(std::pair<std::uint16_t, std::uint16_t>(pos, size));

            viewBuf.remove_suffix(1);
        }
        return std::any(viewBuf.data()[0]);
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
        auto chunkEnd = chunkStart;

        std::advance(chunkEnd, size);
        m_ofs.write(reinterpret_cast<char *>(m_encBuf.data()), size);
        m_encBuf.erase(chunkStart, chunkEnd);
    }
}
