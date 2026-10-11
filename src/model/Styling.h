#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace poliable {

// Gradients
enum class GradientType {
    None,
    Linear,
    Radial,
    Conic
};

struct ColorStop {
    std::string color;
    double offset = 0.0;  // 0.0 to 1.0
};

struct Gradient {
    GradientType type = GradientType::None;
    std::vector<ColorStop> stops;
    double angle = 0.0;  // For linear/conic gradients
    double cx = 0.5, cy = 0.5;  // For radial gradients (center)
    double r = 0.5;  // For radial gradients (radius)
};

// Shadow
struct Shadow {
    std::string color = "#000000";
    double offsetX = 0.0;
    double offsetY = 0.0;
    double blurRadius = 0.0;
    double spreadRadius = 0.0;
};

// Effects
struct Effects {
    std::vector<Shadow> shadows;
    double blur = 0.0;
    double brightness = 1.0;
    double contrast = 1.0;
    double saturation = 1.0;
    double hueRotate = 0.0;
    double opacity = 1.0;  // Blend opacity
};

// Typography
struct Typography {
    std::string fontFamily = "Arial";
    int fontSize = 16;
    double lineHeight = 1.4;
    std::string fontWeight = "400";  // 100-900 or "normal", "bold"
    std::string fontStyle = "normal";  // normal, italic
    std::string textAlign = "left";    // left, center, right
    std::string textDecoration = "none";  // none, underline, line-through
    double letterSpacing = 0.0;
    double wordSpacing = 0.0;
};

// Design Tokens
struct DesignToken {
    std::string id;
    std::string name;
    std::string value;
    std::string category;  // color, spacing, radius, typography, etc.
    std::string description;
};

// Style with Staging 5 features
struct Style {
    // Fill
    std::string fillColor = "#2d2d30";
    Gradient fillGradient;
    bool isFillGradient = false;

    // Stroke
    std::string strokeColor = "#888888";
    double strokeWidth = 1.0;
    std::string strokeStyle = "solid";  // solid, dashed, dotted

    // Corner Radius
    double cornerRadius = 0.0;
    double cornerRadiusTopLeft = 0.0;
    double cornerRadiusTopRight = 0.0;
    double cornerRadiusBottomLeft = 0.0;
    double cornerRadiusBottomRight = 0.0;

    // Typography (for Text nodes)
    std::string textContent = "";
    int fontSize = 16;
    Typography typography;

    // Effects
    Effects effects;

    // Background Image
    std::string backgroundImage = "";
    std::string backgroundSize = "cover";  // auto, cover, contain
    std::string backgroundPosition = "center";

    // Border
    struct Border {
        std::string color = "#888888";
        double width = 1.0;
        std::string style = "solid";
    };
    Border borderTop;
    Border borderRight;
    Border borderBottom;
    Border borderLeft;

    // Variables (reference to design tokens)
    std::unordered_map<std::string, std::string> variables;  // name -> token id
};

// Design Tokens Container
class DesignTokens {
public:
    static DesignTokens& instance();

    void addToken(const DesignToken& token);
    const DesignToken* getToken(const std::string& id) const;
    std::vector<DesignToken> getTokensByCategory(const std::string& category) const;
    void removeToken(const std::string& id);
    void clear();

private:
    DesignTokens() = default;
    std::unordered_map<std::string, DesignToken> m_tokens;
};

} // namespace poliable
