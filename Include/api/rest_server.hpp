
#ifndef REST_SERVER_HPP
#define REST_SERVER_HPP

#include "database/database_toolkit.hpp"

class RestServer
{
public:
	void run(unsigned short port = 8080);

private:
	DatabaseToolkit database_;
};

#endif 