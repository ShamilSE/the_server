#include "../includes/Server.hpp"
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <iostream>

Server::Server(const std::vector<std::string> &conf) : _sockFd(-1), _root(""), _index(""), _autoIndex(false), _envCount(0), _env(nullptr)
{
	_errors[404] = _errors[502] = "default error";
	for (size_t i = 0; i < conf.size(); i++)
	{
		if (conf[i].find("host:") != std::string::npos)
			_host_port.first = ft_strtrim(conf[i].substr(5), " \t");
		else if (conf[i].find("listen:") != std::string::npos)
			_host_port.second = ft_strtrim(conf[i].substr(7), " \t");
		else if (conf[i].find("index:") != std::string::npos)
		{
			if (conf[i].find("auto_index:") != std::string::npos)
			{
				std::string	tmpStr = conf[i].substr(11);
				tmpStr = ft_strtrim(tmpStr, " \t");
				_autoIndex = (tmpStr == "on" ? true : false);
			}
			else
				_index	= ft_strtrim(conf[i].substr(6), " \t");
		}
		else if (conf[i].find("root:") != std::string::npos)
			_root = ft_strtrim(conf[i].substr(5), " \t");
		else if (conf[i].find("server_name:") != std::string::npos)
			_servername = ft_strtrim(conf[i].substr(12), " \t");
		else if (conf[i].find("error_page:") != std::string::npos)
		{
			std::string					tmpStr = ft_strtrim(conf[i].substr(11), " \t");
			std::vector<std::string>	tmpStrVec = ft_split(tmpStr, " ");
			_errors[std::atoi(tmpStrVec[0].c_str())] = tmpStrVec[1];
		}
		else if (conf[i].find("location:") != std::string::npos)
			i = _parseLocation(conf, i);
	}
}

Server::Server(const Server &other) { *this = other; }

Server::~Server() {}

void	Server::run()
{
	int reuseOpt = 1;
	// bzero(&_addr, _addrLen);
	inet_aton(_host_port.first.c_str(), &_addr.sin_addr);
	_addr.sin_port = htons(stoi(_host_port.second));
	_addr.sin_family = AF_INET;
	_addrLen = sizeof(_addr);
	_sockFd = socket(AF_INET, SOCK_STREAM, 0);	/*	создание сокета	*/
	if (_sockFd == -1)
		throw std::string("socket error");
	fcntl(_sockFd, F_SETFL, O_NONBLOCK);	/*	перевод сокета в неблокирующий режим	*/
	if (setsockopt(_sockFd, SOL_SOCKET, SO_REUSEADDR, &reuseOpt, sizeof(reuseOpt)) == -1)	/*	повторное использование порта	*/
		throw std::string("setsockopt error");
	if (bind(_sockFd, (sockaddr*)&_addr, _addrLen) == -1)	/*	привязка сокета к адресу	*/
		throw std::string("bind error");
	if (listen(_sockFd, 128) == -1)	/*	перевод сокета в слушающий режим	*/
		throw std::string("listen error");
	std::cout << "\033[1;35m\t\tserver: " << htons(_addr.sin_port) << " listening\033[0m" << std::endl;
}

