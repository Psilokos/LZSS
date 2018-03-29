#ifndef LZSS_INT_SLIDING_WINDOW_HPP_
# define LZSS_INT_SLIDING_WINDOW_HPP_

# include <cstdint>
# include <fstream>
# include <memory>

namespace lzss
{
    class SlidingWindow
    {
        public:
            typedef std::basic_string_view<std::uint8_t>    Buffer;

        public:
            SlidingWindow(void) = default;
            SlidingWindow(SlidingWindow const &) = delete;
            SlidingWindow(SlidingWindow &&) = delete;
            ~SlidingWindow(void) = default;

            SlidingWindow & operator=(SlidingWindow const &) = delete;
            SlidingWindow & operator=(SlidingWindow &&) = delete;

            Buffer const & searchBuffer(void) const;
            Buffer const & viewBuffer(void) const;

            bool isViewEmpty(void) const;

            void consume(std::uint16_t count);
            void reset(std::ifstream && ifs);

        private:
            void feed(void);

        private:
            std::ifstream                   m_ifs;

            std::unique_ptr<std::uint8_t[]> m_buf;
            std::uint32_t                   m_bufCap;

            std::uint32_t                   m_shift;

            Buffer                          m_searchBuf;
            std::uint16_t                   m_searchBufCap;

            Buffer                          m_viewBuf;
            std::uint8_t                    m_viewBufCap;
    };
}

#endif
