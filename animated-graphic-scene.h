class AnimatedGraphicsItem : public QObject, public QGraphicsItem {
    Q_OBJECT
    
public:
    AnimatedGraphicsItem() {
        setFlag(ItemIsMovable);
        setFlag(ItemIsSelectable);
        
        // Создаем анимацию
        m_animation = new QPropertyAnimation(this, "pos");
        m_animation->setDuration(2000);
        m_animation->setEasingCurve(QEasingCurve::InOutQuad);
        
        connect(m_animation, &QPropertyAnimation::finished,
                this, &AnimatedGraphicsItem::animationFinished);
    }
    
    QRectF boundingRect() const override {
        return QRectF(-50, -50, 100, 100);
    }
    
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override {
        Q_UNUSED(option)
        Q_UNUSED(widget)
        
        painter->setBrush(isSelected() ? Qt::red : Qt::blue);
        painter->drawEllipse(boundingRect());
    }
    
public slots:
    void startAnimation(const QPointF &target) {
        m_animation->setStartValue(pos());
        m_animation->setEndValue(target);
        m_animation->start();
    }
    
signals:
    void animationFinished();
    
protected:
    void advance(int phase) override {
        if (phase == 0) return;
        update(); // Принудительное обновление
    }
    
private:
    QPropertyAnimation *m_animation;
};
