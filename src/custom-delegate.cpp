#include "qtcpp/custom-delegate.h"

#include <QAbstractItemModel>
#include <QPainter>
#include <QSpinBox>
#include <cmath>

namespace {
constexpr int kStarSize = 20;
}

StarDelegate::StarDelegate(QWidget* parent) : QStyledItemDelegate(parent) {
  // A standard 5-pointed star, inscribed in a unit circle centered at
  // (0.5, 0.5): points alternate between the outer radius (the star
  // tips) and an inner radius (the star's inward corners), starting from
  // straight up and going clockwise.
  constexpr double kOuterRadius = 0.5;
  constexpr double kInnerRadius = kOuterRadius * 0.5;
  for (int i = 0; i < 10; ++i) {
    double radius = (i % 2 == 0) ? kOuterRadius : kInnerRadius;
    double angle = -M_PI / 2.0 + i * M_PI / 5.0;
    starPolygon << QPointF(0.5 + radius * std::cos(angle),
                           0.5 + radius * std::sin(angle));
  }

  // A simple diamond, used to mark a rating of zero.
  diamondPolygon << QPointF(0.5, 0.0) << QPointF(1.0, 0.5) << QPointF(0.5, 1.0)
                 << QPointF(0.0, 0.5);
}

void StarDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                         const QModelIndex& index) const {
  int rating = qBound(0, index.data().toInt(), kMaxRating);

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, true);

  if (option.state & QStyle::State_Selected) {
    painter->fillRect(option.rect, option.palette.highlight());
  }

  painter->setPen(Qt::NoPen);
  painter->setBrush(option.state & QStyle::State_Selected
                        ? option.palette.highlightedText()
                        : QBrush(Qt::darkYellow));

  if (rating == 0) {
    painter->translate(
        option.rect.x(),
        option.rect.y() + (option.rect.height() - kStarSize) / 2);
    painter->scale(kStarSize, kStarSize);
    painter->drawPolygon(diamondPolygon);
  } else {
    painter->translate(
        option.rect.x(),
        option.rect.y() + (option.rect.height() - kStarSize) / 2);
    for (int i = 0; i < rating; ++i) {
      painter->save();
      painter->translate(i * kStarSize, 0);
      painter->scale(kStarSize, kStarSize);
      painter->drawPolygon(starPolygon);
      painter->restore();
    }
  }

  painter->restore();
}

QSize StarDelegate::sizeHint(const QStyleOptionViewItem& /*option*/,
                             const QModelIndex& /*index*/) const {
  return QSize(kStarSize * kMaxRating, kStarSize);
}

QWidget* StarDelegate::createEditor(QWidget* parent,
                                    const QStyleOptionViewItem& /*option*/,
                                    const QModelIndex& /*index*/) const {
  // The official Qt "Star Delegate" example implements a fully custom,
  // mouse-tracking StarEditor widget. This uses a plain QSpinBox instead
  // -- less visually interesting, but a real, working editor with far
  // less surface area for bugs in a repo where Qt itself could not be
  // compiled locally to check the result (see the repository README).
  auto* editor = new QSpinBox(parent);
  editor->setRange(0, kMaxRating);
  editor->setFrame(false);
  connect(editor, &QSpinBox::editingFinished, this,
          &StarDelegate::commitAndCloseEditor);
  return editor;
}

void StarDelegate::setEditorData(QWidget* editor,
                                 const QModelIndex& index) const {
  auto* spinBox = qobject_cast<QSpinBox*>(editor);
  if (!spinBox) return;
  spinBox->setValue(qBound(0, index.data(Qt::EditRole).toInt(), kMaxRating));
}

void StarDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                const QModelIndex& index) const {
  auto* spinBox = qobject_cast<QSpinBox*>(editor);
  if (!spinBox) return;
  spinBox->interpretText();
  model->setData(index, spinBox->value(), Qt::EditRole);
}

void StarDelegate::commitAndCloseEditor() {
  auto* editor = qobject_cast<QSpinBox*>(sender());
  if (!editor) return;
  emit commitData(editor);
  emit closeEditor(editor);
}
