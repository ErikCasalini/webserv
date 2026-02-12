#ifndef HTTP_TYPES_H
# define HTTP_TYPES_H

# include <iostream>
# include <map>
# include <string>
# include <vector>
# include <sys/socket.h>
# include <sstream>
# include <netinet/in.h>
# include <cstring>
# include <unistd.h>

# define CRLF "\r\n"

enum method_t {
	del,
	get,
	post
};
std::ostream& operator<<(std::ostream& os, const method_t& m);

enum protocol_t {
	zero_nine,
	one,
	one_one,
	two,
	three
};
std::ostream& operator<<(std::ostream& os, const protocol_t& p);

enum status_t {
	parsing,
	ok,
	bad_request, // 400
	not_implemented // 501
};
std::ostream& operator<<(std::ostream& os, const status_t& s);

struct headers_t {
	headers_t();
	headers_t(const headers_t& src);
	headers_t& operator=(const headers_t& src);
	void clear();

	unsigned long content_length;
	std::string cookies;
	bool keep_alive;
	// TODO: store it as a date type? What format to handle
	std::string if_modified_since;
};
std::ostream& operator<<(std::ostream& os, const headers_t& h);

struct request_t {
	request_t();
	request_t(const request_t& src);
	request_t& operator=(const request_t& src);
	void clear();

	status_t status;
	method_t method;
	std::string target;
	protocol_t protocol;
	headers_t headers;
	std::string body;
};
std::ostream& operator<<(std::ostream& os, const request_t& r);

enum sock_type {
	active,
	passive
};

struct socket_t {
	socket_t();
	void clear();
	std::string str_peer_data(void) const;
	std::string str_data(void) const;
	bool operator==(socket_t &rhs) const;

	int fd;
	sock_type type;
	sockaddr_in peer_data;
	sockaddr_in data;
	int server_id;
};
std::ostream& operator<<(std::ostream& os, const socket_t& s);

#endif
