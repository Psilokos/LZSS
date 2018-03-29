#include <exception>
#include "SlidingWindow.hpp"

#include <cstring>

namespace lzss
{
    SlidingWindow::Buffer const &
    SlidingWindow::searchBuffer(void) const
    {
        return m_searchBuf;
    }

    SlidingWindow::Buffer const &
    SlidingWindow::viewBuffer(void) const
    {
        return m_viewBuf;
    }

    bool
    SlidingWindow::isViewEmpty(void) const
    {
        return m_viewBuf.empty();
    }

    void
    SlidingWindow::consume(std::uint16_t count)
    {
        m_shift += count; // jump manually by storing ptr to label
        if (m_searchBuf.size() + count <= m_searchBufCap)
        {
            m_searchBuf = Buffer(m_searchBuf.data(), m_searchBuf.size() + count);
            m_viewBuf = Buffer(m_viewBuf.data() + count, m_viewBuf.size());
        }
        else
        {
            std::uint16_t const shift = m_searchBuf.size() + count - m_searchBufCap;

            if (!m_ifs.eof() && m_shift > m_bufCap - m_viewBufCap)
            {
                std::memmove(m_buf.get(),
                             m_searchBuf.data() + shift,
                             m_buf.get() + m_bufCap - (m_searchBuf.data() + shift));    // TODO: check Buffer::copy for memory overlap support

                m_searchBuf = Buffer(m_buf.get(), m_searchBufCap);
                m_viewBuf = Buffer(m_buf.get() + m_searchBufCap, m_viewBuf.size() - (m_shift - (m_bufCap - m_viewBufCap)));
                m_shift = m_searchBufCap;
                if (!m_ifs.eof())
                    return this->feed();
            }
            else
            {
                m_searchBuf = Buffer(m_searchBuf.data() + shift, m_searchBufCap);

                std::size_t const shiftMax = m_bufCap - m_viewBuf.size();
                std::size_t const shrink = m_shift > shiftMax ? m_shift - shiftMax : 0;
                m_viewBuf = Buffer(m_viewBuf.data() + count, m_viewBuf.size() - shrink);
            }
        }
    }

    void
    SlidingWindow::reset(std::ifstream && ifs)
    {
        m_ifs = std::move(ifs);

// TODO: automatic detection algorithm depending on ifs_size (seek to eof, std::basic_fs::tellg)
        std::uint8_t nShiftArea = 1; // if 0 then fuck (no view)
        m_searchBufCap = 0x1000;
        m_viewBufCap = 0x2A;
        m_bufCap = (nShiftArea + 1) * m_searchBufCap;

        m_buf = std::make_unique<std::uint8_t[]>(m_bufCap);
        m_searchBuf = Buffer(m_buf.get(), 0);
        m_viewBuf = Buffer(m_buf.get(), 0);
        m_shift = 0;

        this->feed();
    }

    /*
     * private methods
     */

    void
    SlidingWindow::feed(void)
    {
        auto p_storage = reinterpret_cast<decltype(m_ifs)::char_type *>(const_cast<std::uint8_t *>(m_viewBuf.data() + m_viewBuf.size()));
        auto size = m_bufCap - m_searchBuf.size() - m_viewBuf.size();
        if (!m_ifs.read(p_storage, size) && !m_ifs.eof())
            throw std::exception();

        std::size_t maxBufSize = m_viewBuf.size() + m_ifs.gcount();
        m_viewBuf = Buffer(m_viewBuf.data(), maxBufSize < m_viewBufCap ? maxBufSize : m_viewBufCap);
        if (m_ifs.eof())
            m_bufCap -= size - m_ifs.gcount();
    }
}
