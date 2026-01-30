#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "request_parser.h"
# include "http_types.h"
# include <string>

class Response
{

public:

					Response(void);
					~Response(void);
				// 	Response(const Response &src);
	// Response		&operator=(const Response &rhs);
	const request_t	&get_request(void) const;
	void			set_request(const request_t &request);
	void			clear(void);
	const char		*get_buf(void) const;
	size_t			get_buf_size(void) const;

	int				m_sockfd;

private:

	request_t		m_request;
	std::string		m_buffer;

};

#endif
