#include "PropertiesPanel.h"
#include "canvas/CanvasItem.h"
#include "model/Document.h"
#include "model/Node.h"
#include "model/Command.h"
#include "model/UndoStack.h"
#include "layout/LayoutEngine.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QColorDialog>
#include <QColor>
#include <QString>

// ── Helpers ──────────────────────────────────────────────────────────────────

static QDoubleSpinBox* makeSpin(double min, double max, double step = 1.0) {
    auto* s = new QDoubleSpinBox();
    s->setRange(min, max);
    s->setSingleStep(step);
    s->setDecimals(1);
    s->setStyleSheet(
        "QDoubleSpinBox { background: #1e2027; color: #f0f2f8; "
        "border: 1px solid #2e313d; padding: 4px 7px; border-radius: 4px; font-size: 11px; } "
        "QDoubleSpinBox:focus { border: 1px solid #6366f1; background: #22252e; }"
    );
    return s;
}

static QSpinBox* makeIntSpin(int min, int max, int step = 1) {
    auto* s = new QSpinBox();
    s->setRange(min, max);
    s->setSingleStep(step);
    s->setStyleSheet(
        "QSpinBox { background: #1e2027; color: #f0f2f8; "
        "border: 1px solid #2e313d; padding: 4px 7px; border-radius: 4px; font-size: 11px; } "
        "QSpinBox:focus { border: 1px solid #6366f1; background: #22252e; }"
    );
    return s;
}

static QComboBox* makeCombo(const QStringList& items) {
    auto* cb = new QComboBox();
    cb->addItems(items);
    cb->setStyleSheet(
        "QComboBox { background: #1e2027; color: #f0f2f8; "
        "border: 1px solid #2e313d; padding: 4px 8px; border-radius: 4px; font-size: 11px; } "
        "QComboBox:hover, QComboBox:focus { border: 1px solid #6366f1; } "
        "QComboBox QAbstractItemView { background: #1d1f26; color: #e1e4ee; border: 1px solid #2e313d; selection-background-color: #6366f1; }"
    );
    return cb;
}

static QLabel* makeSectionHeader(const QString& title) {
    auto* l = new QLabel(title);
    l->setStyleSheet(
        "QLabel { color: #818cf8; font-size: 10px; font-weight: bold; "
        "text-transform: uppercase; letter-spacing: 0.5px; padding-top: 10px; padding-bottom: 3px; }"
    );
    return l;
}

// ── Construction ─────────────────────────────────────────────────────────────

