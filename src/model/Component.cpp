#include "Component.h"
#include <algorithm>

namespace poliable {

Component::Component(uint64_t id, const std::string& name)
    : m_id(id), m_name(name)
{
}

void Component::setTemplateRoot(std::unique_ptr<Node> root) {
    m_templateRoot = std::move(root);
}

void Component::addVariantProperty(const VariantProperty& prop) {
    m_variantProperties.push_back(prop);
}

void Component::addSlot(const ComponentSlot& slot) {
    m_slots.push_back(slot);
}

std::unique_ptr<Node> Component::instantiate(uint64_t newInstanceId,
                                            const std::unordered_map<std::string, std::string>& /*variantValues*/) const {
    if (!m_templateRoot) {
        auto emptyNode = std::make_unique<Node>(NodeType::Instance, newInstanceId, m_name + " Instance");
        emptyNode->setComponentId(m_id);
        return emptyNode;
    }

    auto instanceNode = m_templateRoot->clone(newInstanceId);
    instanceNode->setName(m_name + " Instance");
    instanceNode->setComponentId(m_id);
    return instanceNode;
}

// ── ComponentManager ─────────────────────────────────────────────────────────

ComponentManager& ComponentManager::instance() {
    static ComponentManager inst;
    return inst;
}

void ComponentManager::registerComponent(std::unique_ptr<Component> component) {
    if (!component) return;
    // Replace if existing with same id
    removeComponent(component->id());
    m_components.push_back(std::move(component));
}

const Component* ComponentManager::getComponent(uint64_t id) const {
    for (const auto& c : m_components) {
        if (c->id() == id) return c.get();
    }
    return nullptr;
}

const Component* ComponentManager::findByName(const std::string& name) const {
    for (const auto& c : m_components) {
        if (c->name() == name) return c.get();
    }
    return nullptr;
}

void ComponentManager::removeComponent(uint64_t id) {
    m_components.erase(
        std::remove_if(m_components.begin(), m_components.end(),
                       [id](const std::unique_ptr<Component>& c) { return c->id() == id; }),
        m_components.end());
}

void ComponentManager::clear() {
    m_components.clear();
}

} // namespace poliable
