#ifndef STORAGE_HPP
# define STORAGE_HPP

# include "Config.h"
# include <string>
# include <list>

using std::string;
using std::list;

class Storage {

public:

				Storage(void);
				~Storage(void);
	void		clear(void);

	void		set_storage_infos(upload_t *upload);

	int			init(list<string> &uri);
	status_t	exec(request_t &request, string &body, headers_t &headers);

private:

	void		store(void);
	string		retrive(void);
	void		del(void);

	string		m_file_name;
	upload_t	*m_storage_infos;

}

#endif
