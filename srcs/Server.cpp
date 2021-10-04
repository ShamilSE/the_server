#include "../includes/Server.hpp"
#include <dirent.h>

Server::Server(const std::vector<std::string> &conf) : _sockFd(-1), _root(""), _index("")
{
	int reuseOpt = 1;

	bzero(&_addr, _addrLen);
	for (size_t i = 0; i < conf.size(); i++)
	{
		if (conf[i].find("host:") != std::string::npos)
		{
			std::string host = ft_strtrim(conf[i].substr(5), " \t");
			inet_aton(host.c_str(), &_addr.sin_addr);
		}
		if (conf[i].find("listen:") != std::string::npos)
		{
			std::string	port = ft_strtrim(conf[i].substr(7), " \t");
			_addr.sin_port = htons(stoi(port));
		}
		if (conf[i].find("index:") != std::string::npos)
		{
			if (conf[i].find("auto_index:") != std::string::npos)
			{
				std::string	tmpStr;

				tmpStr = conf[i].substr(11);
				tmpStr = ft_strtrim(tmpStr, " \t");
				_autoIndex = (tmpStr == "on" ? true : false);
			}
			else
				_index	= ft_strtrim(conf[i].substr(6), " \t");
		}
		if (conf[i].find("root:") != std::string::npos)
			_root = ft_strtrim(conf[i].substr(5), " \t");
		if (conf[i].find("error_page:") != std::string::npos)
		{
			std::string					tmpStr;
			std::vector<std::string>	tmpStrVec;

			tmpStr = ft_strtrim(conf[i].substr(11), " \t");
			tmpStrVec = ft_split(tmpStr, " ");
			_errors[std::atoi(tmpStrVec[0].c_str())] = tmpStrVec[1];
		}
		if (conf[i].find("location:") != std::string::npos)
			i = _parseLocation(conf, i);
	}
	_addr.sin_family = AF_INET;
	_addrLen = sizeof(_addr);
	_sockFd = socket(AF_INET, SOCK_STREAM, 0);	/*	создание сокета	*/
	if (_sockFd == -1)
		throw std::string("sock err");
	fcntl(_sockFd, F_SETFL, O_NONBLOCK);	/*	перевод сокета в неблокирующий режим	*/
	if (setsockopt(_sockFd, SOL_SOCKET, SO_REUSEADDR, &reuseOpt, sizeof(reuseOpt)) == -1)	/*	повторное использование порта	*/
		throw std::string("setsockopt err");
	if (bind(_sockFd, (sockaddr*)&_addr, _addrLen) == -1)	/*	привязка сокета к адресу	*/
		throw std::string("bind err");
	if (listen(_sockFd, 128) == -1)	/*	перевод сокета в слушающий режим	*/
		throw std::string("listen err");
	std::cout << "\033[1;35m\t\tserver: " << htons(_addr.sin_port) << " listening\033[0m" << std::endl;
}

Server::Server(const Server &other) { *this = other; }

Server::~Server() {}

size_t	Server::_parseLocation(const std::vector<std::string> &conf, size_t i)
{
	std::string					tmpStr;

	if (i != conf.size())
	{
		location	loc;

		loc.root = _root;
		loc.index = _index;
		loc.autoIndex = _autoIndex;
		tmpStr = conf[i].substr(9);
		loc.name = ft_strtrim(tmpStr, " \t");
		i++;
		for ( ; i < conf.size() && conf[i].find("location:") == std::string::npos ; ++i)
		{
			if (conf[i].find("allow_methods:") != std::string::npos)
			{
				tmpStr = conf[i].substr(14);
				tmpStr = ft_strtrim(tmpStr, " \t");
				loc.methods = ft_split(tmpStr, " ");
			}
			if (conf[i].find("index:") != std::string::npos)
			{
				if (conf[i].find("auto_index:") != std::string::npos)
				{
					tmpStr = conf[i].substr(11);
					tmpStr = ft_strtrim(tmpStr, " \t");
					loc.autoIndex = (tmpStr == "on" ? true : false);
				}
				else
				{
					tmpStr = conf[i].substr(6);
					loc.index = ft_strtrim(tmpStr, " \t");
				}
			}
			if (conf[i].find("root:") != std::string::npos)
			{
				tmpStr = conf[i].substr(5);
				loc.root = ft_strtrim(tmpStr, " \t");
			}
			if (conf[i].find("max_body_size:") != std::string::npos)
			{
				tmpStr = conf[i].substr(14);
				tmpStr = ft_strtrim(tmpStr, " \t");
				loc.maxBodySize = atoi(tmpStr.c_str());
			}
			if (conf[i].find("redirect:") != std::string::npos)
			{
				std::vector<std::string>	tmpStrVec;

				tmpStr = conf[i].substr(9);
				tmpStr = ft_strtrim(tmpStr, " \t");
				tmpStrVec = ft_split(tmpStr, " ");
				loc.redir[std::atoi(tmpStrVec[0].c_str())] = tmpStrVec[1];
			}
		}
		_locations.push_back(loc);
		return _parseLocation(conf, i);
	}
	return i;
}

