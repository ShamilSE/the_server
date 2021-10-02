#include "../includes/Request.hpp"

Request::Request()
	: _request(""), _header(""), _body(""), _method(""), _url(""), _vProtocol("") {}

Request::Request(const std::string &request)
	: _request(request), _header(""), _body(""), _method(""), _url(""), _vProtocol("")
{
	parseHeader();
}

Request::Request(const Request &other) { *this = other; }

Request::~Request() {}

void	Request::readChunked(const std::string &chunk)
{
	_request += chunk;
}

void	Request::clearRequest()
{
	_request.clear();
	_header.clear();	
	_body.clear();
	_method.clear();	
	_url.clear();
	_vProtocol.clear();
	_info.clear();
}

void	Request::readBody(const std::string &buff)
{
	_body += buff;
}

void	Request::parseHeader()
{
	std::vector<std::string>	tmp;
	std::istringstream			issbuff;
	std::string					line;

	tmp = ft_split(_request, "\r\n\r\n");
	if (!tmp.empty())
	{
		_header = tmp[0];
		if (tmp.size() == 2)
			_body = tmp[1];
		issbuff.str(_header);
		getline(issbuff, line);
		tmp = ft_split(line, " ");
		_method = tmp[0];
		_url = tmp[1];
		_vProtocol = ft_strtrim(tmp[2], "\r\n");
		while (getline(issbuff, line))
		{
			tmp = ft_split(line, ": ");
			_info[tmp[0]] = tmp[1];
		}
	}
}

std::string	Request::getRequest() { return this->_request; }

std::string	Request::getUrl() { return this->_url; }

std::string	Request::getMethod() { return _method; }

std::string Request::getHeaderByKey(std::string key)
{
	try
	{
		return _info.at(key);
	}
	catch(const std::out_of_range)
	{
		return "";
	}
}

Request&	Request::operator = (const Request &other)
{
	if (this != &other)
	{
		this->_request = other._request;
		this->_header = other._header;
		this->_body = other._body;
		this->_method = other._method;
		this->_url = other._url;
		this->_vProtocol = other._vProtocol;
		this->_info = other._info;
	}
	return *this;
}
