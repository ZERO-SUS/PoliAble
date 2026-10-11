#include "Serializer.h"
#include <sstream>
#include <queue>
#include <unordered_set>
#include <iomanip>

namespace poliable {

// ── Type name mapping ────────────────────────────────────────────────────────

static const char* typeName(NodeType t) {
    switch (t) {
        case NodeType::Rectangle: return "rectangle";
        case NodeType::Text:      return "text";
        case NodeType::Group:     return "group";
        case NodeType::Canvas:    return "canvas";
        case NodeType::Frame:     return "frame";
        case NodeType::Image:     return "image";
        case NodeType::Ellipse:   return "ellipse";
        case NodeType::Line:      return "line";
        case NodeType::Flex:      return "flex";
        case NodeType::Grid:      return "grid";
        case NodeType::Scroll:    return "scroll";
        default:                  return "unknown";
    }
}

static NodeType typeFromName(const std::string& n) {
    if (n == "rectangle") return NodeType::Rectangle;
    if (n == "text")      return NodeType::Text;
    if (n == "group")     return NodeType::Group;
    if (n == "canvas")    return NodeType::Canvas;
    if (n == "frame")     return NodeType::Frame;
    if (n == "image")     return NodeType::Image;
    if (n == "ellipse")   return NodeType::Ellipse;
    if (n == "line")      return NodeType::Line;
    if (n == "flex")      return NodeType::Flex;
    if (n == "grid")      return NodeType::Grid;
    if (n == "scroll")    return NodeType::Scroll;
    return NodeType::Unknown;
}

static const char* layoutModeToString(LayoutMode m) {
    switch (m) {
        case LayoutMode::Flex: return "flex";
        case LayoutMode::Grid: return "grid";
        default: return "none";
    }
}

static LayoutMode layoutModeFromString(const std::string& s) {
    if (s == "flex") return LayoutMode::Flex;
    if (s == "grid") return LayoutMode::Grid;
    return LayoutMode::None;
}

static const char* sizingModeToString(SizingMode m) {
    switch (m) {
        case SizingMode::Hug:  return "hug";
        case SizingMode::Fill: return "fill";
        default: return "fixed";
    }
}

static SizingMode sizingModeFromString(const std::string& s) {
    if (s == "hug")  return SizingMode::Hug;
    if (s == "fill") return SizingMode::Fill;
    return SizingMode::Fixed;
}

static const char* hConstraintToString(HorizontalConstraint c) {
    switch (c) {
        case HorizontalConstraint::Right:     return "right";
        case HorizontalConstraint::LeftRight: return "left_right";
        case HorizontalConstraint::Center:    return "center";
        case HorizontalConstraint::Scale:     return "scale";
        default: return "left";
    }
}

static HorizontalConstraint hConstraintFromString(const std::string& s) {
    if (s == "right")      return HorizontalConstraint::Right;
    if (s == "left_right") return HorizontalConstraint::LeftRight;
    if (s == "center")     return HorizontalConstraint::Center;
    if (s == "scale")      return HorizontalConstraint::Scale;
    return HorizontalConstraint::Left;
}

static const char* vConstraintToString(VerticalConstraint c) {
    switch (c) {
        case VerticalConstraint::Bottom:    return "bottom";
        case VerticalConstraint::TopBottom: return "top_bottom";
        case VerticalConstraint::Center:    return "center";
        case VerticalConstraint::Scale:     return "scale";
        default: return "top";
    }
}

static VerticalConstraint vConstraintFromString(const std::string& s) {
    if (s == "bottom")     return VerticalConstraint::Bottom;
    if (s == "top_bottom") return VerticalConstraint::TopBottom;
    if (s == "center")     return VerticalConstraint::Center;
    if (s == "scale")      return VerticalConstraint::Scale;
    return VerticalConstraint::Top;
}

static const char* flexDirToString(FlexDirection d) {
    switch (d) {
        case FlexDirection::Column:        return "column";
        case FlexDirection::RowReverse:    return "row_reverse";
        case FlexDirection::ColumnReverse: return "column_reverse";
        default: return "row";
    }
}

static FlexDirection flexDirFromString(const std::string& s) {
    if (s == "column")         return FlexDirection::Column;
    if (s == "row_reverse")    return FlexDirection::RowReverse;
    if (s == "column_reverse") return FlexDirection::ColumnReverse;
    return FlexDirection::Row;
}

static const char* flexWrapToString(FlexWrap w) {
    switch (w) {
        case FlexWrap::Wrap:        return "wrap";
        case FlexWrap::WrapReverse: return "wrap_reverse";
        default: return "nowrap";
    }
}

static FlexWrap flexWrapFromString(const std::string& s) {
    if (s == "wrap")         return FlexWrap::Wrap;
    if (s == "wrap_reverse") return FlexWrap::WrapReverse;
    return FlexWrap::NoWrap;
}

static const char* justifyToString(JustifyContent j) {
    switch (j) {
        case JustifyContent::FlexEnd:      return "flex_end";
        case JustifyContent::Center:       return "center";
        case JustifyContent::SpaceBetween: return "space_between";
        case JustifyContent::SpaceAround:  return "space_around";
        case JustifyContent::SpaceEvenly:  return "space_evenly";
        default: return "flex_start";
    }
}

static JustifyContent justifyFromString(const std::string& s) {
    if (s == "flex_end")      return JustifyContent::FlexEnd;
    if (s == "center")        return JustifyContent::Center;
    if (s == "space_between") return JustifyContent::SpaceBetween;
    if (s == "space_around")  return JustifyContent::SpaceAround;
    if (s == "space_evenly")  return JustifyContent::SpaceEvenly;
    return JustifyContent::FlexStart;
}

static const char* alignItemsToString(AlignItems a) {
    switch (a) {
        case AlignItems::FlexEnd:  return "flex_end";
        case AlignItems::Center:   return "center";
        case AlignItems::Stretch:  return "stretch";
        case AlignItems::Baseline: return "baseline";
        default: return "flex_start";
    }
}

static AlignItems alignItemsFromString(const std::string& s) {
    if (s == "flex_end")  return AlignItems::FlexEnd;
    if (s == "center")    return AlignItems::Center;
    if (s == "stretch")   return AlignItems::Stretch;
    if (s == "baseline")  return AlignItems::Baseline;
    return AlignItems::FlexStart;
}

static const char* alignSelfToString(AlignSelf a) {
    switch (a) {
        case AlignSelf::FlexStart: return "flex_start";
        case AlignSelf::FlexEnd:   return "flex_end";
        case AlignSelf::Center:    return "center";
        case AlignSelf::Stretch:   return "stretch";
        default: return "auto";
    }
}

static AlignSelf alignSelfFromString(const std::string& s) {
    if (s == "flex_start") return AlignSelf::FlexStart;
    if (s == "flex_end")   return AlignSelf::FlexEnd;
    if (s == "center")     return AlignSelf::Center;
    if (s == "stretch")    return AlignSelf::Stretch;
    return AlignSelf::Auto;
}

// Gradient helper functions
static const char* gradientTypeToString(GradientType t) {
    switch (t) {
        case GradientType::Linear:  return "linear";
        case GradientType::Radial:  return "radial";
        case GradientType::Conic:   return "conic";
        default: return "none";
    }
}

static GradientType gradientTypeFromString(const std::string& s) {
    if (s == "linear") return GradientType::Linear;
    if (s == "radial") return GradientType::Radial;
    if (s == "conic")  return GradientType::Conic;
    return GradientType::None;
}

// ── String escaping ───────────────────────────────────────────────────────────

static std::string escapeString(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    for (char c : s) {
        if      (c == '"')  result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else if (c == '\r') result += "\\r";
        else                result += c;
    }
    return result;
}

static std::string unescapeString(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    for (size_t i = 0; i < s.length(); ++i) {
        if (s[i] == '\\' && i + 1 < s.length()) {
            char next = s[++i];
            if      (next == '"')  result += '"';
            else if (next == '\\') result += '\\';
            else if (next == 'n')  result += '\n';
            else if (next == 'r')  result += '\r';
            else                   result += next;
        } else {
            result += s[i];
        }
    }
    return result;
}

// Helper: read a quoted string token from a stream
// Reads leading '"', then up to closing '"', handling escapes.
static bool readQuoted(std::istringstream& ls, std::string& out) {
    char q;
    ls >> q;
    if (q != '"') return false;
    std::string raw;
    std::getline(ls, raw, '"');
    out = unescapeString(raw);
    return true;
}

// ── Serialise ─────────────────────────────────────────────────────────────────

std::string Serializer::toString(const Document& doc) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(3);

