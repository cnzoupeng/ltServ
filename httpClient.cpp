
#include "httpClient.h"
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio/ip/tcp.hpp>
#include "common.h"

int HttpClient::getPage(const char* host, const char* path, string& page)
{
	boost::asio::ip::tcp::iostream httpStream;

	// Establish a connection to the server.
	httpStream.connect(host, "http");
	if (!httpStream)
	{
		//std::cout << "Unable to connect: " << httpStream.error().message()<< "\n";
		std::cout << "Unable to connect: \n";
		return 1;
	}

    // Send the request. We specify the "Connection: close" header so that the
    // server will close the socket after transmitting the response. This will
    // allow us to treat all data up until the EOF as the content.
    httpStream << "GET " << path << " HTTP/1.0\r\n";
    httpStream << "Host: " << host << "\r\n";
    httpStream << "Accept: */*\r\n";
    httpStream << "Connection: close\r\n\r\n";

    // By default, the stream is tied with itself. This means that the stream
    // automatically flush the buffered output before attempting a read. It is
    // not necessary not explicitly flush the stream at this point.

    // Check that response is OK.
    std::string http_version;
    httpStream >> http_version;
    unsigned int status_code;
    httpStream >> status_code;
    std::string status_message;
    std::getline(httpStream, status_message);
    if (!httpStream || http_version.substr(0, 5) != "HTTP/")
    {
      std::cout << "Invalid response\n";
      return 1;
    }
    if (status_code != 200)
    {
      std::cout << "Response returned with status code " << status_code << "\n";
      return 1;
    }

    // Process the response headers, which are terminated by a blank line.
    std::string header;
	while (std::getline(httpStream, header) && header != "\r")
	{
		//std::cout << header << "\n";
	}

    // Write the remaining data to output.
	ostringstream oss;
	oss << httpStream.rdbuf();
	page = oss.str();

	LogDbg("HttpSuccess:%s%s\n", host, path);
	return 0;
}