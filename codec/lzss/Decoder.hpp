#ifndef LZSS_INT_DECODER_HPP_
# define LZSS_INT_DECODER_HPP_

# include "Codec.hpp"
# include "SlidingWindow.hpp"

namespace lzss
{
    namespace decoder
    {
        class Decoder : public Codec
        {
        public:
            Decoder(void) = default;
            Decoder(Decoder const &) = delete;
            Decoder(Decoder &&) = delete;
            virtual ~Decoder(void) = default;

            Decoder & operator=(Decoder const &) = delete;
            Decoder & operator=(Decoder &&) = delete;

        private:
            virtual void codeFile(std::ifstream && ifs, std::ofstream && ofs);

            void setup(std::ifstream && ifs, std::ofstream && ofs);

            template<typename T>    T extract(void);
            template<typename T>    T readEncBuf(bool consume);

            void feed(uint8_t unread);

        private:
            std::ifstream               m_ifs;
            std::ofstream               m_ofs;

            std::unique_ptr<uint8_t[]>  m_encBuf;
            std::size_t                 m_encBufCap;
            std::size_t                 m_idx;
            std::uint8_t                m_nBits;

            SlidingWindow               m_sliWin;
        };
    }

    typedef decoder::Decoder    Decoder;
}

#endif
