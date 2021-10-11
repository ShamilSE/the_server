#pragma once

#include "WebServ.hpp"
#include "../includes/Request.hpp"
#include "../includes/Response.hpp"
class Request;
class Response;

enum	Status
{
	CLOSE_CONECTION,
	NEW_CLIENT,
	WAITING_FOR_RESPONSE,
	CHUNKED,
	PARTIAL_CONTENT,
};

class Client
{
	private:
		int					_sockFd;
		struct sockaddr_in	_addr;
		socklen_t			_addrLen;
		Request				_request;
		Response			_response;
		std::string			_chunke;
		size_t				_chunkeSize;
		std::string			_allChanke;

		int					_status;
		size_t				_allBytesSend;

	public:
		Client();
		Client(const Client&);

		~Client();

		int				getSockFd();
		sockaddr_in&	getAddrRef();
		socklen_t&		getAddrLenRef();
		Request			getRequest();			// !!!!
		std::string		getResponse();
		std::string		getResponseHeader();

		void			setResponseStatus(const std::string &);
		void			setResponseUrl(const std::string &);
		void			setResponseContent(const std::string &);
		void			setResponseLocation(const std::string &);
		void			setSockFd(const int &);
		void			setRequest(const std::string &);
		void			setRequestBody(const std::string &);

		int				getStatus();						//	!!!
		void			setStatus(const int &);	//	!!!
		bool			isRequest() { return _request.isRequest(); }
		bool			isResponse() { return _response.isResponse(); }
		void			clearRequest();
		void			clearRequestBody() { _request.clearBody(); }
		void			clearResponse();
		void			cliarRequestBody() {  }
		void			parseRequestHeader();
		std::string		getHeaderInfo(const std::string &);

		size_t			getChunkeSize() { return _chunkeSize; }
		void			setChunkeSize(const size_t &size) { _chunkeSize = size; }
		void			addChunkePart(const std::string &chunke) { _chunke += chunke; }
		std::string		getChunke() { return _chunke; }
		void			clearChunke() { _chunke.clear(); }

		std::string		getClientInfo();

		void			clearChunkeSize() { _chunkeSize = 0; }
		void			addAllChunke(const std::string &chunke) { _allChanke += chunke; }
		std::string		getAllChunke() { return _allChanke; }
		void			clearAllChunke() { _allChanke.clear(); }

		size_t			getAllBytesSend() { return _allBytesSend; }
		void			setAllBytesSend(const size_t &bytes) { _allBytesSend = bytes; }
		void			addAllBytesSend(const size_t &bytes) { _allBytesSend += bytes; }

		Client&			operator = (const Client &);

		char*			buff;
		size_t			allReadedBytesCount;
};

std::ostream&	operator << (std::ostream &, Client &);
