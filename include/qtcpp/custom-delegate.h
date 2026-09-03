#pragma once

// A star-rating delegate for QAbstractItemView, in the spirit of Qt's own
// "Star Delegate" example: paints an integer rating (0-5) as stars instead
// of a plain number, and edits it with a QSpinBox.
//
// NOTE: the original version of this file ended with
//   QTableView *view = new QTableView();
//   view->setItemDelegate(new StarDelegate(view));
// sitting directly in the header, outside of any function -- the first
// line is (oddly) legal at namespace scope as a global variable
// definition, but the second is a bare statement, which is not legal
// outside a function body, so this header could never compile if included
// anywhere. That usage now lives in examples/widgets_demo.cpp.
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QPainter>
#include <QPolygonF>
#include <QSize>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>
#include <QWidget>

class StarDelegate : public QStyledItemDelegate {
  Q_OBJECT

 public:
  explicit StarDelegate(QWidget* parent = nullptr);

  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;

  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override;

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                        const QModelIndex& index) const override;

  void setEditorData(QWidget* editor, const QModelIndex& index) const override;
  void setModelData(QWidget* editor, QAbstractItemModel* model,
                    const QModelIndex& index) const override;

 private slots:
  void commitAndCloseEditor();

 private:
  static constexpr int kMaxRating = 5;

  QPolygonF starPolygon;     // used to paint each earned star
  QPolygonF diamondPolygon;  // used to mark a rating of zero
};
