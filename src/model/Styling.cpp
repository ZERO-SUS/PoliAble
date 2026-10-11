#include "Styling.h"

namespace poliable {

// DesignTokens implementation
DesignTokens& DesignTokens::instance() {
    static DesignTokens instance;
    return instance;
}

void DesignTokens::addToken(const DesignToken& token) {
    m_tokens[token.id] = token;
}

const DesignToken* DesignTokens::getToken(const std::string& id) const {
    auto it = m_tokens.find(id);
    if (it != m_tokens.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<DesignToken> DesignTokens::getTokensByCategory(const std::string& category) const {
    std::vector<DesignToken> result;
    for (const auto& [id, token] : m_tokens) {
        if (token.category == category) {
            result.push_back(token);
        }
    }
    return result;
}

void DesignTokens::removeToken(const std::string& id) {
    m_tokens.erase(id);
}

void DesignTokens::clear() {
    m_tokens.clear();
}

} // namespace poliable
