#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include "utils.hpp"

class Request
{
	private:
		std::string							_request;
		std::string							_header;
		std::string							_body;
		std::string							_method;
		std::string							_url;
		std::string							_vProtocol;
		std::map<std::string, std::string>	_info;

		// void	_parseRequest();

	public:
		Request();
		Request(const std::string&);
		Request(const Request&);

		~Request();

		std::string getUrl();
		std::string	getRequest();		// !!!
		std::string	getMethod();		// !!!
		std::string getHeaderByKey(std::string);
		bool		isRequest() { return !_request.empty(); }
		void		readChunked(const std::string &);
		void		parseHeader();
		void		readBody(const std::string &);
		void		clearRequest();
		void		clearBody() { _body.clear(); }
		std::string	getBody() { return _body; }
		void		setBody(const std::string &body) { _body = body; }
		std::string	getProtocolV() { return _vProtocol; }

		Request&	operator = (const Request&);
};
