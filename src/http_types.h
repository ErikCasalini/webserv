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
# include <ctime>
# include <list>

struct config_t;
struct location_t;

# define CRLF "\r\n"

template <typename T>
std::string to_string(T num)
{
	std::stringstream s;
	s << num;
	return (s.str());
}

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
	parsing = 0,
	sending_resp = 1,
	ok = 200,
	created = 201,
	no_content = 204,
	moved_perm = 301,
	moved_temp = 302,
	temp_redir = 307,
	perm_redir = 308,
	bad_request = 400,
	forbidden = 403,
	not_found = 404,
	method_not_allowed = 405,
	internal_err = 500,
	not_implemented = 501,
	bad_gateway = 502
};
std::ostream& operator<<(std::ostream& os, const status_t& s);

enum	cgi_status_t
{
	init,
	write_to_child,
	read_from_child,
	done
};

struct headers_t {
	headers_t();
	headers_t(const headers_t& src);
	headers_t& operator=(const headers_t& src);
	void clear();

	unsigned long content_length;
	std::string content_type;
	std::string cookies;
	bool keep_alive;
	std::string date;
	std::string location;
	std::string server;
	// TODO: store it as a date type? What format to handle
	std::string if_modified_since;
	std::string allow;
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

enum file_stat {
	error,
	nonexistent,
	bad_perms,
	file,
	dir
};

enum sock_type {
	active,
	passive
};
std::ostream& operator<<(std::ostream &os, sock_type s);

struct cgi_uri_infos_t {
	int init(const location_t &location, std::list<std::string> path);
	std::string script_name;
	std::string script_dir;
	std::string path_info;
	std::string script_abs_path;
};

enum fd_type {
	sockt,
	cgi
};

struct epoll_item_t {
	fd_type type;
protected:
	epoll_item_t(fd_type type);
};

struct socket_t : public epoll_item_t {
	socket_t();
	void clear();
	std::string str_peer_interface(void) const;
	std::string str_peer_addr(void) const;
	std::string str_peer_port(void) const;
	std::string str_local_interface(void) const;
	std::string str_local_addr(void) const;
	std::string str_local_port(void) const;
	bool operator==(socket_t &rhs) const;
	bool timeout(config_t &config);

	int fd;
	sock_type socktype;
	int server_id;
	sockaddr_in peer_data;
	sockaddr_in local_data;
	socklen_t local_data_len;
	socklen_t peer_data_len;
	std::time_t last_activity;
};
std::ostream& operator<<(std::ostream& os, const socket_t& s);

struct pipes_t {

	pipes_t(void);
	~pipes_t(void);
	void clear(void);

	int fd_in;
	int fd_out;
};

#endif
