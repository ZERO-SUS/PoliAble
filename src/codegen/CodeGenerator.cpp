#include "CodeGenerator.h"
#include "model/Node.h"
#include <sstream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

namespace poliable {

// ── Helpers ──────────────────────────────────────────────────────────────────

std::string CodeGenerator::indent(int level) {
    return std::string(level * 2, ' ');
}

std::string CodeGenerator::sanitizeClassName(const std::string& name, uint64_t id) {
    std::string res;
    for (char c : name) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            res += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        } else if (c == ' ' || c == '_' || c == '-') {
            res += '-';
        }
    }
    if (res.empty()) res = "element";
    return res + "-" + std::to_string(id);
}

// ── Full Generation ──────────────────────────────────────────────────────────

GeneratedCode CodeGenerator::generate(const Document& doc) {
    GeneratedCode code;
    code.html = generateHtml(doc);
    code.css = generateCss(doc);
    code.reactTsx = generateReactTsx(doc);
    code.tailwindHtml = generateTailwindHtml(doc);
    return code;
}

// ── HTML Generation ──────────────────────────────────────────────────────────

std::string CodeGenerator::generateHtml(const Document& doc) {
    std::ostringstream out;
    out << "<!DOCTYPE html>\n";
    out << "<html lang=\"en\">\n";
    out << "<head>\n";
    out << "  <meta charset=\"UTF-8\">\n";
    out << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    out << "  <title>" << (doc.name().empty() ? "PoliAble Project" : doc.name()) << "</title>\n";
    out << "  <link rel=\"stylesheet\" href=\"styles.css\">\n";
    out << "</head>\n";
    out << "<body>\n";

    if (doc.root()) {
        for (const auto& child : doc.root()->children()) {
            std::string nodeHtml;
            generateNodeHtml(child.get(), nodeHtml, 1);
            out << nodeHtml;
        }
    }

    out << "</body>\n";
    out << "</html>\n";
    return out.str();
}

void CodeGenerator::generateNodeHtml(const Node* node, std::string& html, int indentLevel) {
    if (!node) return;

    std::string tag = "div";
    std::string content = "";

    if (node->type() == NodeType::Text) {
        tag = "p";
        content = node->style().textContent.empty() ? node->name() : node->style().textContent;
    } else if (node->type() == NodeType::Frame) {
        tag = "section";
    } else if (node->name().find("button") != std::string::npos || node->name().find("Button") != std::string::npos) {
        tag = "button";
    } else if (node->name().find("nav") != std::string::npos || node->name().find("Navbar") != std::string::npos) {
        tag = "nav";
    }

    std::string cls = sanitizeClassName(node->name(), node->id());

    html += indent(indentLevel) + "<" + tag + " class=\"" + cls + "\">";

    if (!content.empty()) {
        html += content;
    }

    if (!node->children().empty()) {
        html += "\n";
        for (const auto& child : node->children()) {
            generateNodeHtml(child.get(), html, indentLevel + 1);
        }
        html += indent(indentLevel);
    }

    html += "</" + tag + ">\n";
}

// ── CSS Generation ───────────────────────────────────────────────────────────

std::string CodeGenerator::generateCss(const Document& doc) {
    std::ostringstream out;
    out << "/* ==========================================================================\n";
    out << "   PoliAble Generated Stylesheet\n";
    out << "   Document: " << (doc.name().empty() ? "PoliAble Project" : doc.name()) << "\n";
    out << "   ========================================================================== */\n\n";

    out << ":root {\n";
    out << "  --font-primary: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;\n";
    out << "  --color-bg: #0f172a;\n";
    out << "  --color-text: #f8fafc;\n";
    out << "}\n\n";

    out << "* {\n";
    out << "  box-sizing: border-box;\n";
    out << "  margin: 0;\n";
    out << "  padding: 0;\n";
    out << "}\n\n";

    out << "body {\n";
    out << "  font-family: var(--font-primary);\n";
    out << "  background-color: var(--color-bg);\n";
    out << "  color: var(--color-text);\n";
    out << "  min-height: 100vh;\n";
    out << "}\n\n";

    if (doc.root()) {
        for (const auto& child : doc.root()->children()) {
            std::string nodeCss;
            generateNodeCss(child.get(), nodeCss);
            out << nodeCss;
        }
    }

    // Responsive media queries
    out << "\n/* ── Responsive Breakpoints ── */\n";
    out << "@media (max-width: 1024px) {\n";
    out << "  /* Laptop Breakpoint */\n";
    out << "}\n\n";
    out << "@media (max-width: 768px) {\n";
    out << "  /* Tablet Breakpoint */\n";
    out << "}\n\n";
    out << "@media (max-width: 480px) {\n";
    out << "  /* Mobile Breakpoint */\n";
    out << "}\n";

    return out.str();
}

