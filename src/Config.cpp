#include "Config.h"

listen_t::listen_t()
	: ip(0)
	, port(0)
{};

location_t::location_t()
	: exact_match(false)
	, cgi(false)
	, autoindex(false)
	, redirection(parsing, "")
{};

server_t::server_t()
	: autoindex(false)
	, redirection(parsing, "")
{};

http_t::http_t()
	: autoindex(false)
	, max_body(1048576) // default 1Mo
	, keepalive_timeout(0)
{};

events_t::events_t()
	: max_connections(256)
{};
