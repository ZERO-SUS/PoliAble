#include "layout/LayoutEngine.h"
#include <algorithm>
#include <numeric>
#include <vector>
#include <cmath>

namespace poliable {

void LayoutEngine::getBreakpointDimensions(Breakpoint bp, double& outW, double& outH) {
    switch (bp) {
        case Breakpoint::Desktop:
            outW = 1440.0; outH = 900.0; break;
        case Breakpoint::Laptop:
            outW = 1024.0; outH = 768.0; break;
        case Breakpoint::Tablet:
            outW = 768.0;  outH = 1024.0; break;
        case Breakpoint::Mobile:
            outW = 375.0;  outH = 812.0; break;
    }
}

void LayoutEngine::computeDocumentLayout(Document& doc) {
    if (doc.root()) {
        computeLayout(doc.root());
    }
}

void LayoutEngine::computeLayout(Node* node) {
    if (!node) return;

    // Step 1: Recursively compute children layout first (bottom-up)
    for (const auto& child : node->children()) {
        computeLayout(child.get());
    }

    // Step 2: If this node is a layout container, compute positions/sizes of direct children
    bool isFlex = (node->layout().mode == LayoutMode::Flex) || (node->type() == NodeType::Flex);
    bool isGrid = (node->layout().mode == LayoutMode::Grid) || (node->type() == NodeType::Grid);

    if (isFlex) {
        computeFlexLayout(node);
    } else if (isGrid) {
        computeGridLayout(node);
    }
}

void LayoutEngine::computeFlexLayout(Node* container) {
    const auto& layout = container->layout();
    const auto& children = container->children();
    if (children.empty()) {
        if (layout.widthMode == SizingMode::Hug) {
            container->transform().width = std::max(layout.minWidth, layout.paddingLeft + layout.paddingRight);
        }
        if (layout.heightMode == SizingMode::Hug) {
            container->transform().height = std::max(layout.minHeight, layout.paddingTop + layout.paddingBottom);
        }
        return;
    }

    bool isRow = (layout.flexDirection == FlexDirection::Row || layout.flexDirection == FlexDirection::RowReverse);
    bool isReverse = (layout.flexDirection == FlexDirection::RowReverse || layout.flexDirection == FlexDirection::ColumnReverse);

    double padStart = isRow ? layout.paddingLeft : layout.paddingTop;
    double padEnd   = isRow ? layout.paddingRight : layout.paddingBottom;
    double padCrossStart = isRow ? layout.paddingTop : layout.paddingLeft;
    double padCrossEnd   = isRow ? layout.paddingBottom : layout.paddingRight;

    double mainGap = (layout.gap > 0.0) ? layout.gap : (isRow ? layout.columnGap : layout.rowGap);
    double crossGap = (layout.gap > 0.0) ? layout.gap : (isRow ? layout.rowGap : layout.columnGap);

    double containerMainSize = isRow ? container->transform().width : container->transform().height;
    double containerCrossSize = isRow ? container->transform().height : container->transform().width;

    double availMain = std::max(0.0, containerMainSize - padStart - padEnd);
    double availCross = std::max(0.0, containerCrossSize - padCrossStart - padCrossEnd);

    // Filter children into lines if wrapping, or single line
    struct Line {
        std::vector<Node*> items;
        double mainSize = 0.0;
        double crossSize = 0.0;
    };
    std::vector<Line> lines;
    lines.push_back(Line{});

    // 1. Initial measurement & line splitting
    for (const auto& childPtr : children) {
        Node* child = childPtr.get();
        auto& ct = child->transform();
        const auto& cl = child->layout();

        // Clamp sizes within child limits
        ct.width = std::clamp(ct.width, cl.minWidth, cl.maxWidth);
        ct.height = std::clamp(ct.height, cl.minHeight, cl.maxHeight);

        double childMain = isRow ? ct.width : ct.height;
        double childMarginMain = isRow ? (cl.marginLeft + cl.marginRight) : (cl.marginTop + cl.marginBottom);
        double totalItemMain = childMain + childMarginMain;

        if (layout.flexWrap == FlexWrap::Wrap && !lines.back().items.empty()) {
            double currentLineWithNewItem = lines.back().mainSize + mainGap + totalItemMain;
            if (currentLineWithNewItem > availMain && layout.widthMode != SizingMode::Hug) {
                // Start a new line
                lines.push_back(Line{});
            }
        }

        Line& curLine = lines.back();
        if (!curLine.items.empty()) {
            curLine.mainSize += mainGap;
        }
        curLine.mainSize += totalItemMain;
        curLine.items.push_back(child);
    }

    // 2. Resolve Fill / flex-grow and cross size for each line
    for (auto& line : lines) {
        double fixedMain = 0.0;
        double totalFlexGrow = 0.0;
        int fillCount = 0;

        for (Node* child : line.items) {
            const auto& cl = child->layout();
            bool isFillMain = isRow ? (cl.widthMode == SizingMode::Fill) : (cl.heightMode == SizingMode::Fill);
            double childMarginMain = isRow ? (cl.marginLeft + cl.marginRight) : (cl.marginTop + cl.marginBottom);

            if (isFillMain && (layout.widthMode != SizingMode::Hug || !isRow)) {
                double grow = cl.flexGrow > 0.0 ? cl.flexGrow : 1.0;
                totalFlexGrow += grow;
                fillCount++;
                fixedMain += childMarginMain;
            } else {
                double childMain = isRow ? child->transform().width : child->transform().height;
                fixedMain += childMain + childMarginMain;
            }
        }

        double totalGaps = std::max(0, (int)line.items.size() - 1) * mainGap;
        double freeSpace = std::max(0.0, availMain - fixedMain - totalGaps);

        // Allocate free space to fill items
        if (totalFlexGrow > 0.0 && freeSpace > 0.0) {
            for (Node* child : line.items) {
                const auto& cl = child->layout();
                bool isFillMain = isRow ? (cl.widthMode == SizingMode::Fill) : (cl.heightMode == SizingMode::Fill);
                if (isFillMain) {
                    double grow = cl.flexGrow > 0.0 ? cl.flexGrow : 1.0;
                    double allocated = (grow / totalFlexGrow) * freeSpace;
                    if (isRow) {
                        child->transform().width = std::clamp(allocated, cl.minWidth, cl.maxWidth);
                    } else {
                        child->transform().height = std::clamp(allocated, cl.minHeight, cl.maxHeight);
                    }
                }
            }
        }

        // Recalculate line main size and line max cross size
        line.mainSize = 0.0;
        line.crossSize = 0.0;
        for (size_t i = 0; i < line.items.size(); ++i) {
            Node* child = line.items[i];
            const auto& cl = child->layout();

            // Resolve cross size if Stretch or Fill
            AlignSelf align = (cl.alignSelf != AlignSelf::Auto) ? cl.alignSelf : static_cast<AlignSelf>(layout.alignItems);
            bool isFillCross = isRow ? (cl.heightMode == SizingMode::Fill) : (cl.widthMode == SizingMode::Fill);

            if ((align == AlignSelf::Stretch || isFillCross) && (isRow ? layout.heightMode != SizingMode::Hug : layout.widthMode != SizingMode::Hug)) {
                if (isRow) {
                    double crossVal = availCross - cl.marginTop - cl.marginBottom;
                    child->transform().height = std::clamp(crossVal, cl.minHeight, cl.maxHeight);
                } else {
                    double crossVal = availCross - cl.marginLeft - cl.marginRight;
                    child->transform().width = std::clamp(crossVal, cl.minWidth, cl.maxWidth);
                }
            }

            double childMain = isRow ? child->transform().width : child->transform().height;
            double childMarginMain = isRow ? (cl.marginLeft + cl.marginRight) : (cl.marginTop + cl.marginBottom);
            double childCross = isRow ? child->transform().height : child->transform().width;
            double childMarginCross = isRow ? (cl.marginTop + cl.marginBottom) : (cl.marginLeft + cl.marginRight);

            line.mainSize += childMain + childMarginMain;
            if (i > 0) line.mainSize += mainGap;
            line.crossSize = std::max(line.crossSize, childCross + childMarginCross);
        }
    }

    // 3. Hug container sizing if needed
    if (layout.widthMode == SizingMode::Hug) {
        if (isRow) {
            double maxLineMain = 0.0;
            for (const auto& l : lines) maxLineMain = std::max(maxLineMain, l.mainSize);
            container->transform().width = std::max(layout.minWidth, padStart + padEnd + maxLineMain);
        } else {
            double totalCross = 0.0;
            for (size_t i = 0; i < lines.size(); ++i) {
                totalCross += lines[i].crossSize;
                if (i > 0) totalCross += crossGap;
            }
            container->transform().width = std::max(layout.minWidth, padCrossStart + padCrossEnd + totalCross);
        }
    }

    if (layout.heightMode == SizingMode::Hug) {
        if (isRow) {
            double totalCross = 0.0;
            for (size_t i = 0; i < lines.size(); ++i) {
                totalCross += lines[i].crossSize;
                if (i > 0) totalCross += crossGap;
            }
            container->transform().height = std::max(layout.minHeight, padCrossStart + padCrossEnd + totalCross);
        } else {
            double maxLineMain = 0.0;
            for (const auto& l : lines) maxLineMain = std::max(maxLineMain, l.mainSize);
            container->transform().height = std::max(layout.minHeight, padStart + padEnd + maxLineMain);
        }
    }

    // Recompute available sizes after Hug
    containerMainSize = isRow ? container->transform().width : container->transform().height;
    availMain = std::max(0.0, containerMainSize - padStart - padEnd);

    // 4. Position items along main and cross axes
    double currentCrossPos = padCrossStart;

    for (auto& line : lines) {
        int N = static_cast<int>(line.items.size());
        double excessMain = std::max(0.0, availMain - line.mainSize);

        double mainOffset = 0.0;
        double itemStep = mainGap;

        switch (layout.justifyContent) {
            case JustifyContent::FlexStart:
                mainOffset = 0.0;
                itemStep = mainGap;
                break;
            case JustifyContent::FlexEnd:
                mainOffset = excessMain;
                itemStep = mainGap;
                break;
            case JustifyContent::Center:
                mainOffset = excessMain / 2.0;
                itemStep = mainGap;
                break;
            case JustifyContent::SpaceBetween:
                mainOffset = 0.0;
                itemStep = (N > 1) ? mainGap + excessMain / (N - 1) : mainGap;
                break;
            case JustifyContent::SpaceAround: {
                double unit = (N > 0) ? excessMain / N : 0.0;
                mainOffset = unit / 2.0;
                itemStep = mainGap + unit;
                break;
            }
            case JustifyContent::SpaceEvenly: {
                double unit = (N > 0) ? excessMain / (N + 1) : 0.0;
                mainOffset = unit;
                itemStep = mainGap + unit;
                break;
            }
        }

        double curMain = padStart + mainOffset;

        // If reverse, iterate items in reverse order
        std::vector<Node*> orderedItems = line.items;
        if (isReverse) {
            std::reverse(orderedItems.begin(), orderedItems.end());
        }

        for (Node* child : orderedItems) {
            const auto& cl = child->layout();
            AlignSelf align = (cl.alignSelf != AlignSelf::Auto) ? cl.alignSelf : static_cast<AlignSelf>(layout.alignItems);

            double childCross = isRow ? child->transform().height : child->transform().width;
            double crossOffset = 0.0;

            switch (align) {
                case AlignSelf::FlexEnd:
                    crossOffset = line.crossSize - childCross - (isRow ? cl.marginBottom : cl.marginRight);
                    break;
                case AlignSelf::Center:
                    crossOffset = (line.crossSize - childCross) / 2.0;
                    break;
                case AlignSelf::FlexStart:
                case AlignSelf::Stretch:
                case AlignSelf::Auto:
                default:
                    crossOffset = isRow ? cl.marginTop : cl.marginLeft;
                    break;
            }

            if (isRow) {
                child->transform().x = curMain + cl.marginLeft;
                child->transform().y = currentCrossPos + crossOffset;
                curMain += child->transform().width + cl.marginLeft + cl.marginRight + itemStep;
            } else {
                child->transform().x = currentCrossPos + crossOffset;
                child->transform().y = curMain + cl.marginTop;
                curMain += child->transform().height + cl.marginTop + cl.marginBottom + itemStep;
            }
        }

        currentCrossPos += line.crossSize + crossGap;
    }
}

void LayoutEngine::computeGridLayout(Node* container) {
    const auto& layout = container->layout();
    const auto& children = container->children();
    if (children.empty()) return;

    int cols = std::max(1, layout.gridColumns);
    int rows = std::max(1, layout.gridRows);

    double colGap = (layout.columnGap > 0.0) ? layout.columnGap : layout.gap;
    double rowGap = (layout.rowGap > 0.0) ? layout.rowGap : layout.gap;

    double padL = layout.paddingLeft;
    double padR = layout.paddingRight;
    double padT = layout.paddingTop;
    double padB = layout.paddingBottom;

    double availW = std::max(0.0, container->transform().width - padL - padR - (cols - 1) * colGap);
    double cellW = availW / cols;

    double cellH = 100.0; // default row height
    if (layout.heightMode != SizingMode::Hug) {
        double availH = std::max(0.0, container->transform().height - padT - padB - (rows - 1) * rowGap);
        cellH = availH / rows;
    }

    // Grid placement map: 2D vector for occupied cells
    int maxGridRows = std::max(rows, static_cast<int>(children.size()) + 2);
    std::vector<std::vector<bool>> occupied(maxGridRows, std::vector<bool>(cols, false));

    int currentAutoRow = 0;
    int currentAutoCol = 0;
    int highestRowUsed = 0;

    for (const auto& childPtr : children) {
        Node* child = childPtr.get();
        const auto& cl = child->layout();

        int spanX = std::clamp(cl.gridColumnSpan, 1, cols);
        int spanY = std::max(1, cl.gridRowSpan);

        int targetCol = -1;
        int targetRow = -1;

        if (cl.gridColumnStart > 0 && cl.gridRowStart > 0) {
            targetCol = std::min(cols - 1, cl.gridColumnStart - 1);
            targetRow = cl.gridRowStart - 1;
        } else {
            // Find next free cell fitting (spanX, spanY)
            while (true) {
                if (currentAutoCol + spanX > cols) {
                    currentAutoCol = 0;
                    currentAutoRow++;
                }

                if (currentAutoRow + spanY >= static_cast<int>(occupied.size())) {
                    occupied.resize((currentAutoRow + spanY + 4) * 2, std::vector<bool>(cols, false));
                }

                bool fits = true;
                for (int r = 0; r < spanY && fits; ++r) {
                    for (int c = 0; c < spanX && fits; ++c) {
                        if (occupied[currentAutoRow + r][currentAutoCol + c]) {
                            fits = false;
                        }
                    }
                }

                if (fits) {
                    targetCol = currentAutoCol;
                    targetRow = currentAutoRow;
                    currentAutoCol += spanX;
                    break;
                } else {
                    currentAutoCol++;
                }
            }
        }

        // Ensure occupied grid is large enough
        if (targetRow + spanY >= static_cast<int>(occupied.size())) {
            occupied.resize((targetRow + spanY + 4) * 2, std::vector<bool>(cols, false));
        }

        // Mark cells occupied
        for (int r = 0; r < spanY; ++r) {
            for (int c = 0; c < spanX; ++c) {
                if (targetCol + c < cols) {
                    occupied[targetRow + r][targetCol + c] = true;
                }
            }
        }

        highestRowUsed = std::max(highestRowUsed, targetRow + spanY);

        // Position and size the child
        double itemX = padL + targetCol * (cellW + colGap) + cl.marginLeft;
        double itemY = padT + targetRow * (cellH + rowGap) + cl.marginTop;
        double itemW = spanX * cellW + (spanX - 1) * colGap - cl.marginLeft - cl.marginRight;
        double itemH = spanY * cellH + (spanY - 1) * rowGap - cl.marginTop - cl.marginBottom;

        child->transform().x = itemX;
        child->transform().y = itemY;

        if (cl.widthMode == SizingMode::Fill) {
            child->transform().width = std::clamp(itemW, cl.minWidth, cl.maxWidth);
        }
        if (cl.heightMode == SizingMode::Fill) {
            child->transform().height = std::clamp(itemH, cl.minHeight, cl.maxHeight);
        }
    }

    if (layout.heightMode == SizingMode::Hug) {
        int actualRows = std::max(1, highestRowUsed);
        container->transform().height = std::max(layout.minHeight,
            padT + padB + actualRows * cellH + (actualRows - 1) * rowGap);
    }
}

void LayoutEngine::applyConstraints(Node* child, double oldParentW, double oldParentH,
                                    double newParentW, double newParentH) {
    if (!child || oldParentW <= 0.0 || oldParentH <= 0.0) return;

    auto& t = child->transform();
    const auto& l = child->layout();

    // Horizontal Constraints
    switch (l.horizontalConstraint) {
        case HorizontalConstraint::Left:
            // Stays anchored to left, width unchanged
            break;
        case HorizontalConstraint::Right: {
            double rightDist = oldParentW - (t.x + t.width);
            t.x = newParentW - rightDist - t.width;
            break;
        }
        case HorizontalConstraint::LeftRight: {
            double rightDist = oldParentW - (t.x + t.width);
            double newW = newParentW - t.x - rightDist;
            t.width = std::max(0.0, newW);
            break;
        }
        case HorizontalConstraint::Center: {
            double oldCenterOffset = (t.x + t.width / 2.0) - (oldParentW / 2.0);
            t.x = (newParentW / 2.0) + oldCenterOffset - (t.width / 2.0);
            break;
        }
        case HorizontalConstraint::Scale: {
            double scaleFactor = newParentW / oldParentW;
            t.x *= scaleFactor;
            t.width *= scaleFactor;
            break;
        }
    }

    // Vertical Constraints
    switch (l.verticalConstraint) {
        case VerticalConstraint::Top:
            // Stays anchored to top, height unchanged
            break;
        case VerticalConstraint::Bottom: {
            double bottomDist = oldParentH - (t.y + t.height);
            t.y = newParentH - bottomDist - t.height;
            break;
        }
        case VerticalConstraint::TopBottom: {
            double bottomDist = oldParentH - (t.y + t.height);
            double newH = newParentH - t.y - bottomDist;
            t.height = std::max(0.0, newH);
            break;
        }
        case VerticalConstraint::Center: {
            double oldCenterOffset = (t.y + t.height / 2.0) - (oldParentH / 2.0);
            t.y = (newParentH / 2.0) + oldCenterOffset - (t.height / 2.0);
            break;
        }
        case VerticalConstraint::Scale: {
            double scaleFactor = newParentH / oldParentH;
            t.y *= scaleFactor;
            t.height *= scaleFactor;
            break;
        }
    }
}

void LayoutEngine::applyBreakpoint(Document& doc, Breakpoint bp) {
    if (!doc.root()) return;

    double targetW = 1440.0;
    double targetH = 900.0;
    getBreakpointDimensions(bp, targetW, targetH);

    double oldW = doc.root()->transform().width;
    double oldH = doc.root()->transform().height;

    doc.root()->transform().width = targetW;
    doc.root()->transform().height = targetH;

    // If root is in Absolute layout, adjust direct children via constraints
    if (doc.root()->layout().mode == LayoutMode::None) {
        for (const auto& child : doc.root()->children()) {
            applyConstraints(child.get(), oldW, oldH, targetW, targetH);
        }
    }

    computeDocumentLayout(doc);
}

} // namespace poliable
