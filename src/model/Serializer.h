#pragma once
#include "Document.h"
#include <string>
#include <sstream>

namespace poliable {

class Serializer {
public:
    // Simple line-based text format for now (easy to debug/extend).
    // Format:
    //   # meta
    //   schema <id>
    //   version <major>.<minor>.<patch>
    //   # nodes (BFS, each line: TYPE ID PARENT_ID NAME X Y W H ROT OPACITY FILL STROKE SW)
    //   node <type> <id> <parent> <name> <x> <y> <w> <h> <rot> <op> <fill> <stroke> <sw>
    static std::string toString(const Document& doc);
    static bool fromString(const std::string& data, Document& out, std::string& error);
};

} // namespace poliable
