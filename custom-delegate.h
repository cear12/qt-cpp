class StarDelegate : public QStyledItemDelegate {
    Q_OBJECT
    
public:
    explicit StarDelegate(QWidget *parent = nullptr);
    
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const override;
    
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                     const QModelIndex &index) const override;
    
private slots:
    void commitAndCloseEditor();
    
private:
    mutable QPolygonF starPolygon;
    mutable QPolygonF diamondPolygon;
};

// Применение:
QTableView *view = new QTableView();
view->setItemDelegate(new StarDelegate(view));
