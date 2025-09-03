#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>
#include <memory>
#include <vector>

// Component interface
class IUIComponent {
public:
    virtual ~IUIComponent() = default;
    virtual QWidget* widget() = 0;
    virtual void setVisible(bool visible) = 0;
    virtual void setEnabled(bool enabled) = 0;
    virtual QSize sizeHint() const = 0;
};

// Leaf components
class LabelComponent : public IUIComponent {
    std::unique_ptr<QLabel> m_label;
    
public:
    explicit LabelComponent(const QString& text) {
        m_label = std::make_unique<QLabel>(text);
    }
    
    QWidget* widget() override { return m_label.get(); }
    void setVisible(bool visible) override { m_label->setVisible(visible); }
    void setEnabled(bool enabled) override { m_label->setEnabled(enabled); }
    QSize sizeHint() const override { return m_label->sizeHint(); }
    
    void setText(const QString& text) { m_label->setText(text); }
};

class ButtonComponent : public QObject, public IUIComponent {
    Q_OBJECT
    std::unique_ptr<QPushButton> m_button;
    std::function<void()> m_clickHandler;
    
public:
    explicit ButtonComponent(const QString& text, std::function<void()> handler = nullptr)
        : m_button(std::make_unique<QPushButton>(text)), m_clickHandler(handler) {
        
        if (m_clickHandler) {
            connect(m_button.get(), &QPushButton::clicked, m_clickHandler);
        }
    }
    
    QWidget* widget() override { return m_button.get(); }
    void setVisible(bool visible) override { m_button->setVisible(visible); }
    void setEnabled(bool enabled) override { m_button->setEnabled(enabled); }
    QSize sizeHint() const override { return m_button->sizeHint(); }
};

// Composite component
class PanelComponent : public IUIComponent {
public:
    enum LayoutType { Vertical, Horizontal };
    
private:
    std::unique_ptr<QWidget> m_widget;
    std::unique_ptr<QBoxLayout> m_layout;
    std::vector<std::unique_ptr<IUIComponent>> m_children;
    
public:
    explicit PanelComponent(LayoutType layoutType = Vertical) {
        m_widget = std::make_unique<QWidget>();
        
        if (layoutType == Vertical) {
            m_layout = std::make_unique<QVBoxLayout>(m_widget.get());
        } else {
            m_layout = std::make_unique<QHBoxLayout>(m_widget.get());
        }
    }
    
    QWidget* widget() override { return m_widget.get(); }
    
    void setVisible(bool visible) override {
        m_widget->setVisible(visible);
        for (auto& child : m_children) {
            child->setVisible(visible);
        }
    }
    
    void setEnabled(bool enabled) override {
        m_widget->setEnabled(enabled);
        for (auto& child : m_children) {
            child->setEnabled(enabled);
        }
    }
    
    QSize sizeHint() const override { return m_widget->sizeHint(); }
    
    void addComponent(std::unique_ptr<IUIComponent> component) {
        m_layout->addWidget(component->widget());
        m_children.push_back(std::move(component));
    }
    
    void addSpacer() {
        m_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
    }
    
    template<typename T, typename... Args>
    T* createAndAdd(Args&&... args) {
        static_assert(std::is_base_of_v<IUIComponent, T>, "T must implement IUIComponent");
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = component.get();
        addComponent(std::move(component));
        return ptr;
    }
};

// Builder для создания сложных UI
class UIBuilder {
    std::unique_ptr<PanelComponent> m_root;
    std::stack<PanelComponent*> m_panelStack;
    
public:
    UIBuilder() {
        m_root = std::make_unique<PanelComponent>();
        m_panelStack.push(m_root.get());
    }
    
    UIBuilder& addLabel(const QString& text) {
        m_panelStack.top()->createAndAdd<LabelComponent>(text);
        return *this;
    }
    
    UIBuilder& addButton(const QString& text, std::function<void()> handler = nullptr) {
        m_panelStack.top()->createAndAdd<ButtonComponent>(text, handler);
        return *this;
    }
    
    UIBuilder& beginPanel(PanelComponent::LayoutType type = PanelComponent::Vertical) {
        auto panel = m_panelStack.top()->createAndAdd<PanelComponent>(type);
        m_panelStack.push(panel);
        return *this;
    }
    
    UIBuilder& endPanel() {
        if (m_panelStack.size() > 1) {
            m_panelStack.pop();
        }
        return *this;
    }
    
    UIBuilder& addSpacer() {
        m_panelStack.top()->addSpacer();
        return *this;
    }
    
    std::unique_ptr<PanelComponent> build() {
        return std::move(m_root);
    }
};
