#pragma once
#include <string>
#include <vector>
#include <memory>
#include "model/Document.h"

namespace poliable {

enum class ExportFormat {
    HtmlCss,
    ReactTypeScript,
    TailwindHtml
};

struct GeneratedCode {
    std::string html;
    std::string css;
    std::string reactTsx;
    std::string tailwindHtml;
};

class CodeGenerator {
public:
    // Generate code for document in various formats
    static GeneratedCode generate(const Document& doc);

    static std::string generateHtml(const Document& doc);
    static std::string generateCss(const Document& doc);
    static std::string generateReactTsx(const Document& doc);
    static std::string generateTailwindHtml(const Document& doc);

    // Export complete, runnable project to disk
    static bool exportProject(const Document& doc,
                              const std::string& targetDirectory,
                              ExportFormat format,
                              std::string& outError);

private:
    static void generateNodeHtml(const Node* node, std::string& html, int indentLevel);
    static void generateNodeCss(const Node* node, std::string& css);
    static void generateNodeReact(const Node* node, std::string& tsx, int indentLevel);
    static void generateNodeTailwind(const Node* node, std::string& tw, int indentLevel);
    static std::string getTailwindClasses(const Node* node);
    static std::string sanitizeClassName(const std::string& name, uint64_t id);
    static std::string indent(int level);
};

} // namespace poliable
