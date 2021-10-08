#include "../includes/WebServ.hpp"

WebServ::WebServ(const std::string &conf) : _maxFd(-1)
{
	std::ifstream 				in;
	std::string					tmpStr;
	std::stringstream			ssbuff;
	std::istringstream			issbuff;
	std::vector<std::string>	tmpStrVec;

	in.open(conf);
	if (!in.is_open())
	{
		std::cerr << conf + " [open error]" << std::endl;
		exit(1);
	}
	ssbuff << in.rdbuf();
	in.close();
	issbuff.str(ssbuff.str());
	while (getline(issbuff, tmpStr))
	{
		tmpStr = ft_strtrim(tmpStr, " \t");
		if (tmpStr.empty() || tmpStr.front() == '#')
			continue ;
		if (tmpStr == "server:")
		{
			if (!tmpStrVec.empty())
			{
				try
				{
					// _servers.push_back(Server(tmpStrVec));
					_addServer(Server(tmpStrVec));
				}
				catch (const std::string &error)
				{
					std::cerr << error << std::endl;
				}
				tmpStrVec.clear();
			}
			continue ;
		}
		tmpStrVec.push_back(tmpStr);
	}
	try
	{
		// _servers.push_back(Server(tmpStrVec));
		_addServer(Server(tmpStrVec));
	}
	catch (const std::string &error)
	{
		std::cerr << error << std::endl;
	}
	if (_servers.empty())
		exit(1);
	for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++)
	{
		try
		{
			it->run();
		}
		catch (const std::string &error)
		{
			std::cerr << error << std::endl;
			continue ;
		}
		
		try
		{
			it->getErrorByKey(404);
		}
		catch (std::out_of_range &error)
		{
			std::cout << "no error page provided" << std::endl;
			std::cout << "default error page will be used" << std::endl;
			it->setError(404, "./Servers/404.html");
		}
	}
}

WebServ::WebServ(const WebServ &other) { *this = other; }

WebServ::~WebServ()
{
	std::vector<Server>::iterator	itS = _servers.begin();
	for ( ; itS != _servers.end(); ++itS)
	{
		for (size_t itC = 0; itC < itS->getClientsCount(); ++itC)
			close(itS->getClientSockFd(itC));
		close(itS->getSockFd());
	}
}

void	WebServ::_addServer(const Server &serv)
{
	std::string port = serv.getHostPort().second;

	std::vector<Server>::iterator	itS = _servers.begin();
	for ( ; itS != _servers.end(); ++itS)
	{
		if (itS->getHostPort().second == port)
			return ;
	}
	_servers.push_back(serv);
}

void	WebServ::_setFds()
{
	_maxFd = _servers.back().getSockFd();
	FD_ZERO(&_rFds);
	FD_ZERO(&_wFds);
	std::vector<Server>::iterator itS = _servers.begin();
	for ( ; itS != _servers.end(); ++itS)
	{
		FD_SET(itS->getSockFd(), &_rFds);
		for (size_t itC = 0 ; itC != itS->getClientsCount(); ++itC)
		{
			int clientFd = itS->getClientSockFd(itC);
			FD_SET(clientFd, &_rFds);
			if (itS->isClientResponse(itC))
				FD_SET(clientFd, &_wFds);
			if (clientFd > _maxFd)
				_maxFd = clientFd;
		}
	}
}

void	WebServ::_select()
{
	if (select(_maxFd + 1, &_rFds, &_wFds, nullptr, nullptr) < 1)
	{
		if (errno != EINTR)
			throw std::string("select error");
		else
			throw std::string("signal error");	/*	процесс получил обрабатываемый сигнал	*/
	}
}

void	WebServ::mainCycly()
{
	_setFds();
	_select();
	std::vector<Server>::iterator itS = _servers.begin();
	for ( ; itS != _servers.end(); ++itS)
	{
		if (FD_ISSET(itS->getSockFd(), &_rFds))
			itS->acceptNewClient();
		for (size_t itC = 0; itC < itS->getClientsCount(); ++itC)
		{
			Client	&client = itS->getClientRef(itC);
			int		clientFd = client.getSockFd();
			if (FD_ISSET(clientFd, &_rFds))
			{
				itS->readRequest(client);
				if (client.getStatus() == CLOSE_CONECTION)
				{
					client = *itS->disconectUser(itC);
					clientFd = client.getSockFd();
				}
			}
			if (itS->isClientResponse(itC) && FD_ISSET(clientFd, &_wFds))
				itS->sendResponse(client);
		}
	}
}

WebServ&	WebServ::operator = (const WebServ &other)
{
	if (this != &other)
	{
		this->_servers = other._servers;
		this->_maxFd = other._maxFd;
		this->_rFds = other._rFds;
		this->_wFds = other._wFds;
	}
	return *this;
}
