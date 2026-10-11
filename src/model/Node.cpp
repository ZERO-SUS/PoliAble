#include "Node.h"
#include <algorithm>
#include <cassert>

namespace poliable {

Node::Node(NodeType type, Id id, const std::string& name)
    : m_id(id), m_type(type), m_name(name)
{
    if (type == NodeType::Flex) {
        m_layout.mode = LayoutMode::Flex;
    } else if (type == NodeType::Grid) {
        m_layout.mode = LayoutMode::Grid;
    }
}

void Node::setParent(Node* parent) {
    if (m_parent == parent) return;
    m_parent = parent;
}

void Node::addChild(std::unique_ptr<Node> child) {
    if (!child) return;
    child->setParent(this);
    m_children.push_back(std::move(child));
}

std::unique_ptr<Node> Node::removeChild(Id childId) {
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        if ((*it)->id() == childId) {
            (*it)->setParent(nullptr);
            std::unique_ptr<Node> result = std::move(*it);
            m_children.erase(it);
            return result;
        }
    }
    return nullptr;
}

Node* Node::findChild(Id id) const {
    for (auto& c : m_children) {
        if (c->id() == id) return c.get();
        if (auto* found = c->findChild(id)) return found;
    }
    return nullptr;
}

std::unique_ptr<Node> Node::clone(Id newId) const {
    auto copy = std::make_unique<Node>(m_type, newId, m_name);
    copy->transform() = m_transform;
    copy->style() = m_style;
    copy->setLayout(m_layout);
    copy->setComponentId(m_componentId);

    // Recursively clone children (assigning unique sub-ids or offset ids)
    Id childSubId = newId * 1000 + 1;
    for (const auto& child : m_children) {
        copy->addChild(child->clone(childSubId++));
    }
    return copy;
}

} // namespace poliable