void CodeGenerator::generateNodeCss(const Node* node, std::string& css) {
    if (!node) return;

    std::string cls = sanitizeClassName(node->name(), node->id());
    std::ostringstream s;
    s << "." << cls << " {\n";

    const auto& t = node->transform();
    const auto& st = node->style();
    const auto& lay = node->layout();

    // Layout
    if (lay.mode == LayoutMode::Flex || node->type() == NodeType::Flex) {
        s << "  display: flex;\n";
        s << "  flex-direction: " << (lay.flexDirection == FlexDirection::Column ? "column" : "row") << ";\n";

        // Justify
        switch (lay.justifyContent) {
            case JustifyContent::Center:       s << "  justify-content: center;\n"; break;
            case JustifyContent::FlexEnd:      s << "  justify-content: flex-end;\n"; break;
            case JustifyContent::SpaceBetween: s << "  justify-content: space-between;\n"; break;
            case JustifyContent::SpaceAround:  s << "  justify-content: space-around;\n"; break;
            case JustifyContent::SpaceEvenly:  s << "  justify-content: space-evenly;\n"; break;
            default:                           s << "  justify-content: flex-start;\n"; break;
        }

        // Align items
        switch (lay.alignItems) {
            case AlignItems::Center:   s << "  align-items: center;\n"; break;
            case AlignItems::FlexEnd:  s << "  align-items: flex-end;\n"; break;
            case AlignItems::Stretch:  s << "  align-items: stretch;\n"; break;
            default:                   s << "  align-items: flex-start;\n"; break;
        }

        if (lay.gap > 0) {
            s << "  gap: " << static_cast<int>(lay.gap) << "px;\n";
        }

        if (lay.paddingTop > 0 || lay.paddingRight > 0 || lay.paddingBottom > 0 || lay.paddingLeft > 0) {
            s << "  padding: " << static_cast<int>(lay.paddingTop) << "px "
              << static_cast<int>(lay.paddingRight) << "px "
              << static_cast<int>(lay.paddingBottom) << "px "
              << static_cast<int>(lay.paddingLeft) << "px;\n";
        }
    } else if (lay.mode == LayoutMode::Grid || node->type() == NodeType::Grid) {
        s << "  display: grid;\n";
        int cols = std::max(1, lay.gridColumns);
        s << "  grid-template-columns: repeat(" << cols << ", 1fr);\n";
        if (lay.columnGap > 0 || lay.rowGap > 0) {
            s << "  gap: " << static_cast<int>(lay.rowGap) << "px " << static_cast<int>(lay.columnGap) << "px;\n";
        }
    }

    // Sizing
    if (lay.widthMode == SizingMode::Fill) {
        s << "  width: 100%;\n";
    } else if (lay.widthMode == SizingMode::Hug) {
        s << "  width: fit-content;\n";
    } else {
        s << "  width: " << static_cast<int>(t.width) << "px;\n";
    }

    if (lay.heightMode == SizingMode::Fill) {
        s << "  height: 100%;\n";
    } else if (lay.heightMode == SizingMode::Hug) {
        s << "  height: fit-content;\n";
    } else {
        s << "  height: " << static_cast<int>(t.height) << "px;\n";
    }

    // Colors & Styles
    if (!st.fillColor.empty() && node->type() != NodeType::Text) {
        s << "  background-color: " << st.fillColor << ";\n";
    }

    if (node->type() == NodeType::Text) {
        s << "  color: " << (st.fillColor.empty() ? "#ffffff" : st.fillColor) << ";\n";
        s << "  font-size: " << (st.fontSize > 0 ? st.fontSize : 16) << "px;\n";
    }

    if (st.cornerRadius > 0) {
        s << "  border-radius: " << static_cast<int>(st.cornerRadius) << "px;\n";
    }

    if (st.strokeWidth > 0 && !st.strokeColor.empty()) {
        s << "  border: " << static_cast<int>(st.strokeWidth) << "px "
          << st.strokeStyle << " " << st.strokeColor << ";\n";
    }

    if (t.opacity < 0.999) {
        s << "  opacity: " << t.opacity << ";\n";
    }

    s << "}\n\n";
    css += s.str();

    for (const auto& child : node->children()) {
        generateNodeCss(child.get(), css);
    }
}

