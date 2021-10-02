#include "../includes/Response.hpp"

Response::Response() : _status(""), _content(""), _url("") {}

Response::Response(const Response &other) { *this = other; }

Response::~Response() {}

std::string	Response::getResponse()
{
	std::string response;

	response = "HTTP/1.1 " + _status;
	// if (!_url.empty())
	// 	response += "\nLocation: " + _url;
	response += "\nConnection: Keep-alive";
	response += "\nStatus: " + _status;
	response += "\nContent-Length: " + std::to_string(_content.size());
	response += "\r\n\r\n" + _content;
	return response;
}

std::string Response::getHeader()
{
	std::string header;

	header = "HTTP/1.1 " + _status;
	// if (!_url.empty())
	// 	header += "\nLocation: " + _url;
	header += "\nConnection: Keep-alive";
	header += "\nStatus: " + _status;
	header += "\nContent-Length: " + std::to_string(_content.size());
	header += "\r\n\r\n<+===CONTENT===+>";
	return header;
}

void	Response::setStatus(const std::string &status) { _status = status; }

void	Response::setUrl(const std::string &url) { _url = url; }

void	Response::setContent(const std::string &content) { _content = content; }

bool	Response::isResponse()
{
	if (!_status.empty() || !_content.empty())
		return true;
	return false;
}

void	Response::clearResponse()
{
	_status.clear();
	_content.clear();
	_url.clear();
}

Response&	Response::operator = (const Response &other)
{
	if (this != &other)
	{
		this->_status = other._status;
		this->_content = other._content;
		this->_url = other._url;
	}
	return *this;
}
