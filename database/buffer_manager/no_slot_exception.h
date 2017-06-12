#pragma once

#include <exception>
#include <stdexcept>

class NoSlotException : public std::runtime_error {

  public:

  NoSlotException() : std::runtime_error("No page slot available in RAM") {}

  virtual const char* what() const throw() {
    return std::runtime_error::what();
  }
};
