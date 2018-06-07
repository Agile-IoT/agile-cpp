#include "httpcookiesession.h"

#include <cstring>
#include <sstream>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>

// Class to get the string returned in an HTTP response with curlpp
class WriterMemoryClass{
	char *m_pBuffer = NULL;
	size_t m_Size = 0;
	void* Realloc(void* ptr, size_t size) {if(ptr) return realloc(ptr, size); else return malloc(size);}
public:
	size_t WriteMemoryCallback(char* ptr, size_t size, size_t nmemb);	// callback function that writes the HTTP response into memory
	std::string getResponse() {if(m_pBuffer) return std::string(m_pBuffer); else return "";}	// returns the HTTP response string
};

// Callback function that (re)allocates memory and adds the new data to the end of the buffer
size_t WriterMemoryClass::WriteMemoryCallback(char* ptr, size_t size, size_t nmemb){
	// Calculate the real size of the incoming buffer
	size_t realsize = size * nmemb;
	// (Re)Allocate memory for the buffer
	m_pBuffer = (char*) Realloc(m_pBuffer, m_Size + realsize + 1);
	// Test if the buffer is initialized correctly & copy memory
	if (m_pBuffer == NULL)
		return 0;
	std::memcpy(&(m_pBuffer[m_Size]), ptr, realsize);
	m_Size += realsize;
	m_pBuffer[m_Size] = '\0';
	// Return the real size of the buffer
	return realsize;
}

// Constructor that sets the server URL
HttpCookieSession::HttpCookieSession(std::string serverUrl) : serverUrl(serverUrl){
	try{
		// Do cleanup of used resources (RAII style)
		curlpp::Cleanup myCleanup;
	}
	catch(cURLpp::RuntimeError &e){
		std::cout << e.what() << std::endl;
	}
	catch(cURLpp::LogicError &e){
		std::cout << e.what() << std::endl;
	}
}

// Sends an HTTP GET message appending parameters passed as parameter to the server URL
HttpResponse HttpCookieSession::performHttpGet(std::vector<HttpParam> params){
	std::stringstream paramsStream;
	if(params.size() > 0){
		std::vector<HttpParam>::iterator it = params.begin();
		paramsStream << "?" << it->name << "=" << it->value;
		it++;
		for (; it != params.end(); it++)
			paramsStream << "&" << it->name << "=" << it->value;
	}

	return performHttpGet(paramsStream.str());
}

// Sends an HTTP GET message appending the string passed as parameter to the server URL
HttpResponse HttpCookieSession::performHttpGet(std::string params){
	HttpResponse* getResponse;

	try{
		curlpp::Easy getRequest;
		// Set the URL
		getRequest.setOpt(new curlpp::options::Url(serverUrl + params));
		// Enable the cookie engine (if not, we don't get the set-cookie field from the server)
		getRequest.setOpt(new curlpp::options::CookieJar(""));
		// Include the session cookies
		for(std::list<std::string>::iterator it = sessionCookieList.begin(); it != sessionCookieList.end(); it++)
			getRequest.setOpt(new curlpp::options::CookieList(*it));
		// Set the object that will process the HTTP response
		WriterMemoryClass respWriter;
		curlpp::types::WriteFunctionFunctor writerFunctor = std::bind(&WriterMemoryClass::WriteMemoryCallback,
				&respWriter, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);	// the callback function must be static
		getRequest.setOpt(new curlpp::options::WriteFunction(writerFunctor));
		// Perform the HTTP request
		getRequest.perform();
		// Update the session cookies
		sessionCookieList = curlpp::infos::CookieList::get(getRequest);
		// Built the HTTP response struct
		getResponse = new HttpResponse{respWriter.getResponse(),curlpp::infos::ResponseCode::get(getRequest),curlpp::infos::CookieList::get(getRequest)};
	}
	catch(cURLpp::RuntimeError &e){
		std::cout << e.what() << std::endl;
	}
	catch(cURLpp::LogicError &e){
		std::cout << e.what() << std::endl;
	}

	return *getResponse;
};

// Sends an HTTP POST message with the parameters passed as parameter
HttpResponse HttpCookieSession::performHttpPost(std::vector<HttpParam> params){
	std::stringstream paramsStream;
	if(params.size() > 0){
		std::vector<HttpParam>::iterator it = params.begin();
		paramsStream << it->name << "=" << it->value;
		it++;
		for (; it != params.end(); it++)
			paramsStream << "&" << it->name << "=" << it->value;
	}

	return performHttpPost(paramsStream.str());

}

// Sends an HTTP POST message with the string passed as parameter
HttpResponse HttpCookieSession::performHttpPost(std::string params){
	HttpResponse* postResponse;

	try{
		curlpp::Easy postRequest;
		// Set the URL
		postRequest.setOpt(new curlpp::options::Url(serverUrl));
		// Set the HTTP message as POST and include the parameters
		postRequest.setOpt(new curlpp::options::PostFields(params));
		// Enable the cookie engine (if not, we don't get the set-cookie field from the server)
		postRequest.setOpt(new curlpp::options::CookieJar(""));
		// Include the session cookies
		for(std::list<std::string>::iterator it = sessionCookieList.begin(); it != sessionCookieList.end(); it++)
			postRequest.setOpt(new curlpp::options::CookieList(*it));
		// Set the object that will process the HTTP response
		WriterMemoryClass respWriter;
		curlpp::types::WriteFunctionFunctor writerFunctor = std::bind(&WriterMemoryClass::WriteMemoryCallback,
				&respWriter, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);	// the callback function must be static
		postRequest.setOpt(new curlpp::options::WriteFunction(writerFunctor));
		// Perform the HTTP request
		postRequest.perform();
		// Update the session cookies
		sessionCookieList = curlpp::infos::CookieList::get(postRequest);
		// Built the HTTP response struct
		postResponse = new HttpResponse{respWriter.getResponse(),curlpp::infos::ResponseCode::get(postRequest),curlpp::infos::CookieList::get(postRequest)};
	}
	catch(cURLpp::RuntimeError &e){
		std::cout << e.what() << std::endl;
	}
	catch(cURLpp::LogicError &e){
		std::cout << e.what() << std::endl;
	}

	return *postResponse;

}
