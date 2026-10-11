#pragma once
#include "model/Styling.h"
#include "Node.h"
#include <memory>
#include <string>
#include <cstdint>

namespace poliable {

struct DocumentMetadata {
    uint32_t versionMajor = 1;
    uint32_t versionMinor = 0;
    uint32_t versionPatch = 0;
    std::string schemaId = "poliabledoc.v1";
    std::string generator = "PoliAble";
};

class Document {
public:
    using Id = uint64_t;

    Document();
    explicit Document(const std::string& name);
    ~Document();
    Document(Document&&) noexcept;
    Document& operator=(Document&&) noexcept;

    const DocumentMetadata& metadata() const { return m_metadata; }
    DocumentMetadata& metadata() { return m_metadata; }
    void setMetadata(const DocumentMetadata& m) { m_metadata = m; }

    const std::string& name() const { return m_name; }
    void setName(const std::string& n) { m_name = n; }

    Node* root() const { return m_root.get(); }
    Node* find(Id id) const;

    Node* addChild(Id parentId, std::unique_ptr<Node> child);
    std::unique_ptr<Node> removeNode(Id id);

    Id nextId() { return m_nextId++; }
    void reserveId(Id id) { if (id >= m_nextId) m_nextId = id + 1; }

private:
    DocumentMetadata m_metadata;
    std::string m_name;
    std::unique_ptr<Node> m_root;
    mutable Id m_nextId = 1;
};

} // namespace poliable