PropertiesPanel::PropertiesPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // "no selection" placeholder
    m_noSelLabel = new QLabel("✦  No Selection\n\nSelect an element on canvas\nto view and edit properties.", this);
    m_noSelLabel->setAlignment(Qt::AlignCenter);
    m_noSelLabel->setStyleSheet("QLabel { color: #6b7280; font-size: 11px; line-height: 1.4; padding: 40px 20px; }");
    root->addWidget(m_noSelLabel);

    // Form widget (hidden until something is selected)
    m_form = new QWidget(this);
    auto* form = new QFormLayout(m_form);
    form->setContentsMargins(10, 10, 10, 10);
    form->setSpacing(6);
    form->setLabelAlignment(Qt::AlignRight);

    // ── General Section ──
    form->addRow(makeSectionHeader("General"));

    m_nameEdit = new QLineEdit(m_form);
    m_nameEdit->setStyleSheet(
        "QLineEdit { background: #1e2027; color: #f0f2f8; "
        "border: 1px solid #2e313d; padding: 4px 8px; border-radius: 4px; font-size: 11px; } "
        "QLineEdit:focus { border: 1px solid #6366f1; }"
    );
    form->addRow("Name:", m_nameEdit);

    m_xSpin = makeSpin(-9999, 9999); form->addRow("X:", m_xSpin);
    m_ySpin = makeSpin(-9999, 9999); form->addRow("Y:", m_ySpin);
    m_wSpin = makeSpin(1, 9999);     form->addRow("W:", m_wSpin);
    m_hSpin = makeSpin(1, 9999);     form->addRow("H:", m_hSpin);
    m_opacitySpin = makeSpin(0.0, 1.0, 0.05); form->addRow("Opacity:", m_opacitySpin);

    m_colorBtn = new QPushButton("Pick colour…", m_form);
    m_colorBtn->setStyleSheet(
        "QPushButton { background: #252834; color: #e0e2ec; "
        "border: 1px solid #333646; padding: 5px 10px; border-radius: 4px; font-size: 11px; } "
        "QPushButton:hover { background: #2e3242; border-color: #6366f1; color: #ffffff; }"
    );
    form->addRow("Fill:", m_colorBtn);

    // ── Layout Mode & Sizing Section (Stage 4) ──
    form->addRow(makeSectionHeader("Layout & Sizing"));

    m_layoutModeCombo = makeCombo({"Absolute (None)", "Flexbox (Auto Layout)", "CSS Grid"});
    form->addRow("Mode:", m_layoutModeCombo);

    m_wModeCombo = makeCombo({"Fixed", "Hug Content", "Fill Container"});
    form->addRow("Width Mode:", m_wModeCombo);

    m_hModeCombo = makeCombo({"Fixed", "Hug Content", "Fill Container"});
    form->addRow("Height Mode:", m_hModeCombo);

    m_hConstraintCombo = makeCombo({"Left", "Right", "Left & Right", "Center", "Scale"});
    form->addRow("H Constraint:", m_hConstraintCombo);

    m_vConstraintCombo = makeCombo({"Top", "Bottom", "Top & Bottom", "Center", "Scale"});
    form->addRow("V Constraint:", m_vConstraintCombo);

    // ── Flexbox Container Controls ──
    m_flexGroup = new QWidget(m_form);
    auto* flexForm = new QFormLayout(m_flexGroup);
    flexForm->setContentsMargins(0, 4, 0, 4);
    flexForm->setSpacing(5);
    flexForm->setLabelAlignment(Qt::AlignRight);

    flexForm->addRow(makeSectionHeader("Flexbox Container"));

    m_flexDirCombo = makeCombo({"Row →", "Column ↓", "Row Reverse ←", "Column Reverse ↑"});
    flexForm->addRow("Direction:", m_flexDirCombo);

    m_justifyCombo = makeCombo({"Start", "Center", "End", "Space Between", "Space Around", "Space Evenly"});
    flexForm->addRow("Justify:", m_justifyCombo);

    m_alignItemsCombo = makeCombo({"Stretch", "Start", "Center", "End"});
    flexForm->addRow("Align Items:", m_alignItemsCombo);

    m_gapSpin = makeSpin(0, 500, 2.0);
    flexForm->addRow("Gap:", m_gapSpin);

    // Padding (T, R, B, L)
    auto* padRow = new QWidget(m_flexGroup);
    auto* padHBox = new QHBoxLayout(padRow);
    padHBox->setContentsMargins(0, 0, 0, 0);
    padHBox->setSpacing(3);
    m_padTSpin = makeSpin(0, 500, 2.0);
    m_padRSpin = makeSpin(0, 500, 2.0);
    m_padBSpin = makeSpin(0, 500, 2.0);
    m_padLSpin = makeSpin(0, 500, 2.0);
    padHBox->addWidget(new QLabel("T:")); padHBox->addWidget(m_padTSpin);
    padHBox->addWidget(new QLabel("R:")); padHBox->addWidget(m_padRSpin);
    padHBox->addWidget(new QLabel("B:")); padHBox->addWidget(m_padBSpin);
    padHBox->addWidget(new QLabel("L:")); padHBox->addWidget(m_padLSpin);
    flexForm->addRow("Padding:", padRow);

    m_wrapCheck = new QCheckBox("Wrap Items", m_flexGroup);
    m_wrapCheck->setStyleSheet("QCheckBox { color: #cccccc; }");
    flexForm->addRow("Wrapping:", m_wrapCheck);

    form->addRow(m_flexGroup);

    // ── Grid Container Controls ──
    m_gridGroup = new QWidget(m_form);
    auto* gridForm = new QFormLayout(m_gridGroup);
    gridForm->setContentsMargins(0, 4, 0, 4);
    gridForm->setSpacing(5);
    gridForm->setLabelAlignment(Qt::AlignRight);

    gridForm->addRow(makeSectionHeader("CSS Grid Container"));

    m_gridColsSpin = makeIntSpin(1, 24, 1);
    gridForm->addRow("Columns:", m_gridColsSpin);

    m_gridRowsSpin = makeIntSpin(1, 24, 1);
    gridForm->addRow("Rows:", m_gridRowsSpin);

    m_colGapSpin = makeSpin(0, 200, 2.0);
    gridForm->addRow("Col Gap:", m_colGapSpin);

    m_rowGapSpin = makeSpin(0, 200, 2.0);
    gridForm->addRow("Row Gap:", m_rowGapSpin);

    m_colSpanSpin = makeIntSpin(1, 24, 1);
    gridForm->addRow("Col Span:", m_colSpanSpin);

    m_rowSpanSpin = makeIntSpin(1, 24, 1);
    gridForm->addRow("Row Span:", m_rowSpanSpin);

    form->addRow(m_gridGroup);

    // ── Styling Section (Stage 5) ──
    form->addRow(makeSectionHeader("Styling"));

    // Fill Color with Gradient option
    auto* fillRow = new QWidget(m_form);
    auto* fillHBox = new QHBoxLayout(fillRow);
    fillHBox->setContentsMargins(0, 0, 0, 0);
    fillHBox->setSpacing(3);
    m_fillColorBtn = new QPushButton(fillRow);
    m_fillColorBtn->setFixedWidth(80);
    m_fillColorBtn->setStyleSheet(
        "QPushButton { background: #3c3c3c; border: 1px solid #555; padding: 3px; border-radius: 2px; }"
    );
    connect(m_fillColorBtn, &QPushButton::clicked, this, &PropertiesPanel::onPickFillColor);
    fillHBox->addWidget(m_fillColorBtn);
    fillHBox->addWidget(new QLabel("Fill Color"));

    m_fillGradientCombo = makeCombo({"Solid", "Linear", "Radial", "Conic"});
    fillHBox->addWidget(new QLabel("Type:"));
    fillHBox->addWidget(m_fillGradientCombo);

    form->addRow("Fill:", fillRow);

    // Stroke
    auto* strokeRow = new QWidget(m_form);
    auto* strokeHBox = new QHBoxLayout(strokeRow);
    strokeHBox->setContentsMargins(0, 0, 0, 0);
    strokeHBox->setSpacing(3);
    m_strokeColorBtn = new QPushButton(strokeRow);
    m_strokeColorBtn->setFixedWidth(80);
    m_strokeColorBtn->setStyleSheet(
        "QPushButton { background: #3c3c3c; border: 1px solid #555; padding: 3px; border-radius: 2px; }"
    );
    connect(m_strokeColorBtn, &QPushButton::clicked, this, &PropertiesPanel::onPickStrokeColor);
    strokeHBox->addWidget(m_strokeColorBtn);
    m_strokeWidthSpin = makeSpin(0, 100, 1.0);
    strokeHBox->addWidget(new QLabel("Width:"));
    strokeHBox->addWidget(m_strokeWidthSpin);
    m_strokeStyleCombo = makeCombo({"Solid", "Dashed", "Dotted"});
    strokeHBox->addWidget(new QLabel("Style:"));
    strokeHBox->addWidget(m_strokeStyleCombo);
    form->addRow("Stroke:", strokeRow);

    // Border
    auto* borderRow = new QWidget(m_form);
    auto* borderHBox = new QHBoxLayout(borderRow);
    borderHBox->setContentsMargins(0, 0, 0, 0);
    borderHBox->setSpacing(3);
    m_borderColorBtn = new QPushButton(borderRow);
    m_borderColorBtn->setFixedWidth(80);
    m_borderColorBtn->setStyleSheet(
        "QPushButton { background: #3c3c3c; border: 1px solid #555; padding: 3px; border-radius: 2px; }"
    );
    connect(m_borderColorBtn, &QPushButton::clicked, this, &PropertiesPanel::onPickBorderColor);
    borderHBox->addWidget(m_borderColorBtn);
    m_borderWidthSpin = makeSpin(0, 50, 1.0);
    borderHBox->addWidget(new QLabel("Width:"));
    borderHBox->addWidget(m_borderWidthSpin);
    m_borderStyleCombo = makeCombo({"Solid", "Dashed", "Dotted", "None"});
    borderHBox->addWidget(new QLabel("Style:"));
    borderHBox->addWidget(m_borderStyleCombo);
    form->addRow("Border:", borderRow);

    // Corner Radius
    auto* radiusRow = new QWidget(m_form);
    auto* radiusHBox = new QHBoxLayout(radiusRow);
    radiusHBox->setContentsMargins(0, 0, 0, 0);
    radiusHBox->setSpacing(3);
    m_radiusSpin = makeSpin(0, 100, 1.0);
    radiusHBox->addWidget(m_radiusSpin);
    radiusHBox->addWidget(new QLabel("Radius"));
    form->addRow("Corner:", radiusRow);

    root->addWidget(m_form);
    root->addStretch();

    m_form->setVisible(false);

    // ── Signals ──
    connect(m_nameEdit,    &QLineEdit::textEdited,        this, &PropertiesPanel::onNameChanged);
    connect(m_xSpin,       QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::onXChanged);
    connect(m_ySpin,       QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::onYChanged);
    connect(m_wSpin,       QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::onWChanged);
    connect(m_hSpin,       QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::onHChanged);
    connect(m_opacitySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::onOpacityChanged);
    connect(m_colorBtn,    &QPushButton::clicked,         this, &PropertiesPanel::onPickColor);

    // Layout signals
    connect(m_layoutModeCombo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PropertiesPanel::onLayoutModeChanged);
    connect(m_wModeCombo,       QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PropertiesPanel::onSizingChanged);
    connect(m_hModeCombo,       QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PropertiesPanel::onSizingChanged);
    connect(m_hConstraintCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PropertiesPanel::onConstraintsChanged);
    connect(m_vConstraintCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PropertiesPanel::onConstraintsChanged);

    connect(m_flexDirCombo,    QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PropertiesPanel::onFlexPropsChanged);
    connect(m_justifyCombo,    QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PropertiesPanel::onFlexPropsChanged);
    connect(m_alignItemsCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PropertiesPanel::onFlexPropsChanged);
    connect(m_gapSpin,         QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::onFlexPropsChanged);
    connect(m_padTSpin,        QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::onFlexPropsChanged);
    connect(m_padRSpin,        QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::onFlexPropsChanged);
    connect(m_padBSpin,        QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::onFlexPropsChanged);
    connect(m_padLSpin,        QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::onFlexPropsChanged);
    connect(m_wrapCheck,       &QCheckBox::toggled,                                  this, &PropertiesPanel::onFlexPropsChanged);

    connect(m_gridColsSpin, QOverload<int>::of(&QSpinBox::valueChanged),          this, &PropertiesPanel::onGridPropsChanged);
    connect(m_gridRowsSpin, QOverload<int>::of(&QSpinBox::valueChanged),          this, &PropertiesPanel::onGridPropsChanged);
    connect(m_colGapSpin,   QOverload<double>::of(&QDoubleSpinBox::valueChanged),  this, &PropertiesPanel::onGridPropsChanged);
    connect(m_rowGapSpin,   QOverload<double>::of(&QDoubleSpinBox::valueChanged),  this, &PropertiesPanel::onGridPropsChanged);
    connect(m_colSpanSpin,  QOverload<int>::of(&QSpinBox::valueChanged),          this, &PropertiesPanel::onGridPropsChanged);
    connect(m_rowSpanSpin,  QOverload<int>::of(&QSpinBox::valueChanged),          this, &PropertiesPanel::onGridPropsChanged);

    // Styling signals
    connect(m_fillGradientCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PropertiesPanel::onFillGradientChanged);
    connect(m_strokeWidthSpin,   QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::onStrokeStyleChanged);
    connect(m_strokeStyleCombo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PropertiesPanel::onStrokeStyleChanged);
    connect(m_borderWidthSpin,   QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::onBorderStyleChanged);
    connect(m_borderStyleCombo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PropertiesPanel::onBorderStyleChanged);
    connect(m_radiusSpin,        QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::onRadiusChanged);
}

