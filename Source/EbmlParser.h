#pragma once

#include "Ebml.h"

#include <memory>

class EbmlDocument
{
public:
    EbmlDocument(std::unique_ptr<unsigned char[]> storage = nullptr, std::list<EbmlElement> elements = std::list<EbmlElement>());

    const std::list<EbmlElement>& elements() const;

private:
    std::unique_ptr<unsigned char[]> storage_;
    std::list<EbmlElement> elements_;
};

#if defined(_WIN32)
EbmlDocument parse_ebml_file(const wchar_t* file_path, bool verbose = false);
#else
EbmlDocument parse_ebml_file(const char* file_path, bool verbose = false);
#endif