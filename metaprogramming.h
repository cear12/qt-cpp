#include <QMetaObject>
#include <QMetaProperty>
#include <type_traits>

template<typename T>
class PropertyWrapper {
    static_assert(std::is_base_of_v<QObject, T>, "T must inherit from QObject");
    
    T* m_object;
    
public:
    explicit PropertyWrapper(T* obj) : m_object(obj) {}
    
    template<typename U>
    bool setProperty(const QString& name, const U& value) {
        static_assert(std::is_convertible_v<U, QVariant>, "Value must be convertible to QVariant");
        return m_object->setProperty(name.toLocal8Bit().constData(), QVariant::fromValue(value));
    }
    
    template<typename U>
    U property(const QString& name) const {
        QVariant variant = m_object->property(name.toLocal8Bit().constData());
        if constexpr (std::is_same_v<U, QString>) {
            return variant.toString();
        } else if constexpr (std::is_integral_v<U>) {
            return static_cast<U>(variant.toInt());
        } else {
            return variant.value<U>();
        }
    }
    
    // SFINAE для проверки наличия сигнала
    template<typename Func>
    auto connectToSignal(Func&& func) -> std::enable_if_t<std::is_invocable_v<Func>, bool> {
        // Подключение к сигналу с проверкой типов
        return QObject::connect(m_object, &T::dataChanged, func);
    }
};

// Специализация для различных типов Qt
template<>
class PropertyWrapper<QWidget> {
    QWidget* m_widget;
    
public:
    explicit PropertyWrapper(QWidget* widget) : m_widget(widget) {}
    
    void setVisible(bool visible) { m_widget->setVisible(visible); }
    void setGeometry(const QRect& rect) { m_widget->setGeometry(rect); }
};
