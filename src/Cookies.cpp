#include "Cookies.hpp"
#include <fstream>
#include <ctime>
#include <utility>
#include "general_utils.h"

using std::pair;
using std::string;

const char cookie_t::base64[64] = {
		'A','B','C','D','E','F','G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
		'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c',
		'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q',
		'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4',
		'5', '6', '7', '8', '9', '-', '_'};

cookie_t::cookie_t(void)
{
	char			buf[COOKIE_LEN];
	unsigned char	res[COOKIE_LEN + 1];
	std::ifstream	urandom("/dev/urandom", std::ios::in | std::ios::binary);

	if (!urandom)
		throw CookieError("cookie: opening /dev/urandom failed");

	urandom.read(buf, COOKIE_LEN);
	if (!urandom)
		throw CookieError("cookie: urandom.read() failed");

	for (int i = 0; i < COOKIE_LEN; i++)
		res[i] = base64[static_cast<unsigned char>(buf[i]) % 64];
	res[COOKIE_LEN] = '\0';

	id = "id=" + string(reinterpret_cast<char*>(res));
	last_visit = std::time(NULL);
	exp_date = last_visit + (60 * COOKIE_MAX_AGE);
	exp_date_str = "Expires=" + get_date(exp_date);
	last_visit_str = get_date(last_visit);
}

string	Cookies::create(void)
{
	cookie_t	new_cookie;

	m_cookies.insert(pair<string, cookie_t>(new_cookie.id, new_cookie));
	return (new_cookie.id + "; " + new_cookie.exp_date_str + "; Path=/;");
}

bool	Cookies::find(string id) const
{
	return (m_cookies.count(id));
}

cookie_t	&Cookies::at(string id)
{
	return (m_cookies.at(id));
}

void	Cookies::erase(string id)
{
	m_cookies.erase(id);
}

void		Cookies::remove_expired(void)
{
	std::map<string, cookie_t>::iterator	it = m_cookies.begin();
	std::vector<string>						expired;

	for(; it != m_cookies.end(); it++) {
		if (it->second.exp_date <= std::time(NULL))
			expired.push_back(it->first);
	}

	std::vector<string>::iterator	exp_it = expired.begin();
	for (; exp_it != expired.end(); exp_it++)
		m_cookies.erase(*exp_it);
}