    // Header
    os << "schema "  << doc.metadata().schemaId << "\n";
    os << "version " << doc.metadata().versionMajor << "."
                     << doc.metadata().versionMinor << "."
                     << doc.metadata().versionPatch << "\n";
    os << "name \""  << escapeString(doc.name()) << "\"\n";

    // BFS over all nodes
    std::queue<Node*> q;
    q.push(doc.root());
    std::unordered_set<Node::Id> seen;

    while (!q.empty()) {
        Node* n = q.front(); q.pop();
        if (!n || seen.count(n->id())) continue;
        seen.insert(n->id());

        Node::Id parentId = n->parent() ? n->parent()->id() : 0;
        const auto& t = n->transform();
        const auto& s = n->style();

        // node <type> <id> <parent> "<name>"
        //      <x> <y> <w> <h> <rot> <opacity>
        //      "<fill>" "<stroke>" <strokeWidth>
        //      <cornerRadius> <fontSize> "<textContent>"
        // Serialize the full style with all new properties
        os << "node "
           << typeName(n->type()) << " "
           << n->id()     << " "
           << parentId    << " "
           << "\"" << escapeString(n->name()) << "\" "
           << t.x        << " " << t.y      << " "
           << t.width    << " " << t.height << " "
           << t.rotation << " " << t.opacity << " "
           << "\"" << escapeString(s.fillColor)   << "\" "
           << "\"" << escapeString(s.strokeColor) << "\" "
           << s.strokeWidth   << " "
           << s.cornerRadius  << " "
           << s.fontSize      << " "
           << "\"" << escapeString(s.textContent) << "\""
           << " gradient:" << gradientTypeToString(s.fillGradient.type) << " "
           << s.isFillGradient << " "
           << "\"" << escapeString(s.strokeStyle) << "\" "
           << s.cornerRadiusTopLeft << " "
           << s.cornerRadiusTopRight << " "
           << s.cornerRadiusBottomLeft << " "
           << s.cornerRadiusBottomRight << " "
           << "\"" << escapeString(s.backgroundImage) << "\" "
           << "\"" << escapeString(s.backgroundSize) << "\" "
           << "\"" << escapeString(s.backgroundPosition) << "\""
           << "\n";

        // Effects serialization
        os << "effects " << n->id() << " "
           << s.effects.blur << " "
           << s.effects.brightness << " "
           << s.effects.contrast << " "
           << s.effects.saturation << " "
           << s.effects.hueRotate << " "
           << s.effects.opacity << " "
           << s.effects.shadows.size();
        for (const auto& shadow : s.effects.shadows) {
            os << " shadow:" << escapeString(shadow.color)
               << ":" << shadow.offsetX << ":" << shadow.offsetY
               << ":" << shadow.blurRadius << ":" << shadow.spreadRadius;
        }
        os << "\n";

        const auto& l = n->layout();
        os << "layout " << n->id() << " "
           << layoutModeToString(l.mode) << " "
           << sizingModeToString(l.widthMode) << " "
           << sizingModeToString(l.heightMode) << " "
           << l.minWidth << " " << l.maxWidth << " "
           << l.minHeight << " " << l.maxHeight << " "
           << hConstraintToString(l.horizontalConstraint) << " "
           << vConstraintToString(l.verticalConstraint) << " "
           << l.paddingTop << " " << l.paddingRight << " "
           << l.paddingBottom << " " << l.paddingLeft << " "
           << l.marginTop << " " << l.marginRight << " "
           << l.marginBottom << " " << l.marginLeft << " "
           << flexDirToString(l.flexDirection) << " "
           << flexWrapToString(l.flexWrap) << " "
           << justifyToString(l.justifyContent) << " "
           << alignItemsToString(l.alignItems) << " "
           << l.gap << " " << l.rowGap << " " << l.columnGap << " "
           << l.flexGrow << " " << l.flexShrink << " " << l.flexBasis << " "
           << alignSelfToString(l.alignSelf) << " "
           << l.gridColumns << " " << l.gridRows << " "
           << l.gridColumnSpan << " " << l.gridRowSpan << " "
           << l.gridColumnStart << " " << l.gridRowStart
           << "\n";

        for (auto& c : n->children()) q.push(c.get());
    }

