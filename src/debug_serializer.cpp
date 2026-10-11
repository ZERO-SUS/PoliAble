#include "model/Document.h"
#include "model/Serializer.h"
#include <iostream>

using namespace poliable;

int main() {
    Document doc1("OriginalDoc");
    auto rect = std::make_unique<Node>(NodeType::Rectangle, doc1.nextId(), "MyRect");
    rect->transform().x = 100;
    rect->transform().y = 200;
    doc1.addChild(0, std::move(rect));

    std::string serialized = Serializer::toString(doc1);
    std::cout << "Serialized:\n" << serialized << "\n\n";

    Document doc2;
    std::string error;
    bool success = Serializer::fromString(serialized, doc2, error);

    std::cout << "Parse success: " << success << "\n";
    if (!success) std::cout << "Error: " << error << "\n";
    std::cout << "Doc2 name: " << doc2.name() << "\n";
    std::cout << "Doc2 root children: " << doc2.root()->children().size() << "\n";

    if (!doc2.root()->children().empty()) {
        auto& child = doc2.root()->children()[0];
        std::cout << "Child name: " << child->name() << "\n";
        std::cout << "Child type: " << (int)child->type() << "\n";
    }

    return 0;
}
