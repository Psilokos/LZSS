#include <algorithm>
#include <exception>
#include "SlidingWindow.hpp"

namespace lzss
{
    SlidingWindow::SlidingWindow(void)
    {
        SlidingWindow::m_sliWinPtr = this;
        m_haystack = Haystack();
    }

    bool
    SlidingWindow::isViewEmpty(void) const
    {
        return m_viewBuf.empty();
    }

    SlidingWindow::operator bool(void) const
    {
        return !this->isViewEmpty();
    }

    void
    SlidingWindow::reset(std::ifstream && ifs)
    {
        m_ifs = std::move(ifs);

// TODO: automatic detection algorithm depending on ifs_size (seek to eof, std::basic_fs::tellg)
        std::uint16_t const nShiftArea = 1; // if 0 then fuck (no view)
        m_searchBufCap = 0x1000;
        m_viewBufCap = 0x2A;
        m_bufCap = (nShiftArea + 1) * m_searchBufCap;

        m_buf = std::make_unique<std::uint8_t[]>(m_bufCap);
        m_searchBuf = Buffer(m_buf.get(), 0);
        m_viewBuf = Buffer(m_buf.get(), 0);

        m_shift = 0;
        m_searchBufFull = false;

        m_haystack.clear();

        this->feed();
    }

    void
    SlidingWindow::consume(std::uint8_t count)
    {
        m_shift += count;
        std::size_t const shiftMax = m_bufCap - m_viewBuf.size();
        std::size_t const shrink = m_shift > shiftMax ? m_shift - shiftMax : 0;
        std::uint8_t shift;

        if (!m_ifs.eof() && shrink)
        {
            shift = m_searchBuf.size() + count - m_searchBufCap;
            Buffer::iterator first = m_searchBuf.begin() + shift;
            Buffer::iterator last = m_buf.get() + m_bufCap;
            auto d_first = m_buf.get();
            std::move(first, last, d_first);

            m_searchBuf = Buffer(d_first, m_searchBufCap);
            m_viewBuf = Buffer(d_first + m_searchBuf.size(), m_viewBuf.size() - shrink);
            m_shift = m_searchBufCap;
            if (!m_ifs.eof())
                this->feed();

            m_haystack.clear();
            m_bufCmpCnt = m_viewBuf.size();
            for (auto it = m_searchBuf.end() - 1; it >= m_searchBuf.begin(); --it)
                m_haystack.insert(std::make_pair(Buffer(it, m_viewBuf.size()), std::distance(it, m_searchBuf.end() - 1)));
        }
        else
        {
            if (!m_searchBufFull)
            {
                if (m_searchBuf.size() + count > m_searchBufCap)
                {
                    m_searchBufFull = true;
                    shift = m_searchBuf.size() + count - m_searchBufCap;
                    goto searchbuf_full;
                }

                m_searchBuf = Buffer(m_searchBuf.data(), m_searchBuf.size() + count);
                m_viewBuf = Buffer(m_viewBuf.data() + count, m_viewBuf.size() - shrink);
            }
            else
            {
                shift = count;
searchbuf_full:
                m_bufCmpCnt = m_viewBufCap;
                for (std::uint8_t i = 0; i < shift; ++i)
                    m_haystack.erase(m_searchBuf.substr(i, m_viewBufCap));

                m_searchBuf = Buffer(m_searchBuf.data() + shift, m_searchBufCap);
                m_viewBuf = Buffer(m_viewBuf.data() + count, m_viewBuf.size() - shrink);
            }
            this->updateSearchTree(count);
        }
    }

    std::uint8_t
    SlidingWindow::extract(void)
    {
        std::uint8_t const first = m_viewBuf.front();
        this->consume(1);
        return first;
    }

    std::optional<SlidingWindow::MatchInfo>
    SlidingWindow::match(std::uint8_t minLen)
    {
        Buffer needle = m_viewBuf;
        while (needle.size() >= minLen)
        {
            m_bufCmpCnt = needle.size();
            auto node = m_haystack.find(needle);
            if (node != m_haystack.end())
            {
                MatchInfo match = { node->second, static_cast<std::uint16_t>(needle.length()) };
                return std::make_optional(std::move(match));
            }
            needle.remove_suffix(1);
        }
        return std::nullopt;
    }

    /*
     * private static variables
     */

    SlidingWindow const * SlidingWindow::m_sliWinPtr = nullptr;

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

    void
    SlidingWindow::updateSearchTree(std::uint8_t insertCnt)
    {
        for (auto & node : m_haystack)
            node.second += insertCnt;
        m_bufCmpCnt = m_viewBuf.size();
        for (std::uint8_t i = 0; i < insertCnt; ++i)
            m_haystack.insert_or_assign(Buffer(m_searchBuf.data() + m_searchBuf.size() - 1 - i, m_viewBuf.size()), i);
    }

    /*
     * private nested classes
     */

    SlidingWindow::BufferCompare::BufferCompare(void) :
        m_sliWinPtr(SlidingWindow::m_sliWinPtr)
    {
    }

    bool
    SlidingWindow::BufferCompare::operator()(Buffer const & lhs, Buffer const & rhs) const
    {
        std::uint8_t const count = m_sliWinPtr->m_bufCmpCnt;
        return lhs.compare(0, count, rhs, 0, count) < 0;
    }
}
