#pragma once
#include "Document.h"
#include <string>

namespace poliable {

class Command {
public:
    virtual ~Command() = default;
    virtual void execute(Document& doc) = 0;
    virtual void undo(Document& doc) = 0;
    virtual const char* name() const = 0;
};

class AddNodeCommand : public Command {
public:
    AddNodeCommand(Node::Id parentId, std::unique_ptr<Node> child, Node::Id assignedId);
    void execute(Document& doc) override;
    void undo(Document& doc) override;
    const char* name() const override { return "AddNode"; }
private:
    Node::Id m_parentId;
    std::unique_ptr<Node> m_child;
    Node::Id m_assignedId;
};

class RemoveNodeCommand : public Command {
public:
    explicit RemoveNodeCommand(Node::Id id);
    void execute(Document& doc) override;
    void undo(Document& doc) override;
    const char* name() const override { return "RemoveNode"; }
private:
    Node::Id m_id;
    std::unique_ptr<Node> m_removed;
    Node::Id m_parentId = 0;
};

class SetTransformCommand : public Command {
public:
    SetTransformCommand(Node::Id id, Transform prev, Transform next);
    void execute(Document& doc) override;
    void undo(Document& doc) override;
    const char* name() const override { return "SetTransform"; }
private:
    Node::Id m_id;
    Transform m_prev;
    Transform m_next;
};

class SetNameCommand : public Command {
public:
    SetNameCommand(Node::Id id, std::string prev, std::string next);
    void execute(Document& doc) override;
    void undo(Document& doc) override;
    const char* name() const override { return "SetName"; }
private:
    Node::Id m_id;
    std::string m_prev;
    std::string m_next;
};

class SetStyleCommand : public Command {
public:
    SetStyleCommand(Node::Id id, Style prev, Style next);
    void execute(Document& doc) override;
    void undo(Document& doc) override;
    const char* name() const override { return "SetStyle"; }
private:
    Node::Id m_id;
    Style m_prev;
    Style m_next;
};

class SetLayoutCommand : public Command {
public:
    SetLayoutCommand(Node::Id id, Layout prev, Layout next);
    void execute(Document& doc) override;
    void undo(Document& doc) override;
    const char* name() const override { return "SetLayout"; }
private:
    Node::Id m_id;
    Layout m_prev;
    Layout m_next;
};

class SetDesignTokensCommand : public Command {
public:
    SetDesignTokensCommand(const std::unordered_map<std::string, DesignToken>& prev, const std::unordered_map<std::string, DesignToken>& next);
    void execute(Document& doc) override;
    void undo(Document& doc) override;
    const char* name() const override { return "SetDesignTokens"; }
private:
    std::unordered_map<std::string, DesignToken> m_prev;
    std::unordered_map<std::string, DesignToken> m_next;
};

} // namespace poliable