// ── Public API ────────────────────────────────────────────────────────────────

void PropertiesPanel::setDocument(poliable::Document* doc) {
    m_doc = doc;
    if (m_stack) m_stack->setDocument(doc);
    m_node = nullptr;
    m_form->setVisible(false);
    m_noSelLabel->setVisible(true);
}

void PropertiesPanel::setUndoStack(poliable::UndoStack* stack) {
    m_stack = stack;
}

void PropertiesPanel::updateFromCanvas(CanvasItem* item) {
    showNode(item ? item->node() : nullptr);
}

void PropertiesPanel::updateLayoutControlsVisibility() {
    if (!m_node) return;
    bool isFlex = (m_node->layout().mode == poliable::LayoutMode::Flex) || (m_node->type() == poliable::NodeType::Flex);
    bool isGrid = (m_node->layout().mode == poliable::LayoutMode::Grid) || (m_node->type() == poliable::NodeType::Grid);

    m_flexGroup->setVisible(isFlex);
    m_gridGroup->setVisible(isGrid);
    m_stylingGroup->setVisible(true);
}

void PropertiesPanel::updateStylingControlsVisibility() {
    updateLayoutControlsVisibility();
}

void PropertiesPanel::showNode(poliable::Node* node) {
    m_node = node;
    if (!node) {
        m_form->setVisible(false);
        m_noSelLabel->setVisible(true);
        return;
    }

    m_noSelLabel->setVisible(false);
    m_form->setVisible(true);

    setEditing(true);

    const auto& t = node->transform();
    const auto& s = node->style();
    const auto& l = node->layout();

    m_nameEdit->setText(QString::fromStdString(node->name()));
    m_xSpin->setValue(t.x);
    m_ySpin->setValue(t.y);
    m_wSpin->setValue(t.width);
    m_hSpin->setValue(t.height);
    m_opacitySpin->setValue(t.opacity);

    // Layout mode
    int lmode = 0;
    if (l.mode == poliable::LayoutMode::Flex || node->type() == poliable::NodeType::Flex) lmode = 1;
    else if (l.mode == poliable::LayoutMode::Grid || node->type() == poliable::NodeType::Grid) lmode = 2;
    m_layoutModeCombo->setCurrentIndex(lmode);

    // Sizing modes
    m_wModeCombo->setCurrentIndex(static_cast<int>(l.widthMode));
    m_hModeCombo->setCurrentIndex(static_cast<int>(l.heightMode));

    // Constraints
    m_hConstraintCombo->setCurrentIndex(static_cast<int>(l.horizontalConstraint));
    m_vConstraintCombo->setCurrentIndex(static_cast<int>(l.verticalConstraint));

    // Flex props
    m_flexDirCombo->setCurrentIndex(static_cast<int>(l.flexDirection));
    m_justifyCombo->setCurrentIndex(static_cast<int>(l.justifyContent));
    int aiIdx = (l.alignItems == poliable::AlignItems::Stretch) ? 0 :
                (l.alignItems == poliable::AlignItems::FlexStart) ? 1 :
                (l.alignItems == poliable::AlignItems::Center) ? 2 : 3;
    m_alignItemsCombo->setCurrentIndex(aiIdx);
    m_gapSpin->setValue(l.gap);
    m_padTSpin->setValue(l.paddingTop);
    m_padRSpin->setValue(l.paddingRight);
    m_padBSpin->setValue(l.paddingBottom);
    m_padLSpin->setValue(l.paddingLeft);
    m_wrapCheck->setChecked(l.flexWrap == poliable::FlexWrap::Wrap);

    // Grid props
    m_gridColsSpin->setValue(l.gridColumns);
    m_gridRowsSpin->setValue(l.gridRows);
    m_colGapSpin->setValue(l.columnGap);
    m_rowGapSpin->setValue(l.rowGap);
    m_colSpanSpin->setValue(l.gridColumnSpan);
    m_rowSpanSpin->setValue(l.gridRowSpan);

    updateLayoutControlsVisibility();
    updateStylingControlsVisibility();

    // Tint the colour buttons to show colors
    QColor fill(QString::fromStdString(s.fillColor));
    if (fill.isValid()) {
        QString style = QString(
            "QPushButton { background: %1; color: %2; "
            "border: 1px solid #555; padding: 3px; border-radius: 2px; }"
        ).arg(fill.name())
         .arg(fill.lightness() < 128 ? "#ffffff" : "#000000");
        m_fillColorBtn->setStyleSheet(style);
        m_fillColorBtn->setText(fill.name().toUpper());
    }

    QColor stroke(QString::fromStdString(s.strokeColor));
    if (stroke.isValid()) {
        QString style = QString(
            "QPushButton { background: %1; color: %2; "
            "border: 1px solid #555; padding: 3px; border-radius: 2px; }"
        ).arg(stroke.name())
         .arg(stroke.lightness() < 128 ? "#ffffff" : "#000000");
        m_strokeColorBtn->setStyleSheet(style);
        m_strokeColorBtn->setText(stroke.name().toUpper());
    }

    QColor border(QString::fromStdString(s.borderTop.color));
    if (border.isValid()) {
        QString style = QString(
            "QPushButton { background: %1; color: %2; "
            "border: 1px solid #555; padding: 3px; border-radius: 2px; }"
        ).arg(border.name())
         .arg(border.lightness() < 128 ? "#ffffff" : "#000000");
        m_borderColorBtn->setStyleSheet(style);
        m_borderColorBtn->setText(border.name().toUpper());
    }

    // Styling controls
    int gradientIdx = s.isFillGradient ? static_cast<int>(s.fillGradient.type) : 0;
    m_fillGradientCombo->setCurrentIndex(gradientIdx);
    m_strokeWidthSpin->setValue(s.strokeWidth);
    m_strokeStyleCombo->setCurrentIndex(s.strokeStyle == "dashed" ? 1 : (s.strokeStyle == "dotted" ? 2 : 0));
    m_borderWidthSpin->setValue(s.borderTop.width);
    m_borderStyleCombo->setCurrentIndex(s.borderTop.style == "dashed" ? 1 : (s.borderTop.style == "dotted" ? 2 : (s.borderTop.style == "none" ? 3 : 0)));
    m_radiusSpin->setValue(s.cornerRadius);

    setEditing(false);
}

