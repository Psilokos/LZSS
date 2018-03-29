#ifndef LZSS_INT_SLIDING_WINDOW_HPP_
# define LZSS_INT_SLIDING_WINDOW_HPP_

# include <cstdint>
# include <fstream>
# include <map>
# include <memory>
# include <optional>

namespace lzss
{
    class SlidingWindow
    {
#ifdef DEBUG
        public:
#endif
            typedef std::basic_string_view<std::uint8_t>    Buffer;

        public:
            struct MatchInfo;

        public:
            SlidingWindow(void);
            SlidingWindow(SlidingWindow const &) = delete;
            SlidingWindow(SlidingWindow &&) = delete;
            ~SlidingWindow(void) = default;

            SlidingWindow & operator=(SlidingWindow const &) = delete;
            SlidingWindow & operator=(SlidingWindow &&) = delete;

#ifdef DEBUG
            inline Buffer const &
            searchBuffer(void) const
            {
                return m_searchBuf;
            }
#endif

            bool isViewEmpty(void) const;
            operator bool(void) const;

            void reset(std::ifstream && ifs);

            void            consume(std::uint8_t count);
            std::uint8_t    extract(void);

            std::optional<MatchInfo> match(std::uint8_t minimumMatch);

        private:
            void feed(void);
            void updateSearchTree(std::uint8_t insertCnt);

        private:
            class BufferCompare
            {
                public:
                    BufferCompare(void);
                    BufferCompare(BufferCompare const &) = default;
                    BufferCompare(BufferCompare &&) = default;
                    ~BufferCompare(void) = default;

                    BufferCompare & operator=(BufferCompare const &) = default;
                    BufferCompare & operator=(BufferCompare &&) = default;

                    bool operator()(Buffer const & k1, Buffer const & k2) const;

                private:
                    SlidingWindow const *   m_sliWinPtr;
            };

        private:
            typedef std::map<Buffer, std::uint16_t, BufferCompare>    Haystack;

        private:
            std::ifstream                   m_ifs;

            std::unique_ptr<std::uint8_t[]> m_buf;
            std::uint32_t                   m_bufCap;

            std::uint32_t                   m_shift;

            Buffer                          m_searchBuf;
            std::uint16_t                   m_searchBufCap;
            bool                            m_searchBufFull;

            Buffer                          m_viewBuf;
            std::uint8_t                    m_viewBufCap;

            static SlidingWindow const *    m_sliWinPtr;
            std::uint8_t                    m_bufCmpCnt;
            Haystack                        m_haystack;

        public:
            struct MatchInfo : public std::array<std::uint16_t const, 2>
            {
                inline std::uint16_t const &
                index(void) const
                {
                    return (*this)[0];
                }

                inline std::uint16_t const &
                length(void) const
                {
                    return (*this)[1];
                }
            };
    };
}

#endif
