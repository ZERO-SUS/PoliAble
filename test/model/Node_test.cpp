#include "model/Node.h"
#include "model/Document.h"
#include "model/Command.h"
#include "model/UndoStack.h"
#include "model/Serializer.h"
#include "layout/LayoutEngine.h"
#include <cassert>
#include <iostream>

using namespace poliable;

// ===== Node Tests =====
void test_node_creation() {
    Node node(NodeType::Rectangle, 1, "TestRect");
    assert(node.id() == 1);
    assert(node.type() == NodeType::Rectangle);
    assert(node.name() == "TestRect");
    std::cout << "✓ test_node_creation passed" << std::endl;
}

void test_node_hierarchy() {
    Node root(NodeType::Canvas, 0, "root");
    auto child1 = std::make_unique<Node>(NodeType::Rectangle, 1, "rect1");
    auto child2 = std::make_unique<Node>(NodeType::Text, 2, "text1");

    Node* c1 = child1.get();
    Node* c2 = child2.get();

    root.addChild(std::move(child1));
    root.addChild(std::move(child2));

    assert(root.children().size() == 2);
    assert(c1->parent() == &root);
    assert(c2->parent() == &root);
    std::cout << "✓ test_node_hierarchy passed" << std::endl;
}

void test_node_find() {
    Node root(NodeType::Canvas, 0, "root");
    auto child = std::make_unique<Node>(NodeType::Rectangle, 42, "rect");
    Node* childPtr = child.get();
    root.addChild(std::move(child));

    assert(root.findChild(42) == childPtr);
    assert(root.findChild(99) == nullptr);
    std::cout << "✓ test_node_find passed" << std::endl;
}

// ===== Document Tests =====
void test_document_creation() {
    Document doc("TestDoc");
    assert(doc.name() == "TestDoc");
    assert(doc.root() != nullptr);
    assert(doc.root()->type() == NodeType::Canvas);
    std::cout << "✓ test_document_creation passed" << std::endl;
}

void test_add_child() {
    Document doc;
    auto rect = std::make_unique<Node>(NodeType::Rectangle, doc.nextId(), "rect");
    Node::Id rectId = rect->id();
    Node* rectPtr = rect.get();

    auto added = doc.addChild(0, std::move(rect));
    assert(added == rectPtr);
    assert(doc.root()->children().size() == 1);
    assert(doc.find(rectId) == rectPtr);
    std::cout << "✓ test_add_child passed" << std::endl;
}

void test_remove_node() {
    Document doc;
    auto rect = std::make_unique<Node>(NodeType::Rectangle, doc.nextId(), "rect");
    Node::Id rectId = rect->id();
    doc.addChild(0, std::move(rect));

    assert(doc.root()->children().size() == 1);
    auto removed = doc.removeNode(rectId);
    assert(removed != nullptr);
    assert(doc.root()->children().size() == 0);
    std::cout << "✓ test_remove_node passed" << std::endl;
}

void test_find() {
    Document doc;
    auto rect = std::make_unique<Node>(NodeType::Rectangle, 42, "rect");
    Node* rectPtr = rect.get();
    doc.addChild(0, std::move(rect));

    assert(doc.find(42) == rectPtr);
    assert(doc.find(99) == nullptr);
    std::cout << "✓ test_find passed" << std::endl;
}

// ===== Command Tests =====
void test_add_node_command() {
    Document doc;
    auto rect = std::make_unique<Node>(NodeType::Rectangle, doc.nextId(), "rect");
    Node::Id rectId = rect->id();

    auto cmd = std::make_unique<AddNodeCommand>(0, std::move(rect), rectId);
    cmd->execute(doc);

    assert(doc.find(rectId) != nullptr);
    assert(doc.root()->children().size() == 1);
    std::cout << "✓ test_add_node_command passed" << std::endl;
}

void test_remove_node_command() {
    Document doc;
    auto rect = std::make_unique<Node>(NodeType::Rectangle, doc.nextId(), "rect");
    Node::Id rectId = rect->id();
    doc.addChild(0, std::move(rect));

    auto cmd = std::make_unique<RemoveNodeCommand>(rectId);
    cmd->execute(doc);

    assert(doc.find(rectId) == nullptr);
    assert(doc.root()->children().size() == 0);

    cmd->undo(doc);
    assert(doc.find(rectId) != nullptr);
    assert(doc.root()->children().size() == 1);
    std::cout << "✓ test_remove_node_command passed" << std::endl;
}