size_t	Server::_parseLocation(const std::vector<std::string> &conf, size_t i)
{
	if (i != conf.size())
	{
		std::string	tmpStr;
		location	loc;
		loc.root = _root;
		loc.index = _index;
		loc.cgi.first = "";
		loc.cgi.second = "";
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
			else if (conf[i].find("index:") != std::string::npos)
			{
				tmpStr = conf[i].substr(6);
				loc.index = ft_strtrim(tmpStr, " \t");
			}
			else if (conf[i].find("root:") != std::string::npos)
			{
				tmpStr = conf[i].substr(5);
				loc.root = ft_strtrim(tmpStr, " \t");
			}
			else if (conf[i].find("max_body_size:") != std::string::npos)
			{
				tmpStr = conf[i].substr(14);
				tmpStr = ft_strtrim(tmpStr, " \t");
				loc.maxBodySize = atoi(tmpStr.c_str());
			}
			else if (conf[i].find("redir:") != std::string::npos)
			{
				loc.root = "";
				loc.index = "";
				std::vector<std::string>	tmpStrVec;
				tmpStr = conf[i].substr(6);
				tmpStr = ft_strtrim(tmpStr, " \t");
				tmpStrVec = ft_split(tmpStr, " ");
				loc.redir = std::make_pair(std::atoi(tmpStrVec[0].c_str()), tmpStrVec[1]);
			}
			else if (conf[i].find("cgi_path:") != std::string::npos)
			{
				std::vector<std::string>	tmpStrVec;
				tmpStr = conf[i].substr(9);
				tmpStr = ft_strtrim(tmpStr, " \t");
				tmpStrVec = ft_split(tmpStr, " ");
				loc.cgi = std::make_pair(tmpStrVec[0], tmpStrVec[1]);
			}
		}
		_locations.push_back(loc);
		return _parseLocation(conf, i);
	}
	return i;
}

int	Server::getSockFd() { return _sockFd; }

Client&	Server::getClientRef(const int &itC) { return _clients[itC]; }

unsigned int	Server::getClientsCount() { return _clients.size(); }

int	Server::getClientSockFd(const int &itC) { return _clients[itC].getSockFd(); }

void	Server::acceptNewClient()
{
	Client	newClient;
	int		clientFd;

	clientFd = accept(_sockFd, (sockaddr*)&newClient.getAddrRef(), &newClient.getAddrLenRef());
	if (clientFd == -1)
		throw std::string("accept new client error");
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

void	Server::_readChunke(Client &client)
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
				return ;
			else
			{
				std::string content = client.getChunke();
				if ((content.find("\r\n")) != std::string::npos)
				{
					if (content[0] == '0')
						return ;
					client.setChunkeSize(std::strtol(content.c_str(), NULL, 16));
					client.clearChunke();
				}
			}
		}
		else if (bytesRead == 0)
			client.setStatus(CLOSE_CONECTION);
		else if (bytesRead == -1 )
			throw std::string("recv chunke size error");
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
				return ;
			}
			client.addChunkePart(buff);
			if (client.getChunke().size() == client.getChunkeSize())
			{
				client.addAllChunke(client.getChunke());
				client.clearChunke();
				client.clearChunkeSize();
			}
		}
		else if (bytesRead == 0)
			client.setStatus(CLOSE_CONECTION);
		else if (bytesRead == -1 )
			throw std::string("recv chunke error");
	}
}

