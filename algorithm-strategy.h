#include <QObject>
#include <QPixmap>
#include <QPainter>
#include <memory>
#include <functional>

// Strategy interface
class IImageFilter {
public:
    virtual ~IImageFilter() = default;
    virtual QPixmap apply(const QPixmap& input) = 0;
    virtual QString name() const = 0;
};

// Concrete strategies
class BlurFilter : public IImageFilter {
    int m_radius;
    
public:
    explicit BlurFilter(int radius = 5) : m_radius(radius) {}
    
    QPixmap apply(const QPixmap& input) override {
        // Blur implementation
        QPixmap result = input;
        QPainter painter(&result);
        painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter.fillRect(result.rect(), QColor(0, 0, 0, 50));
        return result;
    }
    
    QString name() const override { return QString("Blur (radius: %1)").arg(m_radius); }
};

class SepiaFilter : public IImageFilter {
public:
    QPixmap apply(const QPixmap& input) override {
        QPixmap result = input;
        // Sepia effect implementation
        QPainter painter(&result);
        painter.setCompositionMode(QPainter::CompositionMode_Overlay);
        painter.fillRect(result.rect(), QColor(255, 204, 153, 100));
        return result;
    }
    
    QString name() const override { return "Sepia"; }
};

// Context class
class ImageProcessor : public QObject {
    Q_OBJECT
    QPixmap m_originalImage;
    std::vector<std::unique_ptr<IImageFilter>> m_filters;
    
public:
    explicit ImageProcessor(QObject* parent = nullptr) : QObject(parent) {
        // Register default filters
        addFilter(std::make_unique<BlurFilter>());
        addFilter(std::make_unique<SepiaFilter>());
    }
    
    void setImage(const QPixmap& image) {
        m_originalImage = image;
        emit imageChanged();
    }
    
    void addFilter(std::unique_ptr<IImageFilter> filter) {
        m_filters.push_back(std::move(filter));
        emit filtersChanged();
    }
    
    QPixmap applyFilter(int filterIndex) {
        if (filterIndex < 0 || filterIndex >= static_cast<int>(m_filters.size())) {
            return m_originalImage;
        }
        
        return m_filters[filterIndex]->apply(m_originalImage);
    }
    
    QStringList availableFilters() const {
        QStringList names;
        for (const auto& filter : m_filters) {
            names << filter->name();
        }
        return names;
    }
    
    // Functional approach alternative
    template<typename Func>
    QPixmap applyCustomFilter(Func&& filterFunc) {
        static_assert(std::is_invocable_r_v<QPixmap, Func, QPixmap>, 
                     "Filter function must take QPixmap and return QPixmap");
        return filterFunc(m_originalImage);
    }
    
signals:
    void imageChanged();
    void filtersChanged();
};
