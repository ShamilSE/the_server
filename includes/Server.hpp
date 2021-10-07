#pragma once

#include "WebServ.hpp"
class Client;

struct location;

class Server
{
	private:
		int							_sockFd;
		struct sockaddr_in			_addr;
		socklen_t					_addrLen;
		std::string					_root;
		std::string					_index;
		bool		 				_autoIndex;
		std::map<int, std::string>	_errors;
		std::vector<location>		_locations;
		std::vector<Client>			_clients;
		size_t 						_envCount;
		char**						_env;

		std::string	_makeAutoindex(Client &);

		size_t		_parseLocation(const std::vector<std::string> &, size_t);
		void		_methodGet(Client &);
		void		_methodPost(Client &);
		std::string	_createListing(std::string &);
		void		_createListingStart(std::string &);
		void		_addRefToListing(std::string &, std::string &, std::string);
		void		_createListingEnd(std::string &);
		std::string	_makeDefaultPage();
		std::string	_checkType(const std::string &);
		bool		_findFile(const std::string &, const std::string &);
		void		_readChunke(Client &);
		bool		_isEndOfChunke(Client &);
		bool		_isMethodAllow(const location &, const std::string &);

		void		_makeCgiEnv(Client &);
		void		_CGI(Client &, const std::string &);

	public:
		Server(const std::vector<std::string> &);
		Server(const Server &);

		~Server();

		void					acceptNewClient();
		void					readRequest(Client &);
		void					sendResponse(Client &);

		int						getSockFd();
		unsigned int			getClientsCount();
		Client&					getClientRef(const int &);
		int						getClientSockFd(const int &);	//	!!!
		std::string				getErrorByKey(int);

		void					setError(int, std::string);

		 bool					isClientResponse(const int &);
		 void					makeClientResponse(Client &);
		 void					eraseClient(const int &);
		std::vector<Client>::iterator	disconectUser(const int &itC);

		Server&					operator = (const Server&);
};