// ── Private helpers ───────────────────────────────────────────────────────────

void PropertiesPanel::setEditing(bool editing) {
    m_isEditing = editing;
    m_xSpin->blockSignals(editing);
    m_ySpin->blockSignals(editing);
    m_wSpin->blockSignals(editing);
    m_hSpin->blockSignals(editing);
    m_opacitySpin->blockSignals(editing);
    m_nameEdit->blockSignals(editing);

    m_layoutModeCombo->blockSignals(editing);
    m_wModeCombo->blockSignals(editing);
    m_hModeCombo->blockSignals(editing);
    m_hConstraintCombo->blockSignals(editing);
    m_vConstraintCombo->blockSignals(editing);

    m_flexDirCombo->blockSignals(editing);
    m_justifyCombo->blockSignals(editing);
    m_alignItemsCombo->blockSignals(editing);
    m_gapSpin->blockSignals(editing);
    m_padTSpin->blockSignals(editing);
    m_padRSpin->blockSignals(editing);
    m_padBSpin->blockSignals(editing);
    m_padLSpin->blockSignals(editing);
    m_wrapCheck->blockSignals(editing);

    m_gridColsSpin->blockSignals(editing);
    m_gridRowsSpin->blockSignals(editing);
    m_colGapSpin->blockSignals(editing);
    m_rowGapSpin->blockSignals(editing);
    m_colSpanSpin->blockSignals(editing);
    m_rowSpanSpin->blockSignals(editing);

    // Styling controls
    m_fillGradientCombo->blockSignals(editing);
    m_strokeWidthSpin->blockSignals(editing);
    m_strokeStyleCombo->blockSignals(editing);
    m_borderWidthSpin->blockSignals(editing);
    m_borderStyleCombo->blockSignals(editing);
    m_radiusSpin->blockSignals(editing);
}

