#ifndef COOKIES_HPP
# define COOKIES_HPP

# include <map>
# include <string>
# include <ctime>
# include <stdexcept>

# define COOKIE_LEN 32
# define COOKIE_MAX_AGE 30 // minutes

using std::map;
using std::string;
using std::time_t;

class	CookieError : public std::runtime_error
{
	public:
			CookieError(const std::string &str)
			: std::runtime_error(str)
			{};
};

struct cookie_t
{
						cookie_t(void);

	static const char	base64[64];

	string				id;
	time_t				exp_date;
	time_t				last_visit;
	string				exp_date_str;
	string				last_visit_str;
};

class Cookies
{

public:

	bool		find(string id) const;
	string		create(void);
	cookie_t	&at(string id);
	void		erase(string id);

private:

	map<string, cookie_t>	m_cookies;

};

#endif
