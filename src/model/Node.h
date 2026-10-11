#pragma once
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "model/Styling.h"

namespace poliable {

enum class NodeType {
    Unknown,
    Rectangle,
    Text,
    Group,
    Canvas,
    Frame,
    Image,
    Ellipse,
    Line,
    Flex,       // Flexbox container
    Grid,       // CSS Grid container
    Scroll,     // Scrollable container
    Component,  // Master component definition (Stage 6)
    Instance    // Component instance (Stage 6)
};

struct Transform {
    double x = 0.0, y = 0.0;
    double width = 100.0, height = 100.0;
    double rotation = 0.0;
    double opacity = 1.0;
};

enum class LayoutMode {
    None,       // Absolute positioning
    Flex,       // Flexbox / Auto Layout
    Grid        // CSS Grid container
};

enum class SizingMode {
    Fixed,      // Fixed px width/height
    Hug,        // Hug content (fit-content)
    Fill        // Fill container (flex-grow: 1 / 100% / 1fr)
};

enum class HorizontalConstraint {
    Left,
    Right,
    LeftRight,  // Stretches horizontally with parent
    Center,
    Scale
};

enum class VerticalConstraint {
    Top,
    Bottom,
    TopBottom,  // Stretches vertically with parent
    Center,
    Scale
};

enum class FlexDirection {
    Row,
    Column,
    RowReverse,
    ColumnReverse
};

enum class FlexWrap {
    NoWrap,
    Wrap,
    WrapReverse
};

enum class JustifyContent {
    FlexStart,
    FlexEnd,
    Center,
    SpaceBetween,
    SpaceAround,
    SpaceEvenly
};

enum class AlignItems {
    FlexStart,
    FlexEnd,
    Center,
    Stretch,
    Baseline
};

enum class AlignSelf {
    Auto,
    FlexStart,
    FlexEnd,
    Center,
    Stretch
};

enum class Breakpoint {
    Desktop,    // 1440px
    Laptop,     // 1024px
    Tablet,     // 768px
    Mobile      // 375px
};

struct Layout {
    LayoutMode mode = LayoutMode::None;

    // Sizing modes
    SizingMode widthMode  = SizingMode::Fixed;
    SizingMode heightMode = SizingMode::Fixed;
    double minWidth       = 0.0;
    double maxWidth       = 100000.0;
    double minHeight      = 0.0;
    double maxHeight      = 100000.0;

    // Constraints (when inside a parent container with LayoutMode::None)
    HorizontalConstraint horizontalConstraint = HorizontalConstraint::Left;
    VerticalConstraint   verticalConstraint   = VerticalConstraint::Top;

    // Padding (for container)
    double paddingTop    = 0.0;
    double paddingRight  = 0.0;
    double paddingBottom = 0.0;
    double paddingLeft   = 0.0;

    // Margin (for child items)
    double marginTop     = 0.0;
    double marginRight   = 0.0;
    double marginBottom  = 0.0;
    double marginLeft    = 0.0;

    // Flexbox container properties
    FlexDirection  flexDirection  = FlexDirection::Row;
    FlexWrap       flexWrap       = FlexWrap::NoWrap;
    JustifyContent justifyContent = JustifyContent::FlexStart;
    AlignItems     alignItems     = AlignItems::Stretch;
    double gap       = 0.0;
    double rowGap    = 0.0;
    double columnGap = 0.0;

    // Flex child item properties
    double    flexGrow   = 0.0;
    double    flexShrink = 1.0;
    double    flexBasis  = 0.0;
    AlignSelf alignSelf  = AlignSelf::Auto;

    // CSS Grid container properties
    int gridColumns     = 2;
    int gridRows        = 2;
    int gridColumnSpan  = 1;
    int gridRowSpan     = 1;
    int gridColumnStart = 0; // 0 = auto
    int gridRowStart    = 0; // 0 = auto
};

class Node {
public:
    using Id = uint64_t;

    Node(NodeType type, Id id, const std::string& name = "");

    Id id() const { return m_id; }
    NodeType type() const { return m_type; }
    const std::string& name() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    Transform& transform() { return m_transform; }
    const Transform& transform() const { return m_transform; }

    Style& style() { return m_style; }
    const Style& style() const { return m_style; }

    Layout& layout() { return m_layout; }
    const Layout& layout() const { return m_layout; }
    void setLayout(const Layout& l) { m_layout = l; }

    Node* parent() const { return m_parent; }
    void setParent(Node* parent);

    void addChild(std::unique_ptr<Node> child);
    std::unique_ptr<Node> removeChild(Id childId);
    Node* findChild(Id id) const;

    const std::vector<std::unique_ptr<Node>>& children() const { return m_children; }

    uint64_t componentId() const { return m_componentId; }
    void setComponentId(uint64_t id) { m_componentId = id; }

    std::unique_ptr<Node> clone(Id newId) const;

private:
    Id m_id;
    NodeType m_type;
    std::string m_name;
    Transform m_transform;
    Style m_style;
    Layout m_layout;
    Node* m_parent = nullptr;
    std::vector<std::unique_ptr<Node>> m_children;
    uint64_t m_componentId = 0;
};

} // namespace poliable