void PropertiesPanel::commitLayoutChange(const poliable::Layout& next) {
    if (!m_node || !m_stack) return;

    poliable::Layout prev = m_node->layout();
    auto cmd = std::make_unique<poliable::SetLayoutCommand>(m_node->id(), prev, next);
    m_stack->execute(std::move(cmd));

    if (m_doc) {
        poliable::LayoutEngine::computeDocumentLayout(*m_doc);
    }
    emit layoutModified();
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void PropertiesPanel::onXChanged(double v) {
    if (!m_node || m_isEditing) return;
    m_node->transform().x = v;
}

void PropertiesPanel::onYChanged(double v) {
    if (!m_node || m_isEditing) return;
    m_node->transform().y = v;
}

void PropertiesPanel::onWChanged(double v) {
    if (!m_node || m_isEditing) return;
    m_node->transform().width = v;
    if (m_doc) {
        poliable::LayoutEngine::computeDocumentLayout(*m_doc);
        emit layoutModified();
    }
}

void PropertiesPanel::onHChanged(double v) {
    if (!m_node || m_isEditing) return;
    m_node->transform().height = v;
    if (m_doc) {
        poliable::LayoutEngine::computeDocumentLayout(*m_doc);
        emit layoutModified();
    }
}

void PropertiesPanel::onOpacityChanged(double v) {
    if (!m_node || m_isEditing) return;
    m_node->transform().opacity = v;
}

void PropertiesPanel::onNameChanged(const QString& text) {
    if (!m_node || m_isEditing) return;
    m_node->setName(text.toStdString());
}

void PropertiesPanel::onPickColor() {
    if (!m_node || !m_stack) return;
    QColor current(QString::fromStdString(m_node->style().fillColor));
    QColor chosen = QColorDialog::getColor(current, this, "Pick Fill Colour");
    if (!chosen.isValid()) return;

    poliable::Style prev = m_node->style();
    m_node->style().fillColor = chosen.name().toStdString();

    QString style = QString(
        "QPushButton { background: %1; color: %2; "
        "border: 1px solid #555; padding: 3px 8px; border-radius: 2px; }"
    ).arg(chosen.name())
     .arg(chosen.lightness() < 128 ? "#ffffff" : "#000000");
    m_colorBtn->setStyleSheet(style);
    m_colorBtn->setText(chosen.name().toUpper());

    poliable::Style next = m_node->style();
    auto cmd = std::make_unique<poliable::SetStyleCommand>(
        m_node->id(), prev, next);
    m_stack->execute(std::move(cmd));
}

void PropertiesPanel::onLayoutModeChanged(int idx) {
    if (!m_node || m_isEditing) return;
    poliable::Layout next = m_node->layout();
    switch (idx) {
        case 1:  next.mode = poliable::LayoutMode::Flex; break;
        case 2:  next.mode = poliable::LayoutMode::Grid; break;
        default: next.mode = poliable::LayoutMode::None; break;
    }
    updateLayoutControlsVisibility();
    commitLayoutChange(next);
}

void PropertiesPanel::onSizingChanged() {
    if (!m_node || m_isEditing) return;
    poliable::Layout next = m_node->layout();
    next.widthMode  = static_cast<poliable::SizingMode>(m_wModeCombo->currentIndex());
    next.heightMode = static_cast<poliable::SizingMode>(m_hModeCombo->currentIndex());
    commitLayoutChange(next);
}

void PropertiesPanel::onConstraintsChanged() {
    if (!m_node || m_isEditing) return;
    poliable::Layout next = m_node->layout();
    next.horizontalConstraint = static_cast<poliable::HorizontalConstraint>(m_hConstraintCombo->currentIndex());
    next.verticalConstraint   = static_cast<poliable::VerticalConstraint>(m_vConstraintCombo->currentIndex());
    commitLayoutChange(next);
}

void PropertiesPanel::onFlexPropsChanged() {
    if (!m_node || m_isEditing) return;
    poliable::Layout next = m_node->layout();
    next.flexDirection = static_cast<poliable::FlexDirection>(m_flexDirCombo->currentIndex());
    next.justifyContent = static_cast<poliable::JustifyContent>(m_justifyCombo->currentIndex());

    int aiIdx = m_alignItemsCombo->currentIndex();
    next.alignItems = (aiIdx == 0) ? poliable::AlignItems::Stretch :
                      (aiIdx == 1) ? poliable::AlignItems::FlexStart :
                      (aiIdx == 2) ? poliable::AlignItems::Center : poliable::AlignItems::FlexEnd;

    next.gap = m_gapSpin->value();
    next.paddingTop = m_padTSpin->value();
    next.paddingRight = m_padRSpin->value();
    next.paddingBottom = m_padBSpin->value();
    next.paddingLeft = m_padLSpin->value();
    next.flexWrap = m_wrapCheck->isChecked() ? poliable::FlexWrap::Wrap : poliable::FlexWrap::NoWrap;

    commitLayoutChange(next);
}

void PropertiesPanel::onGridPropsChanged() {
    if (!m_node || m_isEditing) return;
    poliable::Layout next = m_node->layout();
    next.gridColumns = m_gridColsSpin->value();
    next.gridRows = m_gridRowsSpin->value();
    next.columnGap = m_colGapSpin->value();
    next.rowGap = m_rowGapSpin->value();
    next.gridColumnSpan = m_colSpanSpin->value();
    next.gridRowSpan = m_rowSpanSpin->value();

    commitLayoutChange(next);
}

// ── Stage 5 Styling Slots ───────────────────────────────────────────────────

void PropertiesPanel::onPickFillColor() {
    if (!m_node || !m_stack) return;
    QColor current(QString::fromStdString(m_node->style().fillColor));
    QColor chosen = QColorDialog::getColor(current, this, "Pick Fill Colour");
    if (!chosen.isValid()) return;

    poliable::Style prev = m_node->style();
    m_node->style().fillColor = chosen.name().toStdString();

    QString style = QString(
        "QPushButton { background: %1; color: %2; "
        "border: 1px solid #555; padding: 3px; border-radius: 2px; }"
    ).arg(chosen.name())
     .arg(chosen.lightness() < 128 ? "#ffffff" : "#000000");
    m_fillColorBtn->setStyleSheet(style);
    m_fillColorBtn->setText(chosen.name().toUpper());

    poliable::Style next = m_node->style();
    auto cmd = std::make_unique<poliable::SetStyleCommand>(
        m_node->id(), prev, next);
    m_stack->execute(std::move(cmd));
}

void PropertiesPanel::onPickStrokeColor() {
    if (!m_node || !m_stack) return;
    QColor current(QString::fromStdString(m_node->style().strokeColor));
    QColor chosen = QColorDialog::getColor(current, this, "Pick Stroke Colour");
    if (!chosen.isValid()) return;

    poliable::Style prev = m_node->style();
    m_node->style().strokeColor = chosen.name().toStdString();

    QString style = QString(
        "QPushButton { background: %1; color: %2; "
        "border: 1px solid #555; padding: 3px; border-radius: 2px; }"
    ).arg(chosen.name())
     .arg(chosen.lightness() < 128 ? "#ffffff" : "#000000");
    m_strokeColorBtn->setStyleSheet(style);
    m_strokeColorBtn->setText(chosen.name().toUpper());

    poliable::Style next = m_node->style();
    commitStylingChange(next);
}

void PropertiesPanel::onPickBorderColor() {
    if (!m_node || !m_stack) return;
    QColor current(QString::fromStdString(m_node->style().borderTop.color));
    QColor chosen = QColorDialog::getColor(current, this, "Pick Border Colour");
    if (!chosen.isValid()) return;

    poliable::Style prev = m_node->style();
    m_node->style().borderTop.color = chosen.name().toStdString();
    m_node->style().borderRight.color = chosen.name().toStdString();
    m_node->style().borderBottom.color = chosen.name().toStdString();
    m_node->style().borderLeft.color = chosen.name().toStdString();

    QString style = QString(
        "QPushButton { background: %1; color: %2; "
        "border: 1px solid #555; padding: 3px; border-radius: 2px; }"
    ).arg(chosen.name())
     .arg(chosen.lightness() < 128 ? "#ffffff" : "#000000");
    m_borderColorBtn->setStyleSheet(style);
    m_borderColorBtn->setText(chosen.name().toUpper());

    poliable::Style next = m_node->style();
    commitStylingChange(next);
}

void PropertiesPanel::onFillGradientChanged(int idx) {
    if (!m_node || m_isEditing) return;
    poliable::Style next = m_node->style();
    next.fillGradient.type = (idx == 0) ? poliable::GradientType::None :
                             (idx == 1) ? poliable::GradientType::Linear :
                             (idx == 2) ? poliable::GradientType::Radial : poliable::GradientType::Conic;
    next.isFillGradient = (idx > 0);
    commitStyleChange(next);
}

void PropertiesPanel::onStrokeStyleChanged(int idx) {
    if (!m_node || m_isEditing) return;
    poliable::Style next = m_node->style();
    next.strokeStyle = (idx == 0) ? "solid" : (idx == 1) ? "dashed" : "dotted";
    commitStylingChange(next);
}

void PropertiesPanel::onBorderStyleChanged(int idx) {
    if (!m_node || m_isEditing) return;
    poliable::Style next = m_node->style();
    const char* style = (idx == 0) ? "solid" : (idx == 1) ? "dashed" : (idx == 2) ? "dotted" : "none";
    next.borderTop.style = style;
    next.borderRight.style = style;
    next.borderBottom.style = style;
    next.borderLeft.style = style;
    commitStylingChange(next);
}

void PropertiesPanel::onRadiusChanged(double v) {
    if (!m_node || m_isEditing) return;
    poliable::Style next = m_node->style();
    next.cornerRadius = v;
    next.cornerRadiusTopLeft = v;
    next.cornerRadiusTopRight = v;
    next.cornerRadiusBottomLeft = v;
    next.cornerRadiusBottomRight = v;
    commitStylingChange(next);
}

void PropertiesPanel::commitStyleChange(const poliable::Style& next) {
    if (!m_node || !m_stack) return;
    poliable::Style prev = m_node->style();
    auto cmd = std::make_unique<poliable::SetStyleCommand>(m_node->id(), prev, next);
    m_stack->execute(std::move(cmd));
}

void PropertiesPanel::commitStylingChange(const poliable::Style& next) {
    if (!m_node || !m_stack) return;
    poliable::Style prev = m_node->style();
    auto cmd = std::make_unique<poliable::SetStyleCommand>(m_node->id(), prev, next);
    m_stack->execute(std::move(cmd));
    emit stylingModified();
}
