#include "Config.h"

listen_t::listen_t()
	: ip(0x7F000001) // 127.0.0.1
	, port(80)
{};

location_t::location_t()
	: autoindex(false)
	, cgi(false)
	, exact_match(false)
	, redirection(parsing, "")
{};

server_t::server_t()
	: autoindex(false)
	, max_body_size(1048576) // default 1Mo
	, redirection(parsing, "")
{};

http_t::http_t()
	: autoindex(false)
	// , error_page()
	, keepalive_timeout(0)
	, max_body_size(1048576) // default 1Mo
	, default_type("application/octet-stream")
{};

events_t::events_t()
	: max_connections(256)
{};
