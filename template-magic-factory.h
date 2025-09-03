#include <QObject>
#include <memory>
#include <unordered_map>
#include <functional>

template<typename Base>
class ObjectFactory {
    static_assert(std::is_base_of_v<QObject, Base>, "Base must inherit from QObject");
    
    using CreatorFunc = std::function<std::unique_ptr<Base>()>;
    std::unordered_map<QString, CreatorFunc> m_creators;
    
public:
    template<typename Derived, typename... Args>
    void registerType(const QString& typeName, Args&&... args) {
        static_assert(std::is_base_of_v<Base, Derived>, "Derived must inherit from Base");
        
        m_creators[typeName] = [args...]() -> std::unique_ptr<Base> {
            return std::make_unique<Derived>(args...);
        };
    }
    
    std::unique_ptr<Base> create(const QString& typeName) {
        auto it = m_creators.find(typeName);
        if (it != m_creators.end()) {
            return it->second();
        }
        return nullptr;
    }
    
    QStringList availableTypes() const {
        QStringList types;
        for (const auto& [key, value] : m_creators) {
            types << key;
        }
        return types;
    }
};

// Пример использования
class Shape : public QObject {
    Q_OBJECT
public:
    explicit Shape(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~Shape() = default;
    virtual double area() const = 0;
};

class Circle : public Shape {
    Q_OBJECT
    double m_radius;
    
public:
    explicit Circle(double radius, QObject* parent = nullptr) 
        : Shape(parent), m_radius(radius) {}
        
    double area() const override { return 3.14159 * m_radius * m_radius; }
};

class Rectangle : public Shape {
    Q_OBJECT
    double m_width, m_height;
    
public:
    explicit Rectangle(double width, double height, QObject* parent = nullptr)
        : Shape(parent), m_width(width), m_height(height) {}
        
    double area() const override { return m_width * m_height; }
};

// Использование:
// ObjectFactory<Shape> factory;
// factory.registerType<Circle>("Circle", 5.0);
// factory.registerType<Rectangle>("Rectangle", 10.0, 20.0);
// auto circle = factory.create("Circle");
