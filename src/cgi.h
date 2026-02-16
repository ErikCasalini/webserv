#include "Response.hpp"
#include <string>
#include <vector>
#include <sstream>

template <typename T>
std::string to_string(T num)
{
	std::stringstream s;
	s << num;
	return (s.str());
}

void delete_envp(char*** envp);

char** build_envp(const std::vector<std::string>& env);

const std::vector<std::string> prepare_env(const Response& response);

int close_pipes(int p1[2], int p2[2]);

// Catch exceptions to free envp or switch to an object
std::string exec_cgi(
		const char* script_name,
		const char* script_dir,
		const char* script_path,
		std::string request_body,
		char** envp);