    return os.str();
}

// ── Deserialise ───────────────────────────────────────────────────────────────

bool Serializer::fromString(const std::string& data, Document& out, std::string& error) {
    // Start fresh
    out = Document{};

    std::istringstream is(data);
    std::string line;
    int lineNo = 0;

    struct NodeData {
        std::string typeName;
        Node::Id    id;
        Node::Id    parentId;
        std::string name;
        Transform   transform;
        Style       style;
    };
    std::vector<NodeData> nodes;
    std::unordered_map<Node::Id, Layout> layouts;

    while (std::getline(is, line)) {
        ++lineNo;
        if (line.empty() || line[0] == '#') continue;

        std::istringstream ls(line);
        std::string keyword;
        ls >> keyword;

        if (keyword == "schema") {
            std::string sid;
            ls >> sid;
            out.metadata().schemaId = sid;

        } else if (keyword == "version") {
            int maj = 0, min = 0, pat = 0;
            char d1, d2;
            ls >> maj >> d1 >> min >> d2 >> pat;
            out.metadata().versionMajor = maj;
            out.metadata().versionMinor = min;
            out.metadata().versionPatch = pat;

        } else if (keyword == "name") {
            std::string docName;
            if (!readQuoted(ls, docName)) {
                error = "expected quoted name at line " + std::to_string(lineNo);
                return false;
            }
            out.setName(docName);

        } else if (keyword == "node") {
            NodeData nd{};
            ls >> nd.typeName >> nd.id >> nd.parentId;

            if (!readQuoted(ls, nd.name)) {
                error = "expected quoted node name at line " + std::to_string(lineNo);
                return false;
            }

            auto& t = nd.transform;
            ls >> t.x >> t.y >> t.width >> t.height >> t.rotation >> t.opacity;

            auto& s = nd.style;
            if (!readQuoted(ls, s.fillColor)) {
                error = "expected quoted fill at line " + std::to_string(lineNo);
                return false;
            }
            if (!readQuoted(ls, s.strokeColor)) {
                error = "expected quoted stroke at line " + std::to_string(lineNo);
                return false;
            }
            ls >> s.strokeWidth >> s.cornerRadius >> s.fontSize;
            if (!readQuoted(ls, s.textContent)) {
                s.textContent = "";
            }

            // Read extended style properties
            std::string gradientStr, isGradientStr, strokeStyleStr;
            if (ls >> gradientStr >> isGradientStr) {
                s.fillGradient.type = gradientTypeFromString(gradientStr);
                s.isFillGradient = (isGradientStr == "1");
            }
            if (ls >> strokeStyleStr) {
                s.strokeStyle = strokeStyleStr;
            }
            ls >> s.cornerRadiusTopLeft >> s.cornerRadiusTopRight
               >> s.cornerRadiusBottomLeft >> s.cornerRadiusBottomRight;
            if (ls.peek() == '"') {
                readQuoted(ls, s.backgroundImage);
            }
            if (ls.peek() == '"') {
                readQuoted(ls, s.backgroundSize);
            }
            if (ls.peek() == '"') {
                readQuoted(ls, s.backgroundPosition);
            }

            nodes.push_back(std::move(nd));

        } else if (keyword == "layout") {
            Node::Id nodeId = 0;
            std::string modeStr, wModeStr, hModeStr, hConstStr, vConstStr;
            std::string fDirStr, fWrapStr, justStr, aItemsStr, aSelfStr;
            Layout l{};

            ls >> nodeId >> modeStr >> wModeStr >> hModeStr
               >> l.minWidth >> l.maxWidth >> l.minHeight >> l.maxHeight
               >> hConstStr >> vConstStr
               >> l.paddingTop >> l.paddingRight >> l.paddingBottom >> l.paddingLeft
               >> l.marginTop >> l.marginRight >> l.marginBottom >> l.marginLeft
               >> fDirStr >> fWrapStr >> justStr >> aItemsStr
               >> l.gap >> l.rowGap >> l.columnGap
               >> l.flexGrow >> l.flexShrink >> l.flexBasis >> aSelfStr
               >> l.gridColumns >> l.gridRows
               >> l.gridColumnSpan >> l.gridRowSpan
               >> l.gridColumnStart >> l.gridRowStart;

            l.mode = layoutModeFromString(modeStr);
            l.widthMode = sizingModeFromString(wModeStr);
            l.heightMode = sizingModeFromString(hModeStr);
            l.horizontalConstraint = hConstraintFromString(hConstStr);
            l.verticalConstraint = vConstraintFromString(vConstStr);
            l.flexDirection = flexDirFromString(fDirStr);
            l.flexWrap = flexWrapFromString(fWrapStr);
            l.justifyContent = justifyFromString(justStr);
            l.alignItems = alignItemsFromString(aItemsStr);
            l.alignSelf = alignSelfFromString(aSelfStr);

            layouts[nodeId] = l;

        } else if (keyword == "effects") {
            Node::Id nodeId;
            ls >> nodeId;
            if (nodeId > 0) {
                for (auto& nd : nodes) {
                    if (nd.id == nodeId) {
                        ls >> nd.style.effects.blur >> nd.style.effects.brightness
                           >> nd.style.effects.contrast >> nd.style.effects.saturation
                           >> nd.style.effects.hueRotate >> nd.style.effects.opacity;
                        int shadowCount = 0;
                        ls >> shadowCount;
                        nd.style.effects.shadows.clear();
                        for (int i = 0; i < shadowCount; ++i) {
                            std::string token;
                            if (ls >> token && token.find("shadow:") == 0) {
                                Shadow shadow;
                                // Parse shadow data from token: shadow:color:ox:oy:blur:spread
                                std::string color;
                                double ox, oy, blur, spread;
                                int colonCount = 0;
                                for (size_t pos = 7; pos < token.length(); ++pos) {
                                    if (token[pos] == ':') {
                                        colonCount++;
                                        continue;
                                    }
                                    if (colonCount == 0) color += token[pos];
                                    else if (colonCount == 1) {
                                        std::string val; val += token[pos];
                                        // Parse ox (simplified)
                                    }
                                }
                                nd.style.effects.shadows.push_back(shadow);
                            }
                        }
                        break;
                    }
                }
            }

        } else {
            error = "unknown keyword '" + keyword + "' at line " + std::to_string(lineNo);
            return false;
        }
    }

    // Build tree: nodes are in BFS order, so parents always appear before children
    for (auto& nd : nodes) {
        out.reserveId(nd.id);

        if (nd.id == 0) {
            // The root node — update in-place
            Node* root = out.root();
            root->setName(nd.name);
            root->transform() = nd.transform;
            root->style()     = nd.style;
        } else {
            auto node = std::make_unique<Node>(typeFromName(nd.typeName), nd.id, nd.name);
            node->transform() = nd.transform;
            node->style()     = nd.style;

            if (out.addChild(nd.parentId, std::move(node)) == nullptr) {
                error = "could not attach node id=" + std::to_string(nd.id)
                      + " to parent id=" + std::to_string(nd.parentId)
                      + " (parent not found) at line " + std::to_string(lineNo);
                return false;
            }
        }
    }

    // Apply deserialized layout configurations
    for (const auto& [nodeId, l] : layouts) {
        if (auto* node = out.find(nodeId)) {
            node->setLayout(l);
        }
    }

    return true;
}

} // namespace poliable
