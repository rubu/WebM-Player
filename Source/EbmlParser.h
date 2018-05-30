#pragma once

#include "Ebml.h"

class EbmlDocument
{
public:
    EbmlDocument(std::unique_ptr<unsigned char[]> storage = nullptr, std::list<EbmlElement> elements = std::list<EbmlElement>());

    const std::list<EbmlElement>& elements();

private:
    std::unique_ptr<unsigned char[]> storage_;
    std::list<EbmlElement> elements_;
};

EbmlDocument parse_ebml_file(const char* file_path, bool verbose = false);