// ── React / TypeScript Generation ────────────────────────────────────────────

std::string CodeGenerator::generateReactTsx(const Document& doc) {
    std::ostringstream out;
    out << "import React from 'react';\n";
    out << "import './styles.css';\n\n";

    out << "export interface " << (doc.name().empty() ? "App" : doc.name()) << "Props {\n";
    out << "  className?: string;\n";
    out << "}\n\n";

    out << "export const " << (doc.name().empty() ? "App" : doc.name()) << ": React.FC<"
        << (doc.name().empty() ? "App" : doc.name()) << "Props> = ({ className = '' }) => {\n";
    out << "  return (\n";
    out << "    <main className={`poliable-root ${className}`}>\n";

    if (doc.root()) {
        for (const auto& child : doc.root()->children()) {
            std::string nodeJsx;
            generateNodeReact(child.get(), nodeJsx, 3);
            out << nodeJsx;
        }
    }

    out << "    </main>\n";
    out << "  );\n";
    out << "};\n\n";
    out << "export default " << (doc.name().empty() ? "App" : doc.name()) << ";\n";

    return out.str();
}

void CodeGenerator::generateNodeReact(const Node* node, std::string& tsx, int indentLevel) {
    if (!node) return;

    std::string tag = "div";
    std::string content = "";

    if (node->type() == NodeType::Text) {
        tag = "p";
        content = node->style().textContent.empty() ? node->name() : node->style().textContent;
    } else if (node->name().find("Button") != std::string::npos || node->name().find("button") != std::string::npos) {
        tag = "button";
    } else if (node->name().find("Navbar") != std::string::npos || node->name().find("nav") != std::string::npos) {
        tag = "nav";
    }

    std::string cls = sanitizeClassName(node->name(), node->id());

    tsx += indent(indentLevel) + "<" + tag + " className=\"" + cls + "\">";

    if (!content.empty()) {
        tsx += content;
    }

    if (!node->children().empty()) {
        tsx += "\n";
        for (const auto& child : node->children()) {
            generateNodeReact(child.get(), tsx, indentLevel + 1);
        }
        tsx += indent(indentLevel);
    }

    tsx += "</" + tag + ">\n";
}

// ── Tailwind CSS Mapping ─────────────────────────────────────────────────────

std::string CodeGenerator::getTailwindClasses(const Node* node) {
    if (!node) return "";
    std::string tw;

    const auto& st = node->style();
    const auto& lay = node->layout();

    if (lay.mode == LayoutMode::Flex || node->type() == NodeType::Flex) {
        tw += "flex ";
        tw += (lay.flexDirection == FlexDirection::Column) ? "flex-col " : "flex-row ";

        switch (lay.justifyContent) {
            case JustifyContent::Center:       tw += "justify-center "; break;
            case JustifyContent::SpaceBetween: tw += "justify-between "; break;
            case JustifyContent::FlexEnd:      tw += "justify-end "; break;
            default:                           tw += "justify-start "; break;
        }

        switch (lay.alignItems) {
            case AlignItems::Center:   tw += "items-center "; break;
            case AlignItems::FlexEnd:  tw += "items-end "; break;
            case AlignItems::Stretch:  tw += "items-stretch "; break;
            default:                   tw += "items-start "; break;
        }

        if (lay.gap >= 16) tw += "gap-4 ";
        else if (lay.gap >= 12) tw += "gap-3 ";
        else if (lay.gap >= 8) tw += "gap-2 ";

        if (lay.paddingLeft >= 24) tw += "p-6 ";
        else if (lay.paddingLeft >= 16) tw += "p-4 ";
        else if (lay.paddingLeft >= 8) tw += "p-2 ";
    } else if (lay.mode == LayoutMode::Grid || node->type() == NodeType::Grid) {
        tw += "grid grid-cols-2 gap-4 p-4 ";
    }

    // Radius
    if (st.cornerRadius >= 12) tw += "rounded-xl ";
    else if (st.cornerRadius >= 8) tw += "rounded-lg ";
    else if (st.cornerRadius >= 4) tw += "rounded-md ";

    // Fill
    if (!st.fillColor.empty() && node->type() != NodeType::Text) {
        if (st.fillColor == "#6366f1") tw += "bg-indigo-600 ";
        else if (st.fillColor == "#3b82f6") tw += "bg-blue-600 ";
        else if (st.fillColor == "#10b981") tw += "bg-emerald-600 ";
        else if (st.fillColor == "#1e2028") tw += "bg-slate-900 border border-slate-700 ";
        else if (st.fillColor == "#1a1b22") tw += "bg-slate-900/90 ";
    }

    // Text
    if (node->type() == NodeType::Text) {
        tw += "text-white ";
        if (st.fontSize >= 24) tw += "text-2xl font-bold ";
        else if (st.fontSize >= 18) tw += "text-lg font-semibold ";
        else if (st.fontSize >= 14) tw += "text-sm ";
        else tw += "text-xs text-slate-300 ";
    }

    return tw;
}

