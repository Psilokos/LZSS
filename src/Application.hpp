#ifndef APPLICATON_HPP_
# define APPLICATION_HPP_

# include <string>
# include <vector>

class Application
{
    public:
        Application(void) = delete;
        Application(std::string && name);
        Application(Application const &) = delete;
        Application(Application &&) = delete;
        ~Application(void) = default;

        Application operator=(Application const &) = delete;
        Application operator=(Application &&) = delete;

        int run(std::vector<std::string> const & filename) const;

    private:
        std::string const   m_name;
        bool const          m_encode;
};

#endif