int	Server::getSockFd() { return _sockFd; }


unsigned int	Server::getClientsCount() { return _clients.size(); }

int	Server::getClientSockFd(const int &itC) { return _clients[itC].getSockFd(); }

void	Server::acceptNewClient()
{
	Client	newClient;

	int clientFd = accept(_sockFd, (sockaddr*)&newClient.getAddrRef(), &newClient.getAddrLenRef());
	if (clientFd == -1)
		throw std::string("accept err");
	fcntl(clientFd, F_SETFL, O_NONBLOCK);
	newClient.setSockFd(clientFd);
	newClient.setStatus(NEW_CLIENT);
	_clients.push_back(newClient);
	std::cout << "\033[1;35m\t\tnew client (";
	std::cout << newClient;
	std::cout << ") connect to server: ";
	std::cout << ntohs(_addr.sin_port);
	std::cout << "\033[0m" << std::endl;
}

bool	Server::_isEndOfChunke(Client &client)
{
	std::string		content = client.getChunke();

	if (content.find("0\r\n\r\n") != std::string::npos)
	{
		client.setRequestBody(client.getAllChunke());
		makeClientResponse(client);
		client.setStatus(WAITING_FOR_RESPONSE);
		client.clearChunke();
		client.clearAllChunke();
		client.clearChunkeSize();
		return true;
	}
	return false;
}

ssize_t	Server::_readChunke(Client &client)
{
	int				clientFd = client.getSockFd();
	int				bytesRead;

	if (client.getChunkeSize() == 0)
	{
		char	buff[2];
		bytesRead = recv(clientFd, buff, 1, 0);
		if (bytesRead > 0)
		{
			buff[bytesRead] = 0;
			client.addChunkePart(buff);
			if (_isEndOfChunke(client) == true)
				return bytesRead;
			else
			{
				std::string content = client.getChunke();
				if ((content.find("\r\n")) != std::string::npos)
				{
					if (content[0] == '0')
						return bytesRead;
					client.setChunkeSize(std::strtol(content.c_str(), NULL, 16));
					client.clearChunke();
				}
			}
		}
		else if (bytesRead == 0)
			return bytesRead;
		else if (bytesRead == -1 )
			throw std::string("recv err");
	}
	else
	{
		std::string	content = client.getChunke();
		int			alreadyRead = content.size();
		int			needToRead = client.getChunkeSize() - alreadyRead;
		if (needToRead == 0)
			needToRead = 3;
		char		buff[needToRead];
		bytesRead = recv(clientFd, buff, needToRead, 0);
		if (bytesRead > 0)
		{
			buff[bytesRead] = 0;
			if (needToRead == 3 && std::string(buff) == "\r\n")
			{
				client.clearChunke();
				client.clearChunkeSize();
				return bytesRead;
			}
			client.addChunkePart(buff);
			if (client.getChunke().size() == client.getChunkeSize())
			{
				client.addAllChunke(client.getChunke());
				client.clearChunke();
				client.clearChunkeSize();
				// std::cout << "\033[1;35m\t\tchunked request from client (";
				// std::cout << client;
				// std::cout << ")\033[0m" << std::endl;
			}
		}
		else if (bytesRead == 0)
			return bytesRead;
		else if (bytesRead == -1 )
			throw std::string("recv err");
	}
	return bytesRead;
}

