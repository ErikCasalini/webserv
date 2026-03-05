#include "Storage.hpp"

Storage::Storage(void)
: m_storage_infos(NULL)
{}

Storage::~Storage(void)
{}

void	Storage::clear(void)
{
	m_storage_infos = NULL;
	m_file_name.clear();
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
	return (0);
}

status_t	Storage::exec(request_t &request, string &body, headers_t &headers)
{

}

void	Storage::store(void)
{

}

string	Storage::retrive(void)
{

}

void	Storage::del(void)
{

}
