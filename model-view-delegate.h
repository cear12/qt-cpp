#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QApplication>
#include <vector>

// Custom Data Model
class PersonModel : public QAbstractTableModel {
    Q_OBJECT
    
    struct Person {
        QString name;
        int age;
        QString email;
    };
    
    std::vector<Person> m_persons;
    
public:
    enum Column {
        NameColumn = 0,
        AgeColumn,
        EmailColumn,
        ColumnCount
    };
    
    explicit PersonModel(QObject* parent = nullptr) : QAbstractTableModel(parent) {
        m_persons = {
            {"Alice", 25, "alice@example.com"},
            {"Bob", 30, "bob@example.com"},
            {"Charlie", 35, "charlie@example.com"}
        };
    }
    
    // Model interface implementation
    int rowCount(const QModelIndex& = QModelIndex()) const override {
        return static_cast<int>(m_persons.size());
    }
    
    int columnCount(const QModelIndex& = QModelIndex()) const override {
        return ColumnCount;
    }
    
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= static_cast<int>(m_persons.size()))
            return QVariant();
            
        const Person& person = m_persons[index.row()];
        
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            switch (index.column()) {
                case NameColumn: return person.name;
                case AgeColumn: return person.age;
                case EmailColumn: return person.email;
            }
        }
        return QVariant();
    }
    
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
                case NameColumn: return tr("Name");
                case AgeColumn: return tr("Age");
                case EmailColumn: return tr("Email");
            }
        }
        return QVariant();
    }
    
    Qt::ItemFlags flags(const QModelIndex& index) const override {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }
    
    bool setData(const QModelIndex& index, const QVariant& value, int role) override {
        if (role == Qt::EditRole && index.isValid()) {
            Person& person = m_persons[index.row()];
            switch (index.column()) {
                case NameColumn:
                    person.name = value.toString();
                    break;
                case AgeColumn:
                    person.age = value.toInt();
                    break;
                case EmailColumn:
                    person.email = value.toString();
                    break;
                default:
                    return false;
            }
            emit dataChanged(index, index);
            return true;
        }
        return false;
    }
};

// Custom Delegate
class PersonDelegate : public QStyledItemDelegate {
    Q_OBJECT
    
public:
    explicit PersonDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}
    
    void paint(QPainter* painter, const QStyleOptionViewItem& option, 
               const QModelIndex& index) const override {
        QStyledItemDelegate::paint(painter, option, index);
        
        // Custom painting for age column
        if (index.column() == PersonModel::AgeColumn) {
            int age = index.data().toInt();
            if (age < 30) {
                painter->fillRect(option.rect, QColor(200, 255, 200, 100));
            }
        }
    }
};
