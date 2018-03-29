#ifndef LZSS_INT_DECODER_HPP_
# define LZSS_INT_DECODER_HPP_

# include "Codec.hpp"

namespace lzss
{
    class Decoder : public Codec
    {
        public:
            Decoder(void) = default;
            Decoder(Decoder const &) = delete;
            Decoder(Decoder &&) = delete;
            virtual ~Decoder(void) = default;

            Decoder & operator()(Decoder const &) = delete;
            Decoder & operator()(Decoder &&) = delete;

        private:
            virtual void codeFilePriv(std::ifstream & ifs, std::ofstream & ofs);
    };
}

#endif
