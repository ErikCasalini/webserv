#include "Config.h"

listen_t::listen_t()
	: ip(DEFAULT_IP)
	, port(DEFAULT_PORT)
{};

location_t::location_t()
	: autoindex(false)
	, cgi(false)
	, cgi_nph(false)
	, exact_match(false)
	, redirection(parsing, "")
{};

server_t::server_t()
	: autoindex(false)
	, redirection(parsing, "")
{};

http_t::http_t()
	: autoindex(false)
	, keepalive_timeout(0)
	, max_body_size(1048576) // default 1MB
	, default_type("application/octet-stream")
{
	error_page[400] = DEFAULT_ERROR_400;
	error_page[401] = DEFAULT_ERROR_401;
	error_page[402] = DEFAULT_ERROR_402;
	error_page[403] = DEFAULT_ERROR_403;
	error_page[404] = DEFAULT_ERROR_404;
	error_page[500] = DEFAULT_ERROR_500;
	error_page[501] = DEFAULT_ERROR_501;
	error_page[502] = DEFAULT_ERROR_502;
};

events_t::events_t()
	: max_connections(256)
{};
