#ifndef LZSS_INT_ENCODER_HPP_
# define LZSS_INT_ENCODER_HPP_

# include <any>
# include "Codec.hpp"
# include "SlidingWindow.hpp"

namespace lzss
{
    class Encoder : public Codec
    {
        public:
            Encoder(void) = default;
            Encoder(Encoder const &) = delete;
            Encoder(Encoder &&) = delete;
            virtual ~Encoder(void) = default;

            Encoder & operator=(Encoder const &) = delete;
            Encoder & operator=(Encoder &&) = delete;

        private:
            virtual void codeFilePriv(std::ifstream & ifs, std::ofstream & ofs);

            std::any encodeView(void);
            template<typename T> void writeToBuffer(T code);
            void writeChunkToFile(bool force = false);

        private:
            std::ofstream           m_ofs;
            SlidingWindow           m_sliWin;
            std::vector<uint8_t>    m_encBuf;
            std::uint8_t            m_nBits;
    };
}

#endif
