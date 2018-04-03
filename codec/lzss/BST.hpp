#ifndef LZSS_INT_BST_HPP_
# define LZSS_INT_BST_HPP_

# include <cxxabi.h>
# include <functional>
# include <iomanip>
# include <iostream>
# include <typeinfo>
# include <utility>

class ITree
{
    public:
        typedef std::pair<std::type_info const &, std::type_info const &>   UnderlyingTypes_t;

    public:
        virtual ~ITree(void) = default;

        virtual UnderlyingTypes_t underlyingTypes(void) const = 0;
        virtual void dumpUnderlyingTypes(void) const = 0;
};

class ATree : public ITree
{
    public:
        virtual ~ATree(void) = default;

        virtual UnderlyingTypes_t underlyingTypes(void) const = 0;

        virtual void dumpUnderlyingTypes(void) const
        {
            std::function<void(char const *)> deleter = [](char const * ptr) { std::free(const_cast<char *>(ptr)); };
            std::unique_ptr<char const, decltype(deleter)> str(nullptr, deleter);
            auto uTypes = this->underlyingTypes();

            str.reset(abi::__cxa_demangle(uTypes.first.name(), nullptr, nullptr, nullptr));
            std::cout << "key_type:\t" << str.get() << std::endl;

            str.reset(abi::__cxa_demangle(uTypes.second.name(), nullptr, nullptr, nullptr));
            std::cout << "value_type:\t" << str.get() << std::endl;
        }
};

template<typename K, typename V>
class BST : public ATree
{
    public:
        typedef BST<K, V>   Type;

    private:
        typedef std::unique_ptr<Type>   PointerType;

    public:
        BST(void) = default;

        template<typename... Args>
        BST(K const & key, Args && ... args) :
            m_init(true), m_data(std::make_pair(key, V(std::forward<Args>(args)...)))
        {
        }

        BST(Type const &) = delete;

        BST(Type && rhs) :
            m_init(rhs.m_init),
            m_data(std::move(rhs.m_data)),
            m_left(std::move(rhs.m_left)),
            m_right(std::move(rhs.m_right))
        {
            rhs.m_init = decltype(m_init)();
        }

        virtual ~BST(void) = default;

        Type & operator=(Type const &) = delete;

        Type & operator=(Type && rhs)
        {
            m_init = rhs.m_init;
            rhs.m_init = decltype(m_init)();
            m_data = std::move(rhs.m_data);
            if (&rhs == m_left.get())
            {
                m_right = std::move(rhs.m_right);
                m_left = std::move(rhs.m_left);
            }
            else
            {
                m_left = std::move(rhs.m_left);
                m_right = std::move(rhs.m_right);
            }
        }

        virtual UnderlyingTypes_t underlyingTypes(void) const
        {
            return std::make_pair(std::cref(typeid(K)), std::cref(typeid(V)));
        }

        K const & key(void) const
        {
            return m_data.first;
        }

        V const & value(void) const
        {
            return m_data.second;
        }

        void value(V const & val)
        {
            if (m_init)
                m_data.second = val;
        }

        Type const & min(void) const
        {
            return const_cast<Type *>(this)->min();
        }

        Type & min(void)
        {
            return m_left ? **this->min(nullptr) : *this;
        }

        Type const & max(void) const
        {
            return const_cast<Type *>(this)->max();
        }

        Type & max(void)
        {
            return m_right ? **this->max(nullptr) : *this;
        }

        template<typename... Args>
        void reset(K const & key, Args && ... args)
        {
            m_data = std::make_pair(key, V(std::forward<Args>(args)...));
            m_left.reset();
            m_right.reset();
            m_init = true;
        }

        template<typename... Args>
        void insert(K const & key, Args && ... args)
        {
            if (!m_init)
                return this->reset(key, std::forward<Args>(args)...);

            Type * node = this;
            while (true)
            {
                if (key == node->m_data.first)
                {
                    node->m_data.second = V(std::forward<Args>(args)...);
                    return;
                }

                PointerType & child = key < node->m_data.first ? node->m_left : node->m_right;
                if (child)
                    node = child.get();
                else
                {
                    child = std::make_unique<Type>(key, std::forward<Args>(args)...);
                    return;
                }
            }
        }

        void remove(K const & key, PointerType * p_self = nullptr)
        {
            if (!m_init)
                return;

            if (key < m_data.first)
                m_left->remove(key, &m_left);
            else if (key > m_data.first)
                m_right->remove(key, &m_right);
            else
            {
                if (!m_left && !m_right)
                {
                    if (p_self)
                        p_self->reset();
                    else
                        m_init = false;
                }
                else if (!m_left || !m_right)
                {
                    PointerType & child = m_left ? m_left : m_right;
                    if (p_self)
                        *p_self = std::move(child);
                    else
                        *this = std::move(*child);
                }
                else
                {
                    PointerType & min = *m_right->min(&m_right);
                    if (p_self)
                        *p_self = std::move(min);
                    else
                    {
                        m_data = std::move(min->m_data);
                        min.reset();
                    }
                }
            }
        }

        operator bool(void) const
        {
            return m_init;
        }

        static void dump(Type const * p_self, std::ostream & os, std::size_t depth = 0)
        {
            os << std::string(depth * 2, ' ');
            if (!p_self || !p_self->m_init)
            {
                os << "[]" << std::endl;
                return;
            }

            os << '[' << p_self->m_data.first << "] " << std::quoted(std::to_string(p_self->m_data.second), '|') << std::endl;
            Type::dump(p_self->m_left.get(), os, depth + 1);
            Type::dump(p_self->m_right.get(), os, depth + 1);
        }

    private:
        PointerType * min(PointerType * p_self)
        {
            if (!m_init)
                return nullptr;

            if (!m_left)
                return p_self;
            PointerType * node = &m_left;
            while ((*node)->m_left)
                node = &(*node)->m_left;
            return node;
        }

        PointerType & max(PointerType * p_self)
        {
            if (!m_init)
                return nullptr;

            if (!m_right)
                return p_self;
            PointerType * node = &m_right;
            while ((*node)->m_right)
                node = &(*node)->m_right;
            return node;
        }

    private:
        bool            m_init;
        std::pair<K, V> m_data;
        PointerType     m_left;
        PointerType     m_right;
};

template<typename K, typename V>
std::ostream & operator<<(std::ostream & os, BST<K, V> const & rhs)
{
    BST<K, V>::dump(&rhs, os);
    return os;
}

#endif