std::string CodeGenerator::generateTailwindHtml(const Document& doc) {
    std::ostringstream out;
    out << "<!DOCTYPE html>\n";
    out << "<html lang=\"en\">\n";
    out << "<head>\n";
    out << "  <meta charset=\"UTF-8\">\n";
    out << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    out << "  <title>" << (doc.name().empty() ? "PoliAble Project" : doc.name()) << "</title>\n";
    out << "  <script src=\"https://cdn.tailwindcss.com\"></script>\n";
    out << "</head>\n";
    out << "<body class=\"bg-slate-950 text-slate-100 min-h-screen p-8\">\n";

    if (doc.root()) {
        for (const auto& child : doc.root()->children()) {
            std::string nodeTw;
            generateNodeTailwind(child.get(), nodeTw, 1);
            out << nodeTw;
        }
    }

    out << "</body>\n";
    out << "</html>\n";
    return out.str();
}

void CodeGenerator::generateNodeTailwind(const Node* node, std::string& tw, int indentLevel) {
    if (!node) return;

    std::string tag = "div";
    std::string content = "";

    if (node->type() == NodeType::Text) {
        tag = "p";
        content = node->style().textContent.empty() ? node->name() : node->style().textContent;
    } else if (node->name().find("button") != std::string::npos || node->name().find("Button") != std::string::npos) {
        tag = "button";
    }

    std::string classes = getTailwindClasses(node);

    tw += indent(indentLevel) + "<" + tag + " class=\"" + classes + "\">";

    if (!content.empty()) {
        tw += content;
    }

    if (!node->children().empty()) {
        tw += "\n";
        for (const auto& child : node->children()) {
            generateNodeTailwind(child.get(), tw, indentLevel + 1);
        }
        tw += indent(indentLevel);
    }

    tw += "</" + tag + ">\n";
}

// ── Export Project to Disk ───────────────────────────────────────────────────

bool CodeGenerator::exportProject(const Document& doc,
                                 const std::string& targetDirectory,
                                 ExportFormat format,
                                 std::string& outError) {
    try {
        fs::path outDir(targetDirectory);
        if (!fs::exists(outDir)) {
            fs::create_directories(outDir);
        }

        if (format == ExportFormat::HtmlCss) {
            // Write index.html
            std::ofstream htmlFile(outDir / "index.html");
            if (!htmlFile.is_open()) {
                outError = "Failed to create index.html";
                return false;
            }
            htmlFile << generateHtml(doc);

            // Write styles.css
            std::ofstream cssFile(outDir / "styles.css");
            if (!cssFile.is_open()) {
                outError = "Failed to create styles.css";
                return false;
            }
            cssFile << generateCss(doc);

            // Write README.md
            std::ofstream readme(outDir / "README.md");
            readme << "# " << doc.name() << "\n\nGenerated by PoliAble Studio.\n";

        } else if (format == ExportFormat::ReactTypeScript) {
            fs::create_directories(outDir / "src");
            fs::create_directories(outDir / "public");

            // App.tsx
            std::ofstream appFile(outDir / "src" / "App.tsx");
            appFile << generateReactTsx(doc);

            // styles.css
            std::ofstream cssFile(outDir / "src" / "styles.css");
            cssFile << generateCss(doc);

            // package.json
            std::ofstream pkg(outDir / "package.json");
            pkg << "{\n"
                << "  \"name\": \"" << sanitizeClassName(doc.name(), 1) << "\",\n"
                << "  \"version\": \"1.0.0\",\n"
                << "  \"private\": true,\n"
                << "  \"dependencies\": {\n"
                << "    \"react\": \"^18.2.0\",\n"
                << "    \"react-dom\": \"^18.2.0\"\n"
                << "  }\n"
                << "}\n";

        } else if (format == ExportFormat::TailwindHtml) {
            std::ofstream twFile(outDir / "index.html");
            twFile << generateTailwindHtml(doc);
        }

        return true;
    } catch (const std::exception& e) {
        outError = e.what();
        return false;
    }
}

} // namespace poliable
