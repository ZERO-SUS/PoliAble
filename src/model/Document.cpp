#include "Document.h"
#include <cassert>

namespace poliable {

Document::Document() : Document("Untitled") {}

Document::Document(const std::string& name)
    : m_name(name)
    , m_root(std::make_unique<Node>(NodeType::Canvas, 0, "root"))
{
    m_nextId = 1;
}

Document::~Document() = default;

Document::Document(Document&& other) noexcept
    : m_metadata(other.m_metadata)
    , m_name(std::move(other.m_name))
    , m_root(std::move(other.m_root))
    , m_nextId(other.m_nextId)
{}

Document& Document::operator=(Document&& other) noexcept {
    if (this != &other) {
        m_metadata = other.m_metadata;
        m_name = std::move(other.m_name);
        m_root = std::move(other.m_root);
        m_nextId = other.m_nextId;
    }
    return *this;
}

Node* Document::find(Id id) const {
    if (m_root->id() == id) return m_root.get();
    return m_root->findChild(id);
}

Node* Document::addChild(Id parentId, std::unique_ptr<Node> child) {
    Node* parent = find(parentId);
    if (!parent) return nullptr;
    Node* raw = child.get();
    reserveId(child->id());
    parent->addChild(std::move(child));
    return raw;
}

std::unique_ptr<Node> Document::removeNode(Id id) {
    Node* target = find(id);
    if (!target || !target->parent()) return nullptr;
    return target->parent()->removeChild(id);
}

} // namespace poliable
