#ifndef __NEXT_HTTPCOOKIESESSION_H__
#define __NEXT_HTTPCOOKIESESSION_H__

#include <string>
#include <vector>
#include <list>

#define HTTP_SUCCESS 200

// Contains a single param to be sent in an HTTP request
struct HttpParam{
	std::string name;
	std::string value;
};

// HTTP response is returned in this struct
struct HttpResponse{
	std::string message;
	long code;
	std::list<std::string> cookies;
};

// Establishes a cookie-base HTTP connection
class HttpCookieSession{
public:
	HttpCookieSession(std::string serverUrl);	// constructor
	HttpResponse performHttpGet(std::vector<HttpParam> params);	// sends an HTTP GET message
	HttpResponse performHttpGet(std::string params);	// sends an HTTP GET message
	HttpResponse performHttpPost(std::vector<HttpParam> params);	// sends an HTTP POST message
	HttpResponse performHttpPost(std::string params);	// sends an HTTP POST message
private:
	std::string serverUrl;
	std::list<std::string> sessionCookieList;
};

#endif
