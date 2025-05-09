// Copyright 2021 -  Universidad de Costa Rica. CC BY 4.0

#include <cassert>
#include <stdexcept>
#include <string>

#include "http/HttpMessage.hpp"
#include "network/NetworkAddress.hpp"

const char* const HttpMessage::lineSeparator = "\r\n";

HttpMessage::HttpMessage(const Socket& socket)
  : socket{socket}
  , sharedBody{new std::stringstream()} {
}

HttpMessage::~HttpMessage() {
}

Socket& HttpMessage::getSocket() {
  return this->socket;
}

NetworkAddress HttpMessage::getNetworkAddress() const {
  return this->socket.getNetworkAddress();
}

std::string HttpMessage::getHeader(const std::string& key
  , const std::string& defaultvalue) {
  const Headers::const_iterator& itr = this->headers.find(key);
  return itr != this->headers.end() ? itr->second : defaultvalue;
}

std::string HttpMessage::guessContentType() const {
  // This is a naive incomplete implementation. See file signatures for a more
  // exhaustive scope: https://en.wikipedia.org/wiki/List_of_file_signatures
  const std::string& content = this->body().str();

  // If body is empty, no way to guess
  if (content.empty()) {
    return "";
  }

  // If body starts with "<"
  if (content[0] == '<') {
    return "text/html";
  }

  // If body starts with printable character
  if (::isgraph(content[0])) {
    return "text/plain";
  }

  // Binary data
  return "application/octet-stream";
}

bool HttpMessage::operator==(const HttpMessage& other) const {
  return this->sharedBody == other.sharedBody;
}
