#include "Ebml.h"
#include "EbmlParser.h"
#include "Utilities.h"

#include <memory>
#include <stack>
#include <tuple>
#include <list>
#include <sstream>
#include <ctime>
#include <algorithm>

EbmlDocument::EbmlDocument(std::unique_ptr<unsigned char[]> storage, std::list<EbmlElement> elements) : storage_(std::move(storage)),
    elements_(std::move(elements))
{
}

const std::list<EbmlElement>& EbmlDocument::elements() const
{
    return elements_;
}

enum class EbmlParserState
{
    ParseElementId,
    ParseElementLength,
    ParseElementValue,
};

struct FileDeleter
{
	void operator()(FILE* file)
	{
		fclose(file);
	}
};

#if defined(_WIN32)
EbmlDocument parse_ebml_file(const wchar_t* file_path, bool verbose)
#else
EbmlDocument parse_ebml_file(const char* file_path, bool verbose)
#endif
{
#if defined(_WIN32)
	FILE* file = nullptr;
	_wfopen_s(&file, file_path, L"rb");
	std::unique_ptr<FILE, FileDeleter> ebml_file(file);
#else
    std::unique_ptr<FILE, FileDeleter> ebml_file(fopen(file_path, "rb"));
#endif
    if (ebml_file == nullptr)
    {
#if defined(_WIN32)
        throw std::runtime_error(format_message("could not open file %ls", file_path));
#else
		throw std::runtime_error(format_message("could not open file %s", file_path));
#endif
    }
    fseek(ebml_file.get(), 0L, SEEK_END);
    size_t file_size = ftell(ebml_file.get()), remaining_file_size = file_size;
    fseek(ebml_file.get(), 0L, SEEK_SET);
    std::unique_ptr<unsigned char[]> ebml_file_contents(new unsigned char[file_size]);
    if (fread(ebml_file_contents.get(), 1, file_size, ebml_file.get()) != file_size)
    {
#if defined(_WIN32)
        throw std::runtime_error(format_message("could not read %zu bytes from file %ls", file_size, file_path));
#else
		throw std::runtime_error(format_message("could not read %zu bytes from file %s", file_size, file_path));
#endif
    }
    EbmlParserState ebml_parser_state = EbmlParserState::ParseElementId;
    std::stack<EbmlElement> ebml_element_stack;
    std::stack<uint64_t> ebml_element_size_stack;
    std::list<EbmlElement> ebml_element_tree;
    EbmlElementId current_ebml_element_id = EbmlElementId::EBML;
    EbmlElementType current_ebml_element_type = EbmlElementType::Unknown;
    uint64_t current_ebml_element_size = 0;
    ebml_element_size_stack.push(file_size);
    size_t current_ebml_element_id_length = 0, current_ebml_element_size_length  = 0;
    try
    {
        while (remaining_file_size > 0)
        {
            auto file_offset = file_size - remaining_file_size;
            switch (ebml_parser_state)
            {
            case EbmlParserState::ParseElementId:
            {
                current_ebml_element_id = read_ebml_element_id(&ebml_file_contents[file_offset], remaining_file_size, current_ebml_element_id_length);
                if (ebml_element_size_stack.top() != -1)
                {
                    ebml_element_size_stack.top() -= current_ebml_element_id_length;
                }
                else if (ebml_element_size_stack.top() == -1 && get_ebml_element_level(current_ebml_element_id) == get_ebml_element_level(ebml_element_stack.top().id()))
                {
                    auto element = ebml_element_stack.top();
                    ebml_element_stack.pop();
                    element.calculate_size();
                    ebml_element_size_stack.pop();
                    if (ebml_element_size_stack.top() != -1)
                    {
                        ebml_element_size_stack.top() -= element.size();
                    }
                    ebml_element_stack.top().add_child(element);
                }
                ebml_parser_state = EbmlParserState::ParseElementLength;
            }
            break;
            case EbmlParserState::ParseElementLength:
                current_ebml_element_size = get_ebml_element_size(&ebml_file_contents[file_offset], remaining_file_size, current_ebml_element_size_length);
                remaining_file_size -= current_ebml_element_size_length;
                if (ebml_element_size_stack.top() != -1)
                {
                    ebml_element_size_stack.top() -= current_ebml_element_size_length;
                }
                ebml_parser_state = EbmlParserState::ParseElementValue;
                break;
            case EbmlParserState::ParseElementValue:
                current_ebml_element_type = get_ebml_element_type(current_ebml_element_id);
                if (current_ebml_element_type == EbmlElementType::Master)
                {
                    ebml_element_stack.push(EbmlElement(current_ebml_element_id, current_ebml_element_type, current_ebml_element_size, current_ebml_element_id_length, current_ebml_element_size_length));
                    ebml_element_size_stack.push(current_ebml_element_size);
                }
                else
                {
                    remaining_file_size -= static_cast<size_t>(current_ebml_element_size);
                    if (ebml_element_size_stack.top() != -1)
                    {
                        ebml_element_size_stack.top() -= static_cast<size_t>(current_ebml_element_size);
                    }
                    if (ebml_element_stack.size() > 0)
                    {
                        ebml_element_stack.top().add_child(EbmlElement(current_ebml_element_id, current_ebml_element_type, current_ebml_element_size, current_ebml_element_id_length, current_ebml_element_size_length, &ebml_file_contents[file_offset]));
                        while (ebml_element_size_stack.top() == 0)
                        {
                            auto parent = ebml_element_stack.top();
                            ebml_element_stack.pop();
                            ebml_element_size_stack.pop();
                            if (parent.size() == -1)
                            {
                                parent.calculate_size();
                            }
                            if (ebml_element_size_stack.top() != -1)
                            {
                                ebml_element_size_stack.top() -= parent.size();
                            }
                            if (ebml_element_stack.size() == 0)
                            {
                                ebml_element_tree.push_back(parent);
                                break;
                            }
                            else
                            {
                                ebml_element_stack.top().add_child(parent);
                            }
                        }
                    }
                    else
                    {
                        ebml_element_tree.push_back(EbmlElement(current_ebml_element_id, current_ebml_element_type, current_ebml_element_size, current_ebml_element_id_length, current_ebml_element_size_length, &ebml_file_contents[file_offset]));
                    }
                }
                ebml_parser_state = EbmlParserState::ParseElementId;
                break;
            default:
                throw std::runtime_error("parser state error");
            }
        }
    }
    catch (std::exception& exception)
    {
        std::cerr << "Parsing error at offset 0x" << std::hex << file_size - remaining_file_size << ": " << exception.what() << std::endl;
        return EbmlDocument();
    }
    if (remaining_file_size)
    {
        int i = 3;
    }
    while (ebml_element_stack.size() > 0)
    {
        EbmlElement parent = ebml_element_stack.top();
        ebml_element_stack.pop();
        if (ebml_element_stack.size() == 0)
        {
            ebml_element_tree.push_back(parent);
        }
        else
        {
            ebml_element_stack.top().add_child(parent);
        }
    }
    for (const auto& ebml_element : ebml_element_tree)
    {
        const auto seek_heads = ebml_element.children(EbmlElementId::SeekHead);
        for (const auto& seek_head : seek_heads)
        {
            const auto& seeks =seek_head->children(EbmlElementId::Seek);
            for (const auto& seek : seeks)
            {
                const auto* seek_id = seek->first_child(EbmlElementId::SeekID);
                if (seek_id)
                {
                    const auto* ebml_element_descriptor = get_ebml_element_descriptor(static_cast<EbmlElementId>(std::stoi(seek_id->value())));
                    if (ebml_element_descriptor)
                    {
                    }
                }
            }
        }
    }
    if (verbose)
    {
        std::cout << std::endl << "*** EBML element tree ***" << std::endl;
        for (EbmlElement& ebml_element : ebml_element_tree)
        {
            ebml_element.print(0);
        }
        std::cout << std::endl;
    }
    return EbmlDocument(std::move(ebml_file_contents), std::move(ebml_element_tree));
}