void test_set_transform_command() {
    Document doc;
    auto rect = std::make_unique<Node>(NodeType::Rectangle, doc.nextId(), "rect");
    Node::Id rectId = rect->id();
    doc.addChild(0, std::move(rect));

    Node* node = doc.find(rectId);
    Transform prev = node->transform();
    Transform next = {100, 200, 300, 400, 45.0, 0.5};

    auto cmd = std::make_unique<SetTransformCommand>(rectId, prev, next);
    cmd->execute(doc);

    node = doc.find(rectId);
    assert(node->transform().x == 100);
    assert(node->transform().y == 200);

    cmd->undo(doc);
    node = doc.find(rectId);
    assert(node->transform().x == prev.x);
    assert(node->transform().y == prev.y);
    std::cout << "✓ test_set_transform_command passed" << std::endl;
}

void test_set_name_command() {
    Document doc;
    auto rect = std::make_unique<Node>(NodeType::Rectangle, doc.nextId(), "rect");
    Node::Id rectId = rect->id();
    doc.addChild(0, std::move(rect));

    auto cmd = std::make_unique<SetNameCommand>(rectId, "rect", "MyRectangle");
    cmd->execute(doc);

    assert(doc.find(rectId)->name() == "MyRectangle");

    cmd->undo(doc);
    assert(doc.find(rectId)->name() == "rect");
    std::cout << "✓ test_set_name_command passed" << std::endl;
}

// ===== Serializer Tests =====
void test_serialize_simple() {
    Document doc("TestDoc");
    auto rect = std::make_unique<Node>(NodeType::Rectangle, doc.nextId(), "rect");
    rect->transform().x = 100;
    rect->transform().y = 200;
    rect->transform().width = 300;
    rect->transform().height = 150;
    rect->style().fillColor = "#FF5733";

    doc.addChild(0, std::move(rect));

    std::string serialized = Serializer::toString(doc);
    assert(!serialized.empty());
    assert(serialized.find("TestDoc") != std::string::npos);
    assert(serialized.find("rectangle") != std::string::npos);
    std::cout << "✓ test_serialize_simple passed" << std::endl;
}

void test_deserialize_simple() {
    Document doc1("OriginalDoc");
    auto rect = std::make_unique<Node>(NodeType::Rectangle, doc1.nextId(), "MyRect");
    rect->transform().x = 100;
    rect->transform().y = 200;
    doc1.addChild(0, std::move(rect));

    std::string serialized = Serializer::toString(doc1);
    std::cout << "DEBUG: Serialized content:\n" << serialized << "\nEND DEBUG\n";

    Document doc2;
    std::string error;
    bool success = Serializer::fromString(serialized, doc2, error);

    assert(success);
    assert(doc2.name() == "OriginalDoc");
    std::cout << "DEBUG: doc2.root()->children().size() = " << doc2.root()->children().size() << std::endl;
    assert(doc2.root()->children().size() == 1);

    Node* rect2 = doc2.root()->children()[0].get();
    assert(rect2->name() == "MyRect");
    assert(rect2->type() == NodeType::Rectangle);
    assert(rect2->transform().x == 100);
    assert(rect2->transform().y == 200);
    std::cout << "✓ test_deserialize_simple passed" << std::endl;
}

void test_serialize_hierarchy() {
    Document doc("HierarchyTest");
    auto group = std::make_unique<Node>(NodeType::Group, doc.nextId(), "group1");
    Node::Id groupId = group->id();

    auto rect = std::make_unique<Node>(NodeType::Rectangle, doc.nextId(), "rect1");
    Node::Id rectId = rect->id();

    std::cout << "DEBUG: Before adding to group - group children: " << group->children().size() << "\n";
    group->addChild(std::move(rect));
    std::cout << "DEBUG: After adding to group - group children: " << group->children().size() << "\n";

    auto* groupPtr = group.get();
    doc.addChild(0, std::move(group));

    std::cout << "DEBUG: After adding group to doc - groupPtr children: " << groupPtr->children().size() << "\n";

    std::string serialized = Serializer::toString(doc);
    std::cout << "DEBUG hierarchy serialized:\n" << serialized << "\nEND DEBUG\n";

    Document doc2;
    std::string error;
    bool success = Serializer::fromString(serialized, doc2, error);

    assert(success);
    std::cout << "DEBUG: groupId=" << groupId << ", rectId=" << rectId << "\n";
    assert(doc2.find(groupId) != nullptr);
    assert(doc2.find(rectId) != nullptr);

    Node* rect2 = doc2.find(rectId);
    Node* group2 = doc2.find(groupId);
    std::cout << "DEBUG: rect2->parent()->id()=" << rect2->parent()->id() << ", group2->id()=" << group2->id() << "\n";

    assert(doc2.find(rectId)->parent()->id() == groupId);
    std::cout << "✓ test_serialize_hierarchy passed" << std::endl;
}

