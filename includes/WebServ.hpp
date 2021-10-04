#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>

#include "utils.hpp"
#include "Server.hpp"
class Server;
#include "Request.hpp"
#include "Response.hpp"
#include "Client.hpp"

struct location
{
	std::string 				name;
	std::vector<std::string>	methods;
	std::string 				index;
	std::string					root;
	bool		 				autoIndex;
	size_t						maxBodySize;
	std::map<int, std::string>	redir;			//	why map ?!
};

class WebServ
{
	private:
		std::vector<Server>		_servers;
		int						_maxFd;
		fd_set					_rFds, _wFds;

		void		_setFds();
		void		_select();

	public:
		WebServ(const std::string &);
		WebServ(const WebServ &);

		~WebServ();

		void		mainCycly();

		WebServ&	operator = (const WebServ &);
};