ssize_t	Server::readRequest(Client &client)
{
	size_t	buffer_size = 2;
	char	buff[buffer_size];
	int		bytesRead;

	if (client.getStatus() == CHUNKED)
		return _readChunke(client);
	// else if (client.getStatus() == PARTIAL_CONTENT)
	// {
	// 	int	allReadedBytesCount = 0;
	// 	buffer_size = 500;

	// 	while (allReadedBytesCount < std::stoi(client.getRequest().getHeaderByKey("Content-Length")))
	// 	{
	// 		bytesRead = recv(client.getSockFd(), buff, buffer_size - 1, 0);
	// 		if (bytesRead > 0)
	// 		{
	// 			buff[bytesRead] = 0;
	// 			client.setRequestBody(buff);
	// 		}
	// 		else if (bytesRead == 0)
	// 			return allReadedBytesCount;
	// 	}

	// 	return allReadedBytesCount;
	// }
	else
	{
		bytesRead = recv(client.getSockFd(), buff, buffer_size - 1, 0);
		if (bytesRead > 0)
		{
			buff[bytesRead] = '\0';
			client.setRequest(buff);
		}
		else if (bytesRead == 0)
			return CLOSE_CONECTION;
		std::string requestHeader = client.getRequest().getRequest();
		if (requestHeader.find("\r\n\r\n") != std::string::npos)
		{
			client.parseRequestHeader();
			if (client.getRequest().getHeaderByKey("Content-Length") != "")
				client.setStatus(PARTIAL_CONTENT);
			else if (client.getHeaderInfo("Transfer-Encoding").compare(0, 7, "chunked") == 0)
				client.setStatus(CHUNKED);
			else
			{
				makeClientResponse(client);
				client.setStatus(WAITING_FOR_RESPONSE);
			}
		}
	}
	return bytesRead;
}

void	Server::makeClientResponse(Client &client)
{
	std::string method = client.getRequest().getMethod();
	if (method == "GET")
		_methodGet(client);
	else if (method == "POST" || method == "PUT")
		_methodPost(client);
	else
		client.setResponseStatus("405 Method Not Allowed");
}

void	Server::_createListingStart(std::string &content)
{
	content = "<!DOCTYPE HTML>\n";
	content += "<html>\n";
	content += "\t<head>\n";
	content += "\t\t<meta charset=\"utf-8\">\n";
	content += "\t</head>\n";
	content += "\t<body>\n";
}

void	Server::_addRefToListing(std::string &content, std::string &ref, std::string fileName)
{
	content += "\t\t<p><a href=\"";
	content += ref;
	content += "\">";
	content += fileName;
	content += "</a></p>\n";
}

void	Server::_createListingEnd(std::string &content)
{
	content += "\t</body>\n";
	content += "</html>\n";
}