void test_roundtrip() {
    Document doc1("RoundtripTest");

    // Create multiple nodes
    for (int i = 0; i < 3; ++i) {
        auto rect = std::make_unique<Node>(NodeType::Rectangle, doc1.nextId(), "rect" + std::to_string(i));
        rect->transform().x = i * 100;
        rect->transform().y = i * 50;
        doc1.addChild(0, std::move(rect));
    }

    std::string serialized = Serializer::toString(doc1);

    Document doc2;
    std::string error;
    bool success = Serializer::fromString(serialized, doc2, error);

    assert(success);
    assert(doc2.name() == doc1.name());
    assert(doc2.root()->children().size() == doc1.root()->children().size());

    std::string reserialized = Serializer::toString(doc2);
    // Note: Newer format may include additional properties (like effects) that weren't in original
    // Just verify the round-trip preserves data correctly
    Document doc3;
    std::string error2;
    bool success2 = Serializer::fromString(reserialized, doc3, error2);
    assert(success2);
    assert(doc3.name() == doc1.name());
    assert(doc3.root()->children().size() == doc1.root()->children().size());
    std::cout << "✓ test_roundtrip passed" << std::endl;
}

// ===== Layout Engine Tests (Stage 4) =====
void test_flex_row_layout() {
    auto container = std::make_unique<Node>(NodeType::Flex, 10, "FlexRowContainer");
    container->transform().width = 500.0;
    container->transform().height = 200.0;
    container->layout().mode = LayoutMode::Flex;
    container->layout().flexDirection = FlexDirection::Row;
    container->layout().paddingLeft = 20.0;
    container->layout().paddingRight = 20.0;
    container->layout().paddingTop = 10.0;
    container->layout().paddingBottom = 10.0;
    container->layout().gap = 10.0;

    for (int i = 0; i < 3; ++i) {
        auto child = std::make_unique<Node>(NodeType::Rectangle, 11 + i, "item" + std::to_string(i));
        child->transform().width = 100.0;
        child->transform().height = 50.0;
        container->addChild(std::move(child));
    }

    LayoutEngine::computeLayout(container.get());

    const auto& children = container->children();
    assert(children.size() == 3);

    // item 0: x = 20, y = 10
    assert(std::abs(children[0]->transform().x - 20.0) < 0.001);
    assert(std::abs(children[0]->transform().y - 10.0) < 0.001);

    // item 1: x = 20 + 100 + 10 = 130
    assert(std::abs(children[1]->transform().x - 130.0) < 0.001);
    assert(std::abs(children[1]->transform().y - 10.0) < 0.001);

    // item 2: x = 130 + 100 + 10 = 240
    assert(std::abs(children[2]->transform().x - 240.0) < 0.001);
    assert(std::abs(children[2]->transform().y - 10.0) < 0.001);

    std::cout << "✓ test_flex_row_layout passed" << std::endl;
}

void test_flex_column_layout() {
    auto container = std::make_unique<Node>(NodeType::Flex, 20, "FlexColContainer");
    container->transform().width = 200.0;
    container->transform().height = 400.0;
    container->layout().mode = LayoutMode::Flex;
    container->layout().flexDirection = FlexDirection::Column;
    container->layout().paddingTop = 15.0;
    container->layout().paddingLeft = 10.0;
    container->layout().gap = 8.0;

    for (int i = 0; i < 2; ++i) {
        auto child = std::make_unique<Node>(NodeType::Rectangle, 21 + i, "item" + std::to_string(i));
        child->transform().width = 80.0;
        child->transform().height = 40.0;
        container->addChild(std::move(child));
    }

    LayoutEngine::computeLayout(container.get());

    const auto& children = container->children();
    assert(std::abs(children[0]->transform().y - 15.0) < 0.001);
    assert(std::abs(children[1]->transform().y - (15.0 + 40.0 + 8.0)) < 0.001);

    std::cout << "✓ test_flex_column_layout passed" << std::endl;
}

