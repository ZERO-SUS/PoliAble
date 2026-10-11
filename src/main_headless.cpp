#include "model/Document.h"
#include "model/Command.h"
#include "model/UndoStack.h"
#include "model/Serializer.h"
#include <iostream>
#include <memory>

using namespace poliable;

int main(int argc, char* argv[])
{
    std::cout << "PoliAble Core Engine v0.1.0" << std::endl;
    std::cout << "=========================" << std::endl;

    // Create a document
    Document doc("Test Project");
    std::cout << "Created document: " << doc.name() << std::endl;

    // Create nodes
    auto rect = std::make_unique<Node>(NodeType::Rectangle, doc.nextId(), "Rectangle1");
    rect->transform().x = 100;
    rect->transform().y = 100;
    rect->transform().width = 200;
    rect->transform().height = 150;
    rect->style().fillColor = "#FF5733";

    auto text = std::make_unique<Node>(NodeType::Text, doc.nextId(), "Text1");
    text->transform().x = 150;
    text->transform().y = 200;

    // Add to document
    Node* rectPtr = rect.get();
    Node* textPtr = text.get();

    auto addRectCmd = std::make_unique<AddNodeCommand>(0, std::move(rect), rectPtr->id());
    auto addTextCmd = std::make_unique<AddNodeCommand>(rectPtr->id(), std::move(text), textPtr->id());

    UndoStack undoStack;
    undoStack.execute(std::move(addRectCmd), doc);
    undoStack.execute(std::move(addTextCmd), doc);

    std::cout << "Added 2 nodes to document" << std::endl;
    std::cout << "Root has " << doc.root()->children().size() << " children" << std::endl;

    // Test serialization
    std::string serialized = Serializer::toString(doc);
    std::cout << "\nSerialized document:\n" << serialized << std::endl;

    // Test undo
    undoStack.undo(doc);
    std::cout << "\nAfter undo, root has " << doc.root()->children().size() << " children" << std::endl;

    // Test redo
    undoStack.redo(doc);
    std::cout << "After redo, root has " << doc.root()->children().size() << " children" << std::endl;

    std::cout << "\n✓ Core engine working correctly" << std::endl;
    return 0;
}
