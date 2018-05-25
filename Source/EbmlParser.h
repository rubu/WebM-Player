#pragma once

#include "Ebml.h"

std::list<EbmlElement> parse_ebml_file(const char* file_path, bool verbose = false);