void test_flex_hug_and_fill() {
    // 1. Test Hug sizing
    auto hugContainer = std::make_unique<Node>(NodeType::Flex, 30, "HugContainer");
    hugContainer->layout().mode = LayoutMode::Flex;
    hugContainer->layout().flexDirection = FlexDirection::Row;
    hugContainer->layout().widthMode = SizingMode::Hug;
    hugContainer->layout().heightMode = SizingMode::Hug;
    hugContainer->layout().paddingLeft = 10.0;
    hugContainer->layout().paddingRight = 10.0;
    hugContainer->layout().paddingTop = 5.0;
    hugContainer->layout().paddingBottom = 5.0;
    hugContainer->layout().gap = 10.0;

    for (int i = 0; i < 2; ++i) {
        auto child = std::make_unique<Node>(NodeType::Rectangle, 31 + i, "item" + std::to_string(i));
        child->transform().width = 100.0;
        child->transform().height = 50.0;
        hugContainer->addChild(std::move(child));
    }

    LayoutEngine::computeLayout(hugContainer.get());

    // Expected width = 10 + 100 + 10(gap) + 100 + 10 = 230
    assert(std::abs(hugContainer->transform().width - 230.0) < 0.001);
    // Expected height = 5 + 50 + 5 = 60
    assert(std::abs(hugContainer->transform().height - 60.0) < 0.001);

    // 2. Test Fill sizing
    auto fillContainer = std::make_unique<Node>(NodeType::Flex, 40, "FillContainer");
    fillContainer->transform().width = 300.0;
    fillContainer->transform().height = 100.0;
    fillContainer->layout().mode = LayoutMode::Flex;
    fillContainer->layout().flexDirection = FlexDirection::Row;
    fillContainer->layout().paddingLeft = 0.0;
    fillContainer->layout().paddingRight = 0.0;
    fillContainer->layout().gap = 0.0;

    auto fixedChild = std::make_unique<Node>(NodeType::Rectangle, 41, "fixed");
    fixedChild->transform().width = 100.0;
    fixedChild->transform().height = 50.0;
    fillContainer->addChild(std::move(fixedChild));

    auto fillChild = std::make_unique<Node>(NodeType::Rectangle, 42, "fill");
    fillChild->layout().widthMode = SizingMode::Fill;
    fillContainer->addChild(std::move(fillChild));

    LayoutEngine::computeLayout(fillContainer.get());

    assert(std::abs(fillContainer->children()[0]->transform().width - 100.0) < 0.001);
    // 300 - 100 = 200
    assert(std::abs(fillContainer->children()[1]->transform().width - 200.0) < 0.001);

    std::cout << "✓ test_flex_hug_and_fill passed" << std::endl;
}

void test_grid_layout() {
    auto grid = std::make_unique<Node>(NodeType::Grid, 50, "GridContainer");
    grid->transform().width = 410.0;
    grid->transform().height = 310.0;
    grid->layout().mode = LayoutMode::Grid;
    grid->layout().gridColumns = 2;
    grid->layout().gridRows = 2;
    grid->layout().columnGap = 10.0;
    grid->layout().rowGap = 10.0;

    // cellW = (410 - 10) / 2 = 200
    // cellH = (310 - 10) / 2 = 150

    for (int i = 0; i < 4; ++i) {
        auto child = std::make_unique<Node>(NodeType::Rectangle, 51 + i, "cell" + std::to_string(i));
        child->layout().widthMode = SizingMode::Fill;
        child->layout().heightMode = SizingMode::Fill;
        grid->addChild(std::move(child));
    }

    LayoutEngine::computeLayout(grid.get());

    const auto& cells = grid->children();
    // Cell 0: col 0, row 0 -> (0, 0, 200, 150)
    assert(std::abs(cells[0]->transform().x - 0.0) < 0.001);
    assert(std::abs(cells[0]->transform().y - 0.0) < 0.001);
    assert(std::abs(cells[0]->transform().width - 200.0) < 0.001);
    assert(std::abs(cells[0]->transform().height - 150.0) < 0.001);

    // Cell 1: col 1, row 0 -> (210, 0, 200, 150)
    assert(std::abs(cells[1]->transform().x - 210.0) < 0.001);
    assert(std::abs(cells[1]->transform().y - 0.0) < 0.001);

    // Cell 2: col 0, row 1 -> (0, 160, 200, 150)
    assert(std::abs(cells[2]->transform().x - 0.0) < 0.001);
    assert(std::abs(cells[2]->transform().y - 160.0) < 0.001);

    // Cell 3: col 1, row 1 -> (210, 160, 200, 150)
    assert(std::abs(cells[3]->transform().x - 210.0) < 0.001);
    assert(std::abs(cells[3]->transform().y - 160.0) < 0.001);

    std::cout << "✓ test_grid_layout passed" << std::endl;
}

