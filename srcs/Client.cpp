#include "../includes/Client.hpp"

Client::Client() : _sockFd(-1), _addrLen(sizeof(sockaddr_in)), _request(Request()), _chunke(""), _chunkeSize(0), _allBytesSend(0)
{
	bzero(&_chunkeSize, sizeof(_chunkeSize));
	bzero(&_addr, _addrLen);
}

Client::Client(const Client &other) { *this = other; }

Client::~Client() {}

void	Client::setSockFd(const int &fd) { this->_sockFd = fd; }

void	Client::setRequest(const std::string &req) { this->_request.readChunked(req); }

void	Client::setRequestBody(const std::string &buff) { this->_request.readBody(buff); }

void	Client::setResponseStatus(const std::string &status) { _response.setStatus(status); }

void	Client::setResponseUrl(const std::string &url) { _response.setUrl(url); }

void	Client::setResponseContent(const std::string &content) { _response.setContent(content); }

int		Client::getSockFd() { return this->_sockFd; }

int		Client::getStatus() { return _status; }						//	!!!
void	Client::setStatus(const int &status) { _status = status; }	//	!!!

void			Client::setResponseLocation(const std::string &location) { _response.setLocation(location); }

sockaddr_in&	Client::getAddrRef() { return this->_addr; }

socklen_t&		Client::getAddrLenRef() { return this->_addrLen; }

Request	Client::getRequest() { return this->_request; }

void	Client::clearRequest() { _request.clearRequest(); }

void	Client::clearResponse() { _response.clearResponse(); }

void	Client::parseRequestHeader() { _request.parseHeader(); }

std::string		Client::getHeaderInfo(const std::string &key)
{
	return _request.getHeaderByKey(key);
}

std::string		Client::getResponseHeader() { return _response.getHeader(); }

std::string	Client::getResponse() { return this->_response.getResponse(); }

Client&			Client::operator = (const Client &other)
{
	if (this != &other)
	{
		this->_sockFd = other._sockFd;
		this->_addr = other._addr;
		this->_addrLen = other._addrLen;
		this->_request = other._request;
		this->_response = other._response;
		this->_chunke = other._chunke;
		this->_chunkeSize = other._chunkeSize;
		this->_allChanke = other._allChanke;
		this->_status = other._status;
		this->_allBytesSend = other._allBytesSend;
	}
	return *this;
}

std::string Client::getClientInfo()
{
	char client_ipv4_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &getAddrRef().sin_addr, client_ipv4_str, INET_ADDRSTRLEN);
	std::string res(client_ipv4_str);

	res += ":";
	res += std::to_string(ntohs(getAddrRef().sin_port));

	return res;
}

std::ostream&	operator << (std::ostream &out, Client &client)
{
	char client_ipv4_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client.getAddrRef().sin_addr, client_ipv4_str, INET_ADDRSTRLEN);

	out << client_ipv4_str << ":" << ntohs(client.getAddrRef().sin_port);

	return out;
}
