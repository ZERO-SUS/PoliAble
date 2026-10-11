#pragma once
#include "Command.h"
#include <memory>
#include <vector>
#include <functional>

namespace poliable { class Document; }

namespace poliable {

class UndoStack {
public:
    void execute(std::unique_ptr<Command> cmd, Document& doc);
    void undo(Document& doc);
    void redo(Document& doc);

    bool canUndo() const { return !m_undo.empty(); }
    bool canRedo() const { return !m_redo.empty(); }

    size_t undoCount() const { return m_undo.size(); }
    size_t redoCount() const { return m_redo.size(); }

    void clear();

    // Set the document reference for execute() to use
    void setDocument(Document* doc) { m_doc = doc; }

    // Execute command using stored document (for CanvasItem use)
    void execute(std::unique_ptr<Command> cmd);

    // Get the document (for CanvasItem use)
    Document* document() const { return m_doc; }

private:
    std::vector<std::unique_ptr<Command>> m_undo;
    std::vector<std::unique_ptr<Command>> m_redo;
    Document* m_doc = nullptr;
};

} // namespace poliable
