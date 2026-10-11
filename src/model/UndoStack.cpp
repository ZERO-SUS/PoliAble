#include "UndoStack.h"

namespace poliable {

void UndoStack::execute(std::unique_ptr<Command> cmd, Document& doc) {
    if (!cmd) return;
    cmd->execute(doc);
    m_undo.push_back(std::move(cmd));
    m_redo.clear();
}

void UndoStack::undo(Document& doc) {
    if (m_undo.empty()) return;
    auto cmd = std::move(m_undo.back());
    m_undo.pop_back();
    cmd->undo(doc);
    m_redo.push_back(std::move(cmd));
}

void UndoStack::redo(Document& doc) {
    if (m_redo.empty()) return;
    auto cmd = std::move(m_redo.back());
    m_redo.pop_back();
    cmd->execute(doc);
    m_undo.push_back(std::move(cmd));
}

void UndoStack::execute(std::unique_ptr<Command> cmd) {
    if (!cmd || !m_doc) return;
    cmd->execute(*m_doc);
    m_undo.push_back(std::move(cmd));
    m_redo.clear();
}

void UndoStack::clear() {
    m_undo.clear();
    m_redo.clear();
}

} // namespace poliable
