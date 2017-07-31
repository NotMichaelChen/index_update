#ifndef STRLESS_H
#define STRLESS_H

#include <string>

class strless {
   public:
      bool operator() (const std::string & first, const std::string & second ) const;
};

#endif
