#ifndef LZSS_INT_CODEC_HPP_
# define LZSS_INT_CODEC_HPP_

# include <fstream>
# include <queue>
# include <string>
# include <utility>

namespace lzss
{
    class Codec
    {
        protected:
            Codec(void) = default;
            Codec(Codec const &) = delete;
            Codec(Codec &&) = delete;

        public:
            virtual ~Codec(void) = default;

            Codec & operator=(Codec const &) = delete;
            Codec & operator=(Codec &&) = delete;

            bool addFile(std::string const & filename);
            Codec & operator<<(std::string const & filename);

            bool hasFile(void) const;

            void run(void);
            void operator()(void);

            void codeFile(void);

        protected:
            virtual void codeFile(std::ifstream && ifs, std::ofstream && ofs) = 0;

        protected:
            typedef std::queue<std::pair<std::ifstream, std::ofstream>> FileQueue;

            FileQueue   m_fileQ;
    };
}

#endif
