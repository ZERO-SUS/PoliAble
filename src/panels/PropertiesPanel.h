#pragma once
#include <QWidget>
#include "model/Node.h"

namespace poliable { class Document; class UndoStack; }

class QDoubleSpinBox;
class QSpinBox;
class QLineEdit;
class QPushButton;
class QLabel;
class QComboBox;
class QCheckBox;
class QGroupBox;

class PropertiesPanel : public QWidget {
    Q_OBJECT
public:
    explicit PropertiesPanel(QWidget* parent = nullptr);

    void setDocument(poliable::Document* doc);
    void setUndoStack(poliable::UndoStack* stack);

    // Called when canvas selection changes
    void updateFromCanvas(class CanvasItem* item = nullptr);

    // Show properties for a specific node
    void showNode(poliable::Node* node);

signals:
    void layoutModified();
    void stylingModified();

public slots:
    void onXChanged(double v);
    void onYChanged(double v);
    void onWChanged(double v);
    void onHChanged(double v);
    void onOpacityChanged(double v);
    void onNameChanged(const QString& text);
    void onPickColor();
    void onPickFillColor();
    void onPickStrokeColor();
    void onPickBorderColor();

    // Stage 4 Layout Slots
    void onLayoutModeChanged(int idx);
    void onSizingChanged();
    void onConstraintsChanged();
    void onFlexPropsChanged();
    void onGridPropsChanged();

    // Stage 5 Styling Slots
    void onFillGradientChanged(int idx);
    void onStrokeStyleChanged(int idx);
    void onBorderStyleChanged(int idx);
    void onRadiusChanged(double v);

private:
    void setEditing(bool editing);
    void updateLayoutControlsVisibility();
    void updateStylingControlsVisibility();
    void commitLayoutChange(const poliable::Layout& next);
    void commitStyleChange(const poliable::Style& next);
    void commitStylingChange(const poliable::Style& next);

    QLineEdit*       m_nameEdit    = nullptr;
    QDoubleSpinBox*  m_xSpin       = nullptr;
    QDoubleSpinBox*  m_ySpin       = nullptr;
    QDoubleSpinBox*  m_wSpin       = nullptr;
    QDoubleSpinBox*  m_hSpin       = nullptr;
    QDoubleSpinBox*  m_opacitySpin = nullptr;
    QPushButton*     m_colorBtn    = nullptr;
    QLabel*          m_noSelLabel  = nullptr;
    QWidget*         m_form        = nullptr;

    // Layout Engine UI controls (Stage 4)
    QComboBox*       m_layoutModeCombo   = nullptr;
    QComboBox*       m_wModeCombo        = nullptr;
    QComboBox*       m_hModeCombo        = nullptr;
    QComboBox*       m_hConstraintCombo  = nullptr;
    QComboBox*       m_vConstraintCombo  = nullptr;

    // Flex container controls
    QWidget*         m_flexGroup         = nullptr;
    QComboBox*       m_flexDirCombo      = nullptr;
    QComboBox*       m_justifyCombo      = nullptr;
    QComboBox*       m_alignItemsCombo   = nullptr;
    QDoubleSpinBox*  m_gapSpin           = nullptr;
    QDoubleSpinBox*  m_padTSpin          = nullptr;
    QDoubleSpinBox*  m_padRSpin          = nullptr;
    QDoubleSpinBox*  m_padBSpin          = nullptr;
    QDoubleSpinBox*  m_padLSpin          = nullptr;
    QCheckBox*       m_wrapCheck         = nullptr;

    // Grid container controls
    QWidget*         m_gridGroup         = nullptr;
    QSpinBox*        m_gridColsSpin      = nullptr;
    QSpinBox*        m_gridRowsSpin      = nullptr;
    QDoubleSpinBox*  m_colGapSpin        = nullptr;
    QDoubleSpinBox*  m_rowGapSpin        = nullptr;
    QSpinBox*        m_colSpanSpin       = nullptr;
    QSpinBox*        m_rowSpanSpin       = nullptr;

    // Styling controls (Stage 5)
    QWidget*         m_stylingGroup      = nullptr;
    QPushButton*     m_fillColorBtn      = nullptr;
    QComboBox*       m_fillGradientCombo = nullptr;
    QPushButton*     m_strokeColorBtn    = nullptr;
    QDoubleSpinBox*  m_strokeWidthSpin   = nullptr;
    QComboBox*       m_strokeStyleCombo  = nullptr;
    QPushButton*     m_borderColorBtn    = nullptr;
    QDoubleSpinBox*  m_borderWidthSpin   = nullptr;
    QComboBox*       m_borderStyleCombo  = nullptr;
    QDoubleSpinBox*  m_radiusSpin        = nullptr;

    poliable::Document*  m_doc   = nullptr;
    poliable::UndoStack* m_stack = nullptr;
    poliable::Node*      m_node  = nullptr;
    bool                 m_isEditing = false;
};
