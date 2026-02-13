#include "../src/cgi.h"
#include "lib_test.h"
#include <vector>
#include <string>

using std::vector;
using std::string;

vector<string> mock_envp_vector()
{
	vector<string> v;
	v.push_back("CONTENT_LENGTH=20");
	v.push_back("GATEWAY_INTERFACE=CGI/1.1");
	v.push_back("SERVER_PROTOCOL=HTTP/1.0");
	v.push_back("SERVER_SOFTWARE=webserv/2026");
	return (v);
}

void test_build_envp()
{
	vector<string> v = mock_envp_vector();

	char** envp = build_envp(v);
	string tmp;
	assert(((tmp = envp[0]) == "CONTENT_LENGTH=20"));
	assert(((tmp = envp[1]) == "GATEWAY_INTERFACE=CGI/1.1"));
	assert(((tmp = envp[2]) == "SERVER_PROTOCOL=HTTP/1.0"));
	assert(((tmp = envp[3]) == "SERVER_SOFTWARE=webserv/2026"));
	assert((envp[4] == NULL));
	delete_envp(&envp);
}

void test_exec_cgi()
{
	vector<string> v = mock_envp_vector();
	char** envp = build_envp(v);
#define SCRIPT "export.sh"
	string response = exec_cgi(
						SCRIPT,
						"/home/jules/Repositories/projets_42/webserv/test",
						"/home/jules/Repositories/projets_42/webserv/test/" SCRIPT,
						"Hello I'm the body!",
						envp);
	std::cout << response << '\n';
}

void test_delete_envp()
{
}

int main(void)
{
	// TEST(test_build_envp);
	TEST(test_exec_cgi);
	return (0);
}