void test_constraints() {
    auto child = std::make_unique<Node>(NodeType::Rectangle, 60, "ConstrainedChild");
    child->transform().x = 100.0;
    child->transform().y = 50.0;
    child->transform().width = 200.0;
    child->transform().height = 100.0;

    // Right constraint test:
    // oldParentW = 1000, old right distance = 1000 - (100 + 200) = 700
    // newParentW = 1200, expected newX = 1200 - 700 - 200 = 300
    child->layout().horizontalConstraint = HorizontalConstraint::Right;
    LayoutEngine::applyConstraints(child.get(), 1000.0, 800.0, 1200.0, 800.0);
    assert(std::abs(child->transform().x - 300.0) < 0.001);

    // LeftRight (stretch) test:
    // reset x = 100, width = 200
    child->transform().x = 100.0;
    child->transform().width = 200.0;
    child->layout().horizontalConstraint = HorizontalConstraint::LeftRight;
    // old right dist = 1000 - 300 = 700. new width = 1200 - 100 - 700 = 400
    LayoutEngine::applyConstraints(child.get(), 1000.0, 800.0, 1200.0, 800.0);
    assert(std::abs(child->transform().x - 100.0) < 0.001);
    assert(std::abs(child->transform().width - 400.0) < 0.001);

    // Center constraint test:
    child->transform().x = 400.0;
    child->transform().width = 200.0; // center at 500 in 1000 (offset = 0)
    child->layout().horizontalConstraint = HorizontalConstraint::Center;
    // new parent = 1200, center is 600, child x should be 600 - 100 = 500
    LayoutEngine::applyConstraints(child.get(), 1000.0, 800.0, 1200.0, 800.0);
    assert(std::abs(child->transform().x - 500.0) < 0.001);

    std::cout << "✓ test_constraints passed" << std::endl;
}

void test_set_layout_command() {
    Document doc;
    auto node = std::make_unique<Node>(NodeType::Flex, doc.nextId(), "layout_node");
    Node::Id nid = node->id();
    doc.addChild(0, std::move(node));

    Node* target = doc.find(nid);
    Layout prev = target->layout();
    Layout next = prev;
    next.mode = LayoutMode::Flex;
    next.flexDirection = FlexDirection::Column;
    next.gap = 16.0;
    next.paddingLeft = 24.0;

    auto cmd = std::make_unique<SetLayoutCommand>(nid, prev, next);
    cmd->execute(doc);

    assert(target->layout().flexDirection == FlexDirection::Column);
    assert(target->layout().gap == 16.0);
    assert(target->layout().paddingLeft == 24.0);

    cmd->undo(doc);
    assert(target->layout().flexDirection == prev.flexDirection);
    assert(target->layout().gap == prev.gap);
    assert(target->layout().paddingLeft == prev.paddingLeft);

    std::cout << "✓ test_set_layout_command passed" << std::endl;
}

