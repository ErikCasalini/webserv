#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "request_parser.h"
# include "http_types.h"
# include <string>
# include <list>

class Response
{

public:

						Response(void);
						~Response(void);
				// 		Response(const Response &src);
	// Response			&operator=(const Response &rhs);
	const request_t		&get_request(void) const;
	void				set_request(const request_t &request);
	void				clear(void);
	const char			*get_buf(void) const;
	const std::string	&get_path(void) const;
	const std::string	&get_querry(void) const;
	size_t				get_buf_size(void) const;
	void				parse_uri(void);

	int					m_sockfd;
	static const char	authorized_chars[];

private:

	request_t		m_request;
	std::string		m_buffer;
	std::string		m_querry;
	std::string		m_path;
	status_t		m_status;
};

namespace _Response
{
	void						extract_uri_elem(std::string&uri, std::string &path, std::string &querry);
	std::list<std::string>		split_path(const std::string &path);
}

#endif
