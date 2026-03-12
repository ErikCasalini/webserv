#ifndef STORAGE_HPP
# define STORAGE_HPP

# include "Config.h"
# include <string>
# include <list>

using std::string;
using std::list;

class Storage {

public:

				Storage(const config_t &config);
				~Storage(void);
	Storage		&operator=(const Storage &rhs);
	void		clear(void);

	void		set_storage_infos(const storage_t *storage);

	int			init(list<string> &uri);
	status_t	exec(const request_t &request, string &body, headers_t &headers) const;

private:

	string		get_new_file_location(void) const;
	status_t	store(const string &request_body, string &response_body, headers_t &headers) const;
	status_t	retrive(string &body, headers_t &headers) const;
	status_t	suppress(void) const;

	string			m_file_name;
	string			m_file_path;
	const storage_t	*m_storage_infos; // both Storage path and root should end and start by '/'
	const config_t	&m_config;

};

#endif
