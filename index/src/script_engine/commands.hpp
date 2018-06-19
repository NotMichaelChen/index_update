#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <memory>

#include "index.hpp"
#include "document_readers/reader_interface.hpp"
#include "document_readers/RAWreader.hpp"
#include "document_readers/WETreader.hpp"

void commandInsert(std::unique_ptr<Index>& indexptr, std::unique_ptr<ReaderInterface>& docreader, std::vector<std::string>& arguments);

#endif