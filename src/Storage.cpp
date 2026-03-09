#include "Storage.hpp"
#include "http_types.h"
#include "general_utils.h"
#include "response_utils.h"
#include <fstream>
#include <sys/stat.h>
#include <cstdio>

Storage::Storage(void)
: m_storage_infos(NULL)
{}

Storage::~Storage(void)
{}

void	Storage::clear(void)
{
	m_storage_infos = NULL;
	m_file_name.clear();
	m_file_path.clear();
}

// Should rename uplodat to storage
void	Storage::set_storage_infos(upload_t *upload)
{
	m_storage_infos = upload;
}

// Location path must end with '/'
int	Storage::init(list<string> &uri)
{
	list<string>::const_iterator	it_path = m_storage_infos->first.begin();
	list<string>::const_iterator	it_uri = uri.begin();

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
	if (request.headers.keep_alive)
		headers.keep_alive = true;

	switch (request.method) {
		case get:
			return (retrive(response_body, headers));
		case post:
			return (store(request.body, response_body, headers));
		case del:
			return (suppress());
		default:
			throw std::logic_error("storage: invalid exec method");
	}
}

status_t	Storage::retrive(string &body, headers_t &headers) const
{
	status_t	ret;

	ret = read_file_to_body(m_file_path, body);
	if (ret == ok)
		set_body_headers(headers, body, m_file_path);
	return (ret);
}

void	Storage::set_new_file_location(headers_t &headers) const
{
	list<string>::const_iterator	it = m_storage_infos->first.begin();

	headers.location.clear();
	while (it != m_storage_infos->first.end()) {
		headers.location += *it;
		it++;
	}
	headers.location += m_file_name;
}

status_t	Storage::store(const string &request_body, string &response_body, headers_t &headers) const
{
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
	set_new_file_location(headers);
	set_body_headers(headers, response_body, m_file_path);
	return (created);
}

status_t	Storage::suppress(void) const
{
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