void	Server::readRequest(Client &client)
{
	ssize_t	buffer_size = 0;
	int		bytesRead = 0;

	if (client.getStatus() == CHUNKED)
		_readChunke(client);
	else if (client.getStatus() == PARTIAL_CONTENT)
	{
		buffer_size = BUFSIZ;
		char	buff[buffer_size];

		while (client.allReadedBytesCount < std::stoul(client.getRequest().getHeaderByKey("Content-Length")))
		{
			bytesRead = recv(client.getSockFd(), buff, buffer_size - 1, 0);
			if (bytesRead > 0)
			{
				ft_add(client.buff, buff, bytesRead, client.allReadedBytesCount);
				client.allReadedBytesCount += bytesRead;
			}
			else if (bytesRead == 0)
				client.setStatus(CLOSE_CONECTION);
		}
		client.setStatus(WAITING_FOR_RESPONSE);
		makeClientResponse(client);
	}
	else
	{
		buffer_size = 2;
		char	buff[buffer_size];
		bytesRead = recv(client.getSockFd(), buff, buffer_size - 1, 0);
		if (bytesRead > 0)
		{
			buff[bytesRead] = '\0';
			client.setRequest(buff);
		}
		else if (bytesRead == 0)
			client.setStatus(CLOSE_CONECTION);
		else if (bytesRead == -1)
			throw std::string("recv request header error");
		std::string requestHeader = client.getRequest().getRequest();
		if (requestHeader.find("\r\n\r\n") != std::string::npos)
		{
			std::cout << "\t\tREQUEST\n";
			std::cout << client.getRequest().getRequest() << std::endl;
			client.parseRequestHeader();
			if (client.getRequest().getHeaderByKey("Content-Length") != "")
				client.setStatus(PARTIAL_CONTENT);
			else if (client.getHeaderInfo("Transfer-Encoding").compare(0, 7, "chunked") == 0)
				client.setStatus(CHUNKED);
			else
				makeClientResponse(client);
		}
	}
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
	client.setStatus(WAITING_FOR_RESPONSE);
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


std::string	Server::_createListing(std::string &path)
{
	std::string		serverRoot = _root;
	DIR				*dirPtr;
	struct dirent	*s_dirent;
	std::string		content;
	std::string		tmp;

    dirPtr = opendir(path.c_str());
	if (dirPtr == NULL)
	{
		if (errno == ENOENT)		//	Directory not found
			return "";
		else
			throw std::string (path + " [opendir error]");
	}
	if (path.back() != '/')
		path.append("/");
	if (serverRoot.back() != '/')
		serverRoot.append("/");
	_createListingStart(content);
	while ((s_dirent = readdir(dirPtr)) != NULL)
	{
		tmp = path + s_dirent->d_name;
		if (path == serverRoot && ( !strcmp(s_dirent->d_name, ".") || !strcmp(s_dirent->d_name, "..")))
			continue ;
		_addRefToListing(content, tmp, s_dirent->d_name);
	}
	if (errno == EBADF)
		throw std::string (path + " [readdir error]");
	if (closedir(dirPtr) == -1)
		throw std::string (path + " [closedir error]");
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
	page += "\t\t<h1>Welcome to <a href=\"https://github.com/aquinoa-nba\">Aquinoa</a> & <a href=\"https://github.com/ShamilSE\">Mismene</a> server!</h1>\n";
	page += "\t\t<p>If you see this page, the web server is successfully installed and\n";
	page += "\t\tworking. Further configuration is required.</p>\n";
	page += "\t\t<p><em>Thank you for using this server.</em></p>\n";
	page += "\t</body>\n";
	page += "</html>\n";
	return page;
}

std::string Server::_makeDefaultErrorPage(const std::string &errorStr)
{
	std::string	error_page;
	error_page = "<!DOCTYPE html>\n";
	error_page += "<html>\n";
	error_page += "\t<head><title>" + errorStr +"</title></head>\n";
	error_page += "\t<body bgcolor=\"white\">\n";
	error_page += "\t\t<center><h1>" + errorStr + "</h1></center>\n";
	error_page += "\t\t<hr><center><a href=\"https://github.com/aquinoa-nba\">Aquinoa</a> & <a href=\"https://github.com/ShamilSE\">Mismene</a> server</center>\n";
	error_page += "\t</body>\n";
	error_page += "</html>\n";
	return error_page;
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

bool	Server::_findFile(const std::string &path, const std::string &filename)
{
	DIR				*dirPtr;
	struct dirent	*s_dirent;

	dirPtr = opendir(path.c_str());
	if (dirPtr == NULL)
	{
		if (errno == ENOENT)		//	Directory not found
			return false;
		else
			throw std::string (path + " [opendir error]");
	}
	while ((s_dirent = readdir(dirPtr)) != NULL)
	{
        if (filename.compare(s_dirent->d_name) == 0)	//	file found
        {
            closedir(dirPtr);
            return true;
        }
	}
	if (errno == EBADF)
		throw std::string (path + "/" + filename + " [readdir error]");
	if (closedir(dirPtr) == -1)
		throw std::string (path + " [closedir error]");
	return false;		//	file not found
}

void	Server::_boundaryHandler(std::string &boundary, Client& client)
{
	boundary = boundary.substr(boundary.find("boundary=") + 9);
	boundary = ft_strtrim(boundary, "-\r\n");

	std::string body(client.buff, client.allReadedBytesCount);

	body = ft_strtrim(body, "-\r\n");
	body = body.substr(body.find_first_not_of(boundary), body.find_last_not_of(boundary) - body.find_first_not_of(boundary));
	body = ft_strtrim(body, "-\r\n");

	std::vector<std::string>	splitBody = ft_split(body, "\r\n\r\n");

	std::string header = splitBody[0];
	body = splitBody[1];
	std::string filename = header.substr(header.find("filename=") + 9);
	filename = filename.substr(0, filename.find("\r"));
	filename = ft_strtrim(filename, "\"");

	std::ofstream outfile(filename, std::ios::binary);
	outfile.write(body.c_str(), body.length());
	outfile.close();
	free(client.buff);
	client.buff = NULL;
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
		std::string boundary = client.getHeaderInfo("Content-Type");
		if (boundary != "")
		{
			_boundaryHandler(boundary, client);
			client.allReadedBytesCount = 0;
			return ;
		}
		if (!location.cgi.first.empty() && _checkType(client.getRequest().getUrl()) == location.cgi.first)
		{
			_CGI(client, location.cgi.second);
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

std::string		Server::_makeAutoindex(Client &client)
{
	std::string		url = client.getRequest().getUrl();
	size_t			maxLocnameLen = 0;
	std::string		content = "";
	location		location;
	size_t			pos;

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
			if (pos != 0 || maxLocnameLen > location.name.length())
				continue ;
			maxLocnameLen = location.name.length();
			if (url == location.name)
				content = _createListing(location.root);
			else if (location.name != "/")
			{
				url.replace(0, location.name.length(), location.root);
				content = _createListing(url);
			}
		}
	}
	if (content.empty())
	{
		if (url.find(_root) != std::string::npos)
			content = _createListing(url);
	}
	return content;
}

void		Server::_methodGet(Client &client)
{
	std::string				url = client.getRequest().getUrl();
	size_t					maxLocnameLen = 0;
	std::string				content("");
	int						locId = -1;
	std::string				path("");
	std::ostringstream		ssbuff;
	location				loc;
	size_t					pos;
	std::ifstream 			in;

	if (_root.empty())
		content = _makeDefaultPage();
	else if (_autoIndex == true && _checkType(url) == "")
	{
		content = _makeAutoindex(client);
		if (content.empty())
			path = _errors[404];
		else
			client.setResponseStatus("200 OK");
	}
	else
	{
		if (url != "/" && url.back() == '/')
			url.pop_back();
		std::vector<location>::iterator	itLoc = _locations.begin();
		for ( ; itLoc != _locations.end(); ++itLoc)
		{
			loc = *itLoc;
			if (loc.name != "/" && loc.name.back() == '/')
				loc.name.pop_back();
			if (loc.root.back() == '/')
				loc.root.pop_back();
			if ((pos = url.find(loc.name)) != std::string::npos)
			{
				if (pos != 0)
					continue ;
				if (url == loc.name)
				{
					path = loc.root;
					if (path.back() != '/')
						path += "/";
					path += loc.index;
					if (maxLocnameLen < loc.name.length())
					{
						maxLocnameLen = loc.name.length();
						locId = itLoc - _locations.begin();
					}
				}
				else if (url[loc.name.size()] == '/')
				{
					url.replace(pos, loc.name.length(), loc.root);
					if (maxLocnameLen < loc.name.length())
					{
						maxLocnameLen = loc.name.length();
						locId = itLoc - _locations.begin();
					}
				}
			}
		}
		if (locId != -1)
		{
			loc = _locations[locId];
			if (!loc.redir.second.empty())
			{
				client.setResponseStatus("302 Found");
				client.setResponseLocation(loc.redir.second);
				return ;
			}
			if (_isMethodAllow(loc, client.getRequest().getMethod()))
				client.setResponseStatus("200 OK");
			else
			{
				client.setResponseStatus("405 Method Not Allowed");
				return ;
			}
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
						bool res = _findFile(url, loc.index);
						if (res == true)
							path = url + "/" + loc.index;
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
	}
	if (path == _errors[404])
		client.setResponseStatus("404 Not Found");
	if (content.empty())
	{
		if (path == "default error")
			content = _makeDefaultErrorPage("404 Not Found");
		else
		{
			in.open(path);
			if (!in.is_open())
				throw std::string(path + " [open error]");
			ssbuff << in.rdbuf();
			in.close();
			content = ssbuff.str();
		}
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
		throw std::string("send response error");
	if (client.getAllBytesSend() == client.getResponse().size())
	{
		std::cout << "\t\tRESPONSE\n";
		std::cout << client.getResponse() << std::endl;
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
		this->_host_port = other._host_port;
		this->_sockFd = other._sockFd;
		this->_addr = other._addr;
		this->_addrLen = other._addrLen;
		this->_root = other._root;
		this->_index = other._index;
		this->_autoIndex = other._autoIndex;
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
	_envCount = 14;
	_env = (char**)calloc(_envCount, sizeof(char*));
	_env[0] = ft_strdup("AUTH_TYPE=basic");
	_env[1] = ft_strjoin("CONTENT_LENGTH=", std::to_string(request.getBody().size()).c_str());
	_env[2] = ft_strjoin("CONTENT_TYPE=", (request.getHeaderByKey("Content-Type").c_str()));
	_env[3] = ft_strdup("GATEWAY_INTERFACE=CGI/1.1");
	_env[4] = ft_strjoin("PATH_INFO=", request.getUrl().c_str());
	_env[5] = ft_strjoin("PATH_TRANSLATED=", "/Users/mismene/ft_webserver/cgi/cgi_tester");
	_env[6] = ft_strdup("REMOTE_ADDR=");
	_env[7] = ft_strdup("REMOTE_IDENT=");
	_env[8] = ft_strdup("REMOTE_USER=");
	_env[9] = ft_strjoin("REQUEST_METHOD=", request.getMethod().c_str());
	_env[10] = ft_strjoin("REQUEST_URI=http://", (client.getClientInfo() + request.getUrl()).c_str());
	_env[11] = ft_strjoin("SERVER_PROTOCOL=", request.getProtocolV().c_str());
	_env[12] = ft_strdup("HTTP_X_SECRET_HEADER_FOR_TEST=1");
	_env[13] = nullptr;
}

void	Server::_CGI(Client &client, const std::string &path)
{
	int			pipeFd[2];
	pid_t		pid;
	ssize_t 	bytesRead;
	ssize_t 	bytesSend;
	char		buff[BUFSIZ];
	std::string	result_buf = "";
	int			status = 0;
	char**		_argv = new char*[2];
	_argv[0] = ft_strdup(path.c_str());
	_argv[1] = 0;

	if (pipe(pipeFd) != 0)
	{
		client.setResponseStatus("500 Internal Server Error");
		throw std::string("CGI pipe error");
	}
	int	file_fd = open("CGI_file", O_CREAT | O_RDWR | O_TRUNC, 0677);
	if ((pid = fork()) == 0)
	{
		dup2(pipeFd[0], 0);
		close(pipeFd[0]);
		dup2(file_fd, 1);
		close(file_fd);
		_makeCgiEnv(client);
		exit(execve(_argv[0], _argv, _env));
	}
	else if (pid == -1)
		client.setResponseStatus("500 Internal Server Error");
	else
	{
		bytesSend = write(pipeFd[1], client.getRequest().getBody().c_str(), client.getRequest().getBody().size());
		if (bytesSend == -1)
		{
			client.setResponseStatus("500 Internal Server Error");
			throw std::string("CGI write error");
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
			if (bytesRead == -1)
			{
				client.setResponseStatus("500 Internal Server Error");
				throw std::string("CGI read error");
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

void Server::ft_add(char *&dst, char *buf, size_t buf_size, size_t dst_size) {
	size_t i = 0;

	size_t new_dst_size = (dst_size + buf_size);
	char *_realloc_body = dst;
	dst = (char *)malloc(new_dst_size);
	if (dst_size)
	{
		ft_memcpy(dst, _realloc_body, dst_size);
		free(_realloc_body);
		i = dst_size;
	}
	for (size_t it = 0 ; i < new_dst_size; ++i, ++it)
		dst[i] = buf[it];
	dst[i] = '\0';
}
