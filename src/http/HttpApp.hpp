// Copyright 2021 -  Universidad de Costa Rica. CC BY 4.0

#ifndef HTTPAPP_H
#define HTTPAPP_H

#include "common/common.hpp"
#include "prodcons/Assembler.hpp"
#include "common/UtilStruct.hpp"

/**
* @brief Base class for all web applications that can be registered with the
* web server. it inherits from the consumer abstract class
*/
class HttpApp: public Consumer<clientRequest*> {
  /*! \cond PRIVATE */
  /// Web application objects are usually complex. This base class does not
  /// require child classes to allow copying
  DISABLE_COPY(HttpApp);
  /*! \endcond */
 public:
  /// Constructor
  HttpApp(): Consumer<clientRequest*>() {}
  /// Destructor
  ~HttpApp() = default;
  /// Called by the web server when the web server is started
  virtual void start() = 0;
  /// Handle clientRequest
  /// @return true If this application handled the request, false otherwise
  /// and another chained application should handle it
  virtual bool handleClientRequest(clientRequest* request) = 0;
  /// Called when the web server stops, in order to allow the web application
  /// clean up and finish as well
  virtual void stop() = 0;
};

#endif  // HTTPAPP_H
