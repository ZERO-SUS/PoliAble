#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "model/Node.h"

namespace poliable {

// Variant definition (e.g. variant="primary", size="lg")
struct VariantProperty {
    std::string name;
    std::vector<std::string> values;
    std::string defaultValue;
};

// Component slot (named insertion point)
struct ComponentSlot {
    std::string slotId;
    std::string slotName;
};

// Component State
enum class ComponentState {
    Default,
    Hover,
    Active,
    Disabled,
    Focus
};

// Master Component Definition
class Component {
public:
    Component(uint64_t id, const std::string& name);

    uint64_t id() const { return m_id; }
    const std::string& name() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    const Node* templateRoot() const { return m_templateRoot.get(); }
    void setTemplateRoot(std::unique_ptr<Node> root);

    // Variants
    void addVariantProperty(const VariantProperty& prop);
    const std::vector<VariantProperty>& variantProperties() const { return m_variantProperties; }

    // Slots
    void addSlot(const ComponentSlot& slot);
    const std::vector<ComponentSlot>& slots() const { return m_slots; }

    // Instantiate an instance node linked to this component
    std::unique_ptr<Node> instantiate(uint64_t newInstanceId,
                                      const std::unordered_map<std::string, std::string>& variantValues = {}) const;

private:
    uint64_t m_id;
    std::string m_name;
    std::unique_ptr<Node> m_templateRoot;
    std::vector<VariantProperty> m_variantProperties;
    std::vector<ComponentSlot> m_slots;
};

// Component Manager (Singleton registry)
class ComponentManager {
public:
    static ComponentManager& instance();

    void registerComponent(std::unique_ptr<Component> component);
    const Component* getComponent(uint64_t id) const;
    const Component* findByName(const std::string& name) const;
    const std::vector<std::unique_ptr<Component>>& components() const { return m_components; }
    void removeComponent(uint64_t id);
    void clear();

private:
    ComponentManager() = default;
    std::vector<std::unique_ptr<Component>> m_components;
};

} // namespace poliable