std::string	Server::_createListing(std::string serverRoot, std::string &path)
{
	DIR				*dir;
	struct dirent	*ent;
	std::string		content;
	std::string		tmp;

	// std::cout << "opendir:\t" << path << std::endl;
    dir = opendir(path.c_str());
	if (dir == NULL)
	{
		if (errno == ENOENT)		//	Directory not found
			return "";				//	!!!!!!!!!!!!!!!
		else
			throw std::string ("opendir err");
	}
	if (path.back() != '/')
		path.append("/");
	if (serverRoot.back() != '/')
		serverRoot.append("/");
	_createListingStart(content);
	while ((ent = readdir(dir)) != false)
	{
		tmp = path + ent->d_name;
		if (path == serverRoot && ( !strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")))
			continue ;
		_addRefToListing(content, tmp, ent->d_name);
	}
	if (closedir(dir) == -1)
		throw std::string ("closedir err");
	_createListingEnd(content);
	return content;
}

std::string	Server::_makeDefaultPage()
{
	std::string	page;

	page = "<!DOCTYPE html>\n";
	page += "<html>\n";
	page += "\t<head>\n";
	page += "\t\t<title>Welcome to Aquinoa's server!</title>\n";
	page += "\t\t<style>\n";
	page += "\t\t\thtml { color-scheme: light dark; }\n";
	page += "\t\t\tbody { width: 35em; margin: 0 auto;\n";
	page += "\t\t\tfont-family: Tahoma, Verdana, Arial, sans-serif; }\n";
	page += "\t\t</style>\n";
	page += "\t</head>\n";
	page += "\t<body>\n";
	page += "\t\t<h1>Welcome to <a href=\"https://profile.intra.42.fr/users/aquinoa\">Aquinoa</a> server!</h1>\n";
	page += "\t\t<p>If you see this page, the web server is successfully installed and\n";
	page += "\t\tworking. Further configuration is required.</p>\n";
	page += "\t\t<p><em>Thank you for using this server.</em></p>\n";
	page += "\t</body>\n";
	page += "</html>\n";
	return page;
}

std::string	Server::_checkType(const std::string &url)
{
	const char *types [] = { ".bla", ".epub",".js", ".json", ".jar", ".doc", ".pdf", ".xml", ".docx",
							".rar", ".zip", ".tar", ".mp3", ".mp4a", ".mpga", ".wma", ".wax",
							".wav", ".bmp", ".jpg", ".jpeg", ".gif", ".png", ".xpm", ".ico",
							".html", ".css", ".csv", ".txt", ".rtx", ".yaml", ".bmp", ".3gp",
							".mp4", ".mpeg", ".avi", ".movie",
							".bad_extension", ".bla", ".pouic", NULL };

	for (size_t i = 0; types[i] != NULL; ++i)
	{
		if (url.find(types[i]) != std::string::npos)
			return types[i];
	}
	return "";
}

bool	Server::_findFile(std::string &path, std::string &fileName)
{
	DIR				*dirp;
	struct dirent	*dp;

	dirp = opendir(path.c_str());
	if (dirp == NULL)
	{
		if (errno == ENOENT)		//	Directory not found
			return false;				//	!!!!!!!!!!!!!!!
		else
			throw std::string ("opendir err");
	}
	while (dirp)
	{
		errno = 0;
		if ((dp = readdir(dirp)) != NULL)
		{
			if (fileName.compare(dp->d_name) == 0)
			{	//	FOUND
				closedir(dirp);
				return true;
            }
        }
		else
		{
			if (errno == 0)
			{	// NOT FOUND
				closedir(dirp);
				return false;
			}
		}
	}
	closedir(dirp);	//	READ ERROR
	return false;
}

void		Server::_methodPost(Client &client)
{
	std::string		url = client.getRequest().getUrl();
	location		location;
	size_t			pos;
	std::string		path = "";
	size_t			locationIndex;

	if (url != "/" && url.back() == '/')
		url.pop_back();
	for (size_t i = 0; i < _locations.size(); ++i)
	{
		location = _locations[i];
		if (location.name != "/" && location.name.back() == '/')
			location.name.pop_back();
		if (location.root.back() == '/')
			location.root.pop_back();
		if ((pos = url.find(location.name)) != std::string::npos)
		{
			if (pos == 0)
			{
				if (location.name > path)
				{
					path = location.name;
					locationIndex = i;
				}
			}
		}
	}
	if (path != "")
	{
		location = _locations[locationIndex];
		if (_isMethodAllow(location, client.getRequest().getMethod()))
			client.setResponseStatus("201 Created");
		else
		{
			client.setResponseStatus("405 Method Not Allowed");
			return ;
		}
		url.replace(0, location.name.length(), location.root);
		if (_checkType(client.getRequest().getUrl()) == ".bla")
		{
			_CGI(client);
			return ;
		}
		if (url == location.root)
		{
			if (location.maxBodySize >= client.getRequest().getBody().size())
				url += "/file";
			else
			{
				client.setResponseStatus("413 Payload Too Large");
				return ;
			}
		}
		std::ofstream 	out;
		// std::cout << "url: " << url << std::endl;
		out.open(url);
		if (!out.is_open())
			throw std::string("open err!!!");
		std::string content = client.getRequest().getBody();
		content = ft_strtrim(content, "0\r\n\r\n");
		out.write(content.c_str(), content.size());
		out.close();
	}
}

bool		Server::_isMethodAllow(const location &loc, const std::string &method)
{
	if (std::find(loc.methods.begin(), loc.methods.end(), method) != loc.methods.end())
		return true;
	return false;
}

void		Server::_methodGet(Client &client)
{
	std::string				url = client.getRequest().getUrl();
	std::string				content("");
	std::string				path("");
	std::ostringstream		ssbuff;
	size_t					pos;
	std::ifstream 			in;
	location				location;

	if (url != "/" && url.back() == '/')
		url.pop_back();
	for (size_t i = 0; i < _locations.size(); ++i)
	{
		location = _locations[i];

		if (location.name != "/" && location.name.back() == '/')
			location.name.pop_back();
		if (location.root.back() == '/')
			location.root.pop_back();
		if ((pos = url.find(location.name)) != std::string::npos)
		{
			if (pos != 0)
				continue ;
			if (url == location.name)
			{
				if (location.autoIndex == true)
					content = _createListing(_root, location.root);
				else
				{
					path = location.root;
					if (path.back() != '/')
						path += "/";
					path += location.index;
					if (path.back() == '/')
						path.pop_back();
				}
				break ;
			}
			else if (url[location.name.size()] == '/')
			{
				url.replace(pos, location.name.length(), location.root);
				break ;
			}
		}
	}
	if (_isMethodAllow(location, client.getRequest().getMethod()))
		client.setResponseStatus("200 OK");
	else if (url != location.name)
		client.setResponseStatus("404 Not Found");
	else
	{
		client.setResponseStatus("405 Method Not Allowed");
		return ;
	}
	if (content.empty() && path.empty())
	{
		if (url == "/")
			content = _makeDefaultPage();
		else
		{
			std::string type = _checkType(url);
			if (type.empty())
			{
				if (url.size() < _root.size())
					path = _errors[404];
				else
				{
					bool res = _findFile(url, location.index);
					if (res == true)
						path = url + "/" + location.index;
					else
						path = _errors[404];
				}
			}
			else
			{
				path = url.substr(0, url.find_last_of('/'));
				std::string file = url.substr(url.find_last_of('/') + 1);
				if (path.empty())
					path = _root;
				if (path.back() != '/')
					path += "/";
				bool res = _findFile(path, file);
				if (res == true)
					path += file;
				else
					path = _errors[404];
			}
		}
	}
	if (path == _errors[404])
		client.setResponseStatus("404 Not Found");
	if (content.empty())
	{
		in.open(path);
		if (!in.is_open())
			throw std::string("open err");
		ssbuff << in.rdbuf();
		in.close();
		content = ssbuff.str();
	}
	client.setResponseContent(content);
}

void		Server::sendResponse(Client &client)
{
	ssize_t				bytesSend = 0;
	std::string			buff(client.getResponse().substr(client.getAllBytesSend()));

	bytesSend = send(client.getSockFd(), buff.c_str(), buff.size(), 0);
	if (bytesSend > 0)
		client.addAllBytesSend(bytesSend);
	else if (bytesSend == -1)
		throw std::string("send err");
	if (client.getAllBytesSend() == client.getResponse().size())
	{
		client.clearRequest();
		client.clearResponse();
		client.setAllBytesSend(0);
		client.setStatus(-1);
	}
}

bool	Server::isClientResponse(const int &itC) { return _clients[itC].isResponse(); }

Server&	Server::operator = (const Server &other)
{
	if (this != &other)
	{
		this->_sockFd = other._sockFd;
		this->_addr = other._addr;
		this->_addrLen = other._addrLen;
		this->_root = other._root;
		this->_errors = other._errors;
		this->_locations = other._locations;
		this->_clients = other._clients;
	}
	return *this;
}

void	Server::_makeCgiEnv(Client &client)
{
	Request request = client.getRequest();

	request = client.getRequest();
	size_t sizeEnv = 14;
	_char_env = (char**)calloc(sizeEnv, sizeof(char*));
	_char_env[0] = ft_strdup("AUTH_TYPE=basic");
	_char_env[1] = ft_strjoin("CONTENT_LENGTH=", std::to_string(request.getBody().size()).c_str());
	_char_env[2] = ft_strjoin("CONTENT_TYPE=", (request.getHeaderByKey("Content-Type").c_str()));
	_char_env[3] = ft_strdup("GATEWAY_INTERFACE=CGI/1.1");
	_char_env[4] = ft_strjoin("PATH_INFO=", request.getUrl().c_str());
	_char_env[5] = ft_strjoin("PATH_TRANSLATED=", "/Users/mismene/ft_webserver/cgi/cgi_tester");
	_char_env[6] = ft_strdup("REMOTE_ADDR=");
	_char_env[7] = ft_strdup("REMOTE_IDENT=");
	_char_env[8] = ft_strdup("REMOTE_USER=");
	_char_env[9] = ft_strjoin("REQUEST_METHOD=", request.getMethod().c_str());
	_char_env[10] = ft_strjoin("REQUEST_URI=http://", (client.getClientInfo() + request.getUrl()).c_str());
	_char_env[11] = ft_strjoin("SERVER_PROTOCOL=", request.getProtocolV().c_str());
	_char_env[12] = ft_strdup("HTTP_X_SECRET_HEADER_FOR_TEST=1");
	_char_env[13] = nullptr;
}

void	Server::_CGI(Client &client)
{
	int			pipeFd[2];
	pid_t		pid;
	ssize_t 	bytesRead;
	ssize_t 	bytesSend;
	char	buff[BUFSIZ];
	std::string	result_buf = "";
	int		status = 0;
	char** _argv = new char*[2];
	_argv[0] = ft_strdup("./cgi/cgi_tester");
	_argv[1] = 0;


	if (pipe(pipeFd) != 0)
		throw std::string("CGI pipe err");
	int	file_fd = open("file", O_CREAT | O_RDWR | O_TRUNC, 0677);
	if ((pid = fork()) == 0)
	{
		dup2(pipeFd[0], 0);
		close(pipeFd[0]);
		dup2(file_fd, 1);
		close(file_fd);
		_makeCgiEnv(client);
		exit(execve(_argv[0], _argv, _char_env));
	}
	else if (pid == -1)
		client.setResponseStatus("500 Internal Server Error");
	else
	{
		bytesSend = write(pipeFd[1], client.getRequest().getBody().c_str(), client.getRequest().getBody().size());
		if (bytesSend == -1)
		{
			std::cerr << strerror(errno) << std::endl;
			throw std::string("CGI send err");
		}
		close(pipeFd[1]);
		close(pipeFd[0]);
		wait(&status);
		if (!status)
		{
			lseek(file_fd, 0, 0);
			while ((bytesRead = read(file_fd, buff, BUFSIZ)) > 0)
			{
				buff[bytesRead] = '\0';
				result_buf += std::string(buff);
			}
			close(file_fd);
			client.setResponseStatus("200 OK");
			client.setResponseContent(std::string(result_buf).substr(std::string(result_buf).find("\r\n\r\n") + 4));
			free(_argv[0]);
			free(_argv);
		}
		else
			client.setResponseStatus("500 Internal Server Error");
	}
}

std::string	Server::getErrorByKey(int key) {return _errors.at(key);}
void		Server::setError(int key, std::string value) {_errors[key] = value;}
void	Server::eraseClient(const int &itC) { _clients.erase(_clients.begin() + itC); }

std::vector<Client>::iterator	Server::disconectUser(const int &itC)
{
	Client			&client = _clients[itC];
	std::cout << "\033[1;35m\t\tclose conection (" << client << ")" << "\033[0m" << std::endl;
	close(client.getSockFd());
	return _clients.erase(_clients.begin() + itC);
}
