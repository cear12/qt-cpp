#pragma once

// A QGraphicsItem that can be driven by QPropertyAnimation and can emit
// signals. NOTE on a real bug in the original version of this file: it
// inherited QObject + QGraphicsItem directly and animated a property named
// "pos" via QPropertyAnimation(this, "pos") -- but plain QGraphicsItem has
// no such Q_PROPERTY (it deliberately isn't a QObject, for performance), so
// that animation would silently do nothing at runtime (Qt would print a
// "trying to animate a non-existing property" warning and never move the
// item). QGraphicsObject is Qt's purpose-built fix for exactly this case:
// a QObject + QGraphicsItem combination whose pos/x/y/rotation/scale are
// real, animatable Q_PROPERTYs.
#include <QEasingCurve>
#include <QGraphicsObject>
#include <QPainter>
#include <QPointF>
#include <QPropertyAnimation>
#include <QRectF>
#include <QStyleOptionGraphicsItem>
#include <QWidget>

class AnimatedGraphicsItem : public QGraphicsObject {
  Q_OBJECT

 public:
  AnimatedGraphicsItem() {
    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);

    m_animation = new QPropertyAnimation(this, "pos");
    m_animation->setDuration(2000);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);

    connect(m_animation, &QPropertyAnimation::finished, this,
            &AnimatedGraphicsItem::animationFinished);
  }

  QRectF boundingRect() const override { return QRectF(-50, -50, 100, 100); }

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
             QWidget* widget) override {
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setBrush(isSelected() ? Qt::red : Qt::blue);
    painter->drawEllipse(boundingRect());
  }

 public slots:
  void startAnimation(const QPointF& target) {
    m_animation->setStartValue(pos());
    m_animation->setEndValue(target);
    m_animation->start();
  }

 signals:
  void animationFinished();

 protected:
  void advance(int phase) override {
    if (phase == 0) return;
    update();  // force a repaint each animation step
  }

 private:
  QPropertyAnimation* m_animation;
};
