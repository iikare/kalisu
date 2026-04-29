#pragma once

#include <string>

#include "build_target.h"
#include "enum.h"
#include "log.h"

using std::string;

#define NAME(v) #v

class asset {
 public:
  asset(ASSET a, const char* b, unsigned char* data, unsigned int dataLen) {
    assetType = a;
    assetName = b;
    this->data = data;
    this->dataLen = dataLen;
  }

  ASSET assetType;
  string assetName;
  unsigned char* dataVS;
  unsigned char* dataFS;
  unsigned char* data;
  unsigned int dataLen;
};
