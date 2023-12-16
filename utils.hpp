#ifndef FT_IRC_UTILS_HPP
# define FT_IRC_UTILS_HPP

# include <sstream>

template<typename T>
std::string to_string(const T & value) {
    std::ostringstream oss;
    oss << value;
    return (oss.str());
}

#endif
