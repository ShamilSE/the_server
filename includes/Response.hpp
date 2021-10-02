#pragma once

#include <iostream>

class Response
{
	private:
		std::string		_status;
		std::string		_content;
		std::string		_url;

	public:
		Response();
		Response(const Response&);

		~Response();

		void		setStatus(const std::string&);
		void		setContent(const std::string&);
		void		setUrl(const std::string &);


		std::string	getResponse();
		std::string getHeader();

		bool	isResponse();
		void	clearResponse();

		Response&	operator = (const Response&);
};
