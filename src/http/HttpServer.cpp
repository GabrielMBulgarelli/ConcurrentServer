// @copyright C_la_vie 2022
#include <cassert>
#include <stdexcept>
#include <string>

#include "http/HttpApp.hpp"
#include "http/HttpServer.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpRequestHandler.hpp"
#include "http/HttpResponse.hpp"
#include "common/Log.hpp"
#include "network/NetworkAddress.hpp"
#include "network/Socket.hpp"

void signalHandler(int signal) {
  (void)signal;
  HttpServer::getInstance().stop();
  // Stop the server
}

const char* const usage =
  "Usage: webserv [port] [handlers]\n"
  "\n"
  "  port        Network port to listen incoming HTTP requests, default "
    DEFAULT_PORT "\n"
  "  handlers     Number of connection handler theads\n";

HttpServer::HttpServer() {}

HttpServer::~HttpServer() {}

/***
 * @brief makes the server listen forever to the port
 * @param port Port to listen to
*/
void HttpServer::listenForever(const char* port) {
  return TcpServer::listenForever(port);
}

// Overrides the TCP server handle connection for its own purpose
void HttpServer::handleClientConnection(Socket& client) {
  std::cout << "new connection" << std::endl;
  this->socketQueue->push(client);
}

// Adds an application
void HttpServer::chainWebApp(HttpApp* application) {
  assert(application);
  this->applications.push_back(application);
}

int HttpServer::start(int argc, char* argv[]) {
  // Create a queue of client request for the connection handlers
  // and the request handlers
  Queue<clientRequest*>* requestsQueue = new Queue<clientRequest*>();
  bool stopApps = false;
  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);
  try {  // check if in arguments there is a port
    if (this->analyzeArguments(argc, argv)) {
      // Start the log service
      Log::getInstance().start();  // record save in log.txt so
      // we can see what is happening in the server
      // see log.cpp and log.hpp to know more about the log service
      // Create the connection handlers
      this->socketQueue = new Queue<Socket>();
      // Create the connection handlers
      for (size_t locator = 0; locator < this->connectionHandlers; ++locator) {
        HttpConnectionHandler* handler = new HttpConnectionHandler();
        handler->setConsumingQueue(socketQueue);
        handler->setApplications(this->applications);
        handler->setClientRequests(requestsQueue);
        handler->startThread();
        this->handlers.push_back(handler);
      }
      // Start the request dispatcher
      httpRequestHandler = new HttpRequestHandler();
      httpRequestHandler->setConsumingQueue(requestsQueue);
      httpRequestHandler->startThread();
      // Start all web applications
      for (size_t index = 0; index < this->applications.size(); ++index) {
        Queue<clientRequest*>* newQueue = new Queue<clientRequest*>();
        this->applications[index]->setConsumingQueue(newQueue);
        this->applications[index]->startThread();
        httpRequestHandler->registerRedirect(index+1, newQueue);
      }
      stopApps = true;

      // Start waiting for connections
      this->listenForConnections(this->port);  // opening the port to listen for
      const NetworkAddress& address = this->getNetworkAddress();
      Log::append(Log::INFO, "webserver", "Listening on " + address.getIP()
        + " port " + std::to_string(address.getPort()));
      // Accept all client connections
      this->acceptAllConnections();
    }
  } catch (const std::runtime_error& error) {
    std::cerr << error.what() << std::endl;
  }
  // If applications were started
  if (stopApps) {
    // Stop web applications. Give them an opportunity to clean up
    for (size_t i = 0; i <= this->connectionHandlers; ++i) {
      this->socketQueue->push(Socket());
    }
    // Stop all connection handlers
    for (size_t i = 0; i < this->applications.size(); ++i) {
      this->applications[i]->waitToFinish();
      delete applications[i]->getConsumingQueue();
    }
    // Delete handlers
    for (size_t i = 0; i < this->connectionHandlers; ++i) {
      this->handlers[i]->waitToFinish();
      delete this->handlers[i];
    }
    httpRequestHandler->waitToFinish();
  }
  delete requestsQueue;
  delete socketQueue;
  delete httpRequestHandler;
  // Stop the log service
  Log::getInstance().stop();
  return EXIT_SUCCESS;
}

bool HttpServer::analyzeArguments(int argc, char* argv[]) {
  // Traverse all arguments
  for (int index = 1; index < argc; ++index) {
    const std::string argument = argv[index];
    if (argument.find("help") != std::string::npos) {
      std::cout << usage;
      return false;
    }
  }
  if (argc >= 2) {
    this->port = argv[1];
  }
  if (argc >= 3) {
    this->connectionHandlers = std::stoll(argv[2]);
  }
  return true;
}
void HttpServer::stop() {
  /// Stop the server
  this->isClosed = true;
  // Stop listening for incoming client connection requests
  this->stopListening();
}