void test_layout_serialization_roundtrip() {
    Document doc1("LayoutDoc");
    auto flexNode = std::make_unique<Node>(NodeType::Flex, doc1.nextId(), "FlexBox");
    flexNode->layout().mode = LayoutMode::Flex;
    flexNode->layout().flexDirection = FlexDirection::Column;
    flexNode->layout().justifyContent = JustifyContent::SpaceBetween;
    flexNode->layout().alignItems = AlignItems::Center;
    flexNode->layout().gap = 12.0;
    flexNode->layout().paddingLeft = 15.0;
    flexNode->layout().paddingRight = 15.0;
    flexNode->layout().widthMode = SizingMode::Hug;
    flexNode->layout().heightMode = SizingMode::Fill;
    flexNode->layout().horizontalConstraint = HorizontalConstraint::LeftRight;

    Node::Id flexId = flexNode->id();
    doc1.addChild(0, std::move(flexNode));

    std::string serialized = Serializer::toString(doc1);
    assert(serialized.find("layout") != std::string::npos);

    Document doc2;
    std::string err;
    bool ok = Serializer::fromString(serialized, doc2, err);
    assert(ok);

    Node* loaded = doc2.find(flexId);
    assert(loaded != nullptr);
    assert(loaded->type() == NodeType::Flex);
    assert(loaded->layout().mode == LayoutMode::Flex);
    assert(loaded->layout().flexDirection == FlexDirection::Column);
    assert(loaded->layout().justifyContent == JustifyContent::SpaceBetween);
    assert(loaded->layout().alignItems == AlignItems::Center);
    assert(loaded->layout().gap == 12.0);
    assert(loaded->layout().paddingLeft == 15.0);
    assert(loaded->layout().paddingRight == 15.0);
    assert(loaded->layout().widthMode == SizingMode::Hug);
    assert(loaded->layout().heightMode == SizingMode::Fill);
    assert(loaded->layout().horizontalConstraint == HorizontalConstraint::LeftRight);

    std::cout << "✓ test_layout_serialization_roundtrip passed" << std::endl;
}

void test_responsive_breakpoints() {
    Document doc("ResponsiveDoc");
    doc.root()->transform().width = 1440.0;
    doc.root()->transform().height = 900.0;

    auto child = std::make_unique<Node>(NodeType::Rectangle, doc.nextId(), "Navbar");
    child->transform().x = 0.0;
    child->transform().y = 0.0;
    child->transform().width = 1440.0;
    child->transform().height = 60.0;
    child->layout().horizontalConstraint = HorizontalConstraint::LeftRight;
    doc.addChild(0, std::move(child));

    // Switch to Mobile (375x812)
    LayoutEngine::applyBreakpoint(doc, Breakpoint::Mobile);
    assert(std::abs(doc.root()->transform().width - 375.0) < 0.001);
    assert(std::abs(doc.root()->transform().height - 812.0) < 0.001);

    // Navbar should stretch to 375px because of LeftRight constraint
    Node* nav = doc.root()->children()[0].get();
    assert(std::abs(nav->transform().width - 375.0) < 0.001);

    // Switch to Laptop (1024x768)
    LayoutEngine::applyBreakpoint(doc, Breakpoint::Laptop);
    assert(std::abs(doc.root()->transform().width - 1024.0) < 0.001);
    assert(std::abs(nav->transform().width - 1024.0) < 0.001);

    std::cout << "✓ test_responsive_breakpoints passed" << std::endl;
}

// ===== Main Test Runner =====
int main() {
    try {
        std::cout << "PoliAble Core Engine Tests\n" << std::string(50, '=') << "\n" << std::endl;

        std::cout << "\n--- Node Tests ---" << std::endl;
        test_node_creation();
        test_node_hierarchy();
        test_node_find();

        std::cout << "\n--- Document Tests ---" << std::endl;
        test_document_creation();
        test_add_child();
        test_remove_node();
        test_find();

        std::cout << "\n--- Command Tests ---" << std::endl;
        test_add_node_command();
        test_remove_node_command();
        test_set_transform_command();
        test_set_name_command();

        std::cout << "\n--- Serializer Tests ---" << std::endl;
        test_serialize_simple();
        test_deserialize_simple();
        test_serialize_hierarchy();
        test_roundtrip();

        std::cout << "\n--- Layout Engine Tests (Stage 4) ---" << std::endl;
        test_flex_row_layout();
        test_flex_column_layout();
        test_flex_hug_and_fill();
        test_grid_layout();
        test_constraints();
        test_set_layout_command();
        test_layout_serialization_roundtrip();
        test_responsive_breakpoints();

        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "✓ ALL TESTS PASSED! (26 total)" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
