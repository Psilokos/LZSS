#include <algorithm>
#include <exception>
#include "SlidingWindow.hpp"

#include <iostream>

namespace lzss
{
    namespace encoder
    {
        /*
         *  public methods
         */

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
         *  private static variables
         */

        SlidingWindow const * SlidingWindow::m_sliWinPtr = nullptr;

        /*
         *  private methods
         */

        void
        SlidingWindow::feed(void)
        {
            auto p_storage = reinterpret_cast<decltype(m_ifs)::char_type *>(const_cast<std::uint8_t *>(m_viewBuf.data() + m_viewBuf.size()));
            auto size = m_bufCap - m_searchBuf.size() - m_viewBuf.size();

            if (!m_ifs.read(p_storage, size))
            {
                if (!m_ifs.eof())
                    throw std::exception();
                else
                    m_bufCap -= size - m_ifs.gcount();
            }

            std::size_t maxBufSize = m_viewBuf.size() + m_ifs.gcount();
            m_viewBuf = Buffer(m_viewBuf.data(), maxBufSize < m_viewBufCap ? maxBufSize : m_viewBufCap);
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
         *  private nested classes
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

    namespace decoder
    {
        /*
         *  public methods
         */

        bool
        SlidingWindow::canSlide(std::uint16_t distance) const
        {
            return m_winBuf.data() + m_winBuf.size() + distance <= m_buf.get() + m_bufCap;
        }

        void
        SlidingWindow::reset(std::uint16_t size)
        {
            m_winBufCap = size;
            m_bufCap = 2 * size;
            m_buf = std::make_unique<std::uint8_t[]>(m_bufCap);
            m_winBuf = Buffer(m_buf.get(), 0);
        }

        template<>
        void
        SlidingWindow::feed<uint8_t>(uint8_t const & byte)
        {
            std::cout << byte << std::flush;
            const_cast<uint8_t *>(m_winBuf.data())[m_winBuf.size()] = byte;
            this->slide(1);
        }

        template<>
        void
        SlidingWindow::feed<SlidingWindow::MatchInfo>(SlidingWindow::MatchInfo const & match)
        {
            Buffer::iterator const first = m_winBuf.end() - 1 - match.index();
            Buffer::iterator last;
            if (match.index() + 1 < match.length())
            {
                last = m_winBuf.end();
                int i = match.index() + 1;
                while (i < match.length())
                {
                    std::copy(first, last, const_cast<uint8_t *>(m_winBuf.end()));
                    this->slide(match.index() + 1);
                    i += match.index() + 1;
                }
                std::copy(first, last - i % match.length(), const_cast<uint8_t *>(m_winBuf.end()));
            }
            else
            {
                last = first + match.length();
                std::copy(first, last, const_cast<uint8_t *>(m_winBuf.end()));
                std::cout << std::string(first, last) << std::flush;
            }
            this->slide(match.length());
        }

        void
        SlidingWindow::digestTo(std::ofstream & ofs, bool all)
        {
            std::streamsize const count = (all ? m_winBuf.end() : m_winBuf.begin()) - m_buf.get();
            ofs.write(reinterpret_cast<char *>(m_buf.get()), count);

            auto const first = m_winBuf.begin();
            auto const d_first = m_buf.get();
            std::move(first, first + m_winBuf.size(), d_first);
            m_winBuf = Buffer(d_first, m_winBuf.size());
        }

        /*
         *  private methods
         */

        void
        SlidingWindow::slide(std::uint16_t amount)
        {
            if (m_winBuf.size() + amount <= m_winBufCap)
                m_winBuf = Buffer(m_winBuf.data(), m_winBuf.size() + amount);
            else
            {
                std::uint16_t const unused = m_winBufCap - m_winBuf.size();
                std::uint16_t const shift = unused < amount ? unused : amount;
                m_winBuf = Buffer(m_winBuf.data() + shift, m_winBufCap);
            }
        }
    }
}
