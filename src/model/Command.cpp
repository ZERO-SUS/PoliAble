#include "Command.h"
#include "Styling.h"

namespace poliable {

AddNodeCommand::AddNodeCommand(Node::Id parentId, std::unique_ptr<Node> child, Node::Id assignedId)
    : m_parentId(parentId), m_child(std::move(child)), m_assignedId(assignedId) {}

void AddNodeCommand::execute(Document& doc) {
    if (m_child) {
        doc.addChild(m_parentId, std::move(m_child));
    }
}

void AddNodeCommand::undo(Document& doc) {
    if (auto* node = doc.find(m_assignedId)) {
        m_child = doc.removeNode(m_assignedId);
    }
}

RemoveNodeCommand::RemoveNodeCommand(Node::Id id) : m_id(id) {}

void RemoveNodeCommand::execute(Document& doc) {
    if (auto* node = doc.find(m_id)) {
        if (auto* parent = node->parent()) {
            m_parentId = parent->id();
            m_removed = doc.removeNode(m_id);
        }
    }
}

void RemoveNodeCommand::undo(Document& doc) {
    if (m_removed) {
        doc.addChild(m_parentId, std::move(m_removed));
    }
}

SetTransformCommand::SetTransformCommand(Node::Id id, Transform prev, Transform next)
    : m_id(id), m_prev(prev), m_next(next) {}

void SetTransformCommand::execute(Document& doc) {
    if (auto* node = doc.find(m_id)) {
        node->transform() = m_next;
    }
}

void SetTransformCommand::undo(Document& doc) {
    if (auto* node = doc.find(m_id)) {
        node->transform() = m_prev;
    }
}

SetNameCommand::SetNameCommand(Node::Id id, std::string prev, std::string next)
    : m_id(id), m_prev(std::move(prev)), m_next(std::move(next)) {}

void SetNameCommand::execute(Document& doc) {
    if (auto* node = doc.find(m_id)) {
        node->setName(m_next);
    }
}

void SetNameCommand::undo(Document& doc) {
    if (auto* node = doc.find(m_id)) {
        node->setName(m_prev);
    }
}

SetStyleCommand::SetStyleCommand(Node::Id id, Style prev, Style next)
    : m_id(id), m_prev(std::move(prev)), m_next(std::move(next)) {}

void SetStyleCommand::execute(Document& doc) {
    if (auto* node = doc.find(m_id)) {
        node->style() = m_next;
    }
}

void SetStyleCommand::undo(Document& doc) {
    if (auto* node = doc.find(m_id)) {
        node->style() = m_prev;
    }
}

SetLayoutCommand::SetLayoutCommand(Node::Id id, Layout prev, Layout next)
    : m_id(id), m_prev(std::move(prev)), m_next(std::move(next)) {}

void SetLayoutCommand::execute(Document& doc) {
    if (auto* node = doc.find(m_id)) {
        node->layout() = m_next;
    }
}

void SetLayoutCommand::undo(Document& doc) {
    if (auto* node = doc.find(m_id)) {
        node->layout() = m_prev;
    }
}

SetDesignTokensCommand::SetDesignTokensCommand(
    const std::unordered_map<std::string, DesignToken>& prev,
    const std::unordered_map<std::string, DesignToken>& next)
    : m_prev(prev), m_next(next) {}

void SetDesignTokensCommand::execute(Document& doc) {
    DesignTokens::instance().clear();
    for (const auto& [id, token] : m_next) {
        DesignTokens::instance().addToken(token);
    }
}

void SetDesignTokensCommand::undo(Document& doc) {
    DesignTokens::instance().clear();
    for (const auto& [id, token] : m_prev) {
        DesignTokens::instance().addToken(token);
    }
}

} // namespace poliable
