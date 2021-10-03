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
		throw std::string("open err");
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
				_servers.push_back(Server(tmpStrVec));
				tmpStrVec.clear();
			}
			continue ;
		}
		tmpStrVec.push_back(tmpStr);
	}
	_servers.push_back(Server(tmpStrVec));
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
			// if (itS->getClient(itC).getStatus() == WAITING_FOR_RESPONSE)
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
			throw std::string("select err");
		else
			throw std::string("signal err");	/*	процесс получил обрабатываемый сигнал	*/
	}
}

void	WebServ::mainCycly()
{
	int	clientFd;

	while(true)
	{
		_setFds();
		_select();
		std::vector<Server>::iterator itS = _servers.begin();
		if (FD_ISSET(itS->getSockFd(), &_rFds))
			itS->acceptNewClient();
		for (size_t itC = 0; itC < itS->getClientsCount(); ++itC)
		{
			clientFd = itS->getClientSockFd(itC);
			if (FD_ISSET(clientFd, &_rFds))
			{
				if (itS->readRequest(itC) == 0)
				{
					break;
				}
			}
			if (itS->isClientResponse(itC) && FD_ISSET(clientFd, &_wFds))
				itS->sendResponse(itC);
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
