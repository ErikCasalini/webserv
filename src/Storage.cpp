#include "Storage.hpp"
#include "http_types.h"
#include "general_utils.h"
#include "response_utils.h"
#include <fstream>
#include <sys/stat.h>
#include <cstdio>

Storage::Storage(const config_t &config)
: m_storage_infos(NULL),
  m_config(config)
{}

Storage::~Storage(void)
{}

Storage		&Storage::operator=(const Storage &rhs)
{
	if (this != &rhs) {
		m_file_name = rhs.m_file_name;
		m_file_path = rhs.m_file_path;
		m_storage_infos = rhs.m_storage_infos;
	}
	return (*this);
}

void	Storage::clear(void)
{
	m_storage_infos = NULL;
	m_file_name.clear();
	m_file_path.clear();
}

void	Storage::set_storage_infos(const storage_t *storage)
{
	m_storage_infos = storage;
}

// Location path must end with '/'
int	Storage::init(list<string> &uri)
{
	list<string>::const_iterator	it_path = m_storage_infos->first.begin();
	list<string>::const_iterator	it_uri = uri.begin();

	if (uri == m_storage_infos->first)
		return (0);

	if (uri.size() != m_storage_infos->first.size() + 1)
		return (-1);

	for (; it_path != m_storage_infos->first.end(); it_path++, it_uri++) {
		if (*it_path != *it_uri)
			return (-1);
	}
	m_file_name = *it_uri;
	m_file_path = m_storage_infos->second + m_file_name;
	return (0);
}

status_t	Storage::exec(const request_t &request, string &response_body, headers_t &headers) const
{
	status_t	ret;

	switch (request.method) {
		case get:
			ret = retrive(response_body, headers);
			break ;
		case post:
			ret = store(request.body, response_body, headers);
			break ;
		case del:
			ret = suppress();
			break ;
		default:
			throw std::logic_error("storage: invalid exec method");
	}
	if (ret == bad_request)
		headers.keep_alive = false;

	return (ret);
}

status_t	Storage::retrive(string &body, headers_t &headers) const
{
	status_t	ret;

	if (m_file_path == "") {
		ret = generate_indexing(m_storage_infos->second, body, get_new_file_location());
		if (ret == ok) {
			headers.content_length = body.size();
			headers.content_type = "text/html";
		}
		return (ret);
	}

	ret = read_file_to_body(m_file_path, body);
	if (ret == ok)
		set_body_headers(headers, body, m_file_path, m_config);
	return (ret);
}

string	Storage::get_new_file_location(void) const
{
	list<string>::const_iterator	it = m_storage_infos->first.begin();
	string							ret;
	while (it != m_storage_infos->first.end()) {
		ret += *it;
		it++;
	}
	ret += m_file_name;
	return (ret);
}

status_t	Storage::store(const string &request_body, string &response_body, headers_t &headers) const
{
	if (m_file_name == "")
		return (bad_request);

	switch (get_file_type(m_file_path)) {
		case dir:
		case bad_perms:
			return (forbidden);
		case error:
			return (internal_err);
		case file:
			if (access(m_file_path.c_str(), W_OK) == -1)
				return (forbidden);
			break ;
		default: //nonexistent
			break ;
	}

	std::ofstream	file(m_file_path.c_str());

	if (file.fail())
		return (internal_err);

	file << request_body;

	if (file.bad())
		return (internal_err);

	response_body = request_body;
	headers.location = get_new_file_location();
	set_body_headers(headers, response_body, m_file_path, m_config);
	return (created);
}

status_t	Storage::suppress(void) const
{
	if (m_file_name == "")
		return (bad_request);

	switch (get_file_type(m_file_path)) {
		case dir:
		case bad_perms:
			return (forbidden);
		case error:
			return (internal_err);
		case nonexistent:
			return (not_found);
		default: //file
			break ;
	}

	errno = 0;
	std::remove(m_file_path.c_str());
	switch (errno) {
		case 0:
			return (no_content);
		case EPERM:
		case EACCES:
			return (forbidden);
		default:
			return (internal_err);
	}
}
