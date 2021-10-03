#pragma once

#include "WebServ.hpp"
// #include "Client.hpp"
class Client;

struct location;

class Server
{
	private:
		int							_sockFd;	/*	слушающий сокет	*/
		struct sockaddr_in			_addr;
		socklen_t					_addrLen;
		std::string					_root;
		std::string					_index;
		bool		 				_autoIndex;
		std::map<int, std::string>	_errors;
		std::vector<location>		_locations;
		std::vector<Client>			_clients;
		std::map<std::string, std::string>		_env;
		char**									_char_env;

		size_t		_parseLocation(const std::vector<std::string> &, size_t);
		void		_methodGet(const int &);
		void		_methodPost(const int &);	//	!!!
		std::string	_createListing(std::string, std::string &);
		void		_createListingStart(std::string &);
		void		_addRefToListing(std::string &, std::string &, std::string);
		void		_createListingEnd(std::string &);
		std::string	_makeDefaultPage();
		std::string	_checkType(const std::string &);
		bool		_findFile(std::string &, std::string &);
		ssize_t		_readChunke(const int &);
		void		_checkChunke(const int &);
		void		_isEndOfChunke(const int &);
		bool		_isMethodAllow(const location &, const std::string &);
		void		_initEnv(int itC);
		void		_setEnv(std::string, std::string);
		void 		_freeCharEnv();
		void		_executeCGI(int itC);

		void		_CGI(const int &);

		void		_makeCgiEnv(const int &itC);
		void 		execCGI(const int &itC,  const std::string &path);


	public:
		Server(const std::vector<std::string> &);
		Server(const Server &);

		~Server();

		void					setClient(Client&);

		int						getSockFd();
		unsigned short 			getPort();
		unsigned int			getClientsCount();
		int						getClientSockFd(const int &);
		int						getClientStatus(const int &);
		Client					getClient(const int &);

		void					acceptNewClient();
		ssize_t					readRequest(const int &itC);
		void					sendResponse(const int &);

		 bool					isClientRequest(const int &);
		 bool					isClientResponse(const int &);
		 void					makeClientResponse(const int &);

		Server&					operator = (const Server&);
};
