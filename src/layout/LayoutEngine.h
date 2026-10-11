#pragma once
#include "model/Node.h"
#include "model/Document.h"

namespace poliable {

class LayoutEngine {
public:
    // Recursively computes layout for a node and all of its descendants
    static void computeLayout(Node* node);

    // Computes layout for the entire document tree starting at root
    static void computeDocumentLayout(Document& doc);

    // Solves constraints when parent container resizes from (oldW, oldH) to (newW, newH)
    static void applyConstraints(Node* child, double oldParentW, double oldParentH,
                                 double newParentW, double newParentH);

    // Switches responsive viewport breakpoint and recalculates layout
    static void applyBreakpoint(Document& doc, Breakpoint bp);

    // Helper: returns default dimensions for a breakpoint
    static void getBreakpointDimensions(Breakpoint bp, double& outW, double& outH);

private:
    static void computeFlexLayout(Node* container);
    static void computeGridLayout(Node* container);
};

} // namespace poliable
