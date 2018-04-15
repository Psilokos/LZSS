#ifndef LZSS_INT_SLIDING_WINDOW_HPP_
# define LZSS_INT_SLIDING_WINDOW_HPP_

# include <cstdint>
# include <fstream>
# include <map>
# include <memory>
# include <optional>

namespace lzss
{
    namespace codec
    {
        class SlidingWindow
        {
        protected:
            typedef std::basic_string_view<uint8_t>    Buffer;

        public:
            struct MatchInfo : public std::array<uint16_t const, 2>
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

        public:
            virtual ~SlidingWindow(void) = default;
        };
    }

    namespace encoder
    {
        class SlidingWindow : private codec::SlidingWindow
        {
        public:
#ifdef DEBUG
            typedef codec::SlidingWindow::Buffer    Buffer;
#endif

        public:
            SlidingWindow(void);
            SlidingWindow(SlidingWindow const &) = delete;
            SlidingWindow(SlidingWindow &&) = delete;
            virtual ~SlidingWindow(void) = default;

            SlidingWindow & operator=(SlidingWindow const &) = delete;
            SlidingWindow & operator=(SlidingWindow &&) = delete;

#ifdef DEBUG
            inline Buffer const &
            searchBuffer(void) const
            {
                return m_searchBuf;
            }
#endif

            inline std::uint16_t
            size(void) const
            {
                return m_searchBufCap;
            }

            bool isViewEmpty(void) const;
            operator bool(void) const;

            virtual void reset(std::ifstream && ifs);

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

                    bool operator()(Buffer const & b1, Buffer const & b2) const;

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
        };
    };

    namespace decoder
    {
        class SlidingWindow : public codec::SlidingWindow
        {
        public:
            virtual ~SlidingWindow(void) = default;

            bool canSlide(std::uint16_t distance) const;

            void reset(uint16_t size);

            template<typename T>
            void feed(T const &);

            void digestTo(std::ofstream & ofs, bool all = false);

        private:
            void slide(uint16_t amount);

        private:
            std::unique_ptr<uint8_t[]>  m_buf;
            std::size_t                 m_bufCap;

            Buffer                      m_winBuf;
            std::uint16_t               m_winBufCap;
        };
    };
}

#endif
