#include <QUndoCommand>
#include <QUndoStack>
#include <QString>

class Document : public QObject {
    Q_OBJECT
    QString m_content;
    
public:
    void setContent(const QString& content) {
        if (m_content != content) {
            m_content = content;
            emit contentChanged(content);
        }
    }
    
    QString content() const { return m_content; }
    
signals:
    void contentChanged(const QString& content);
};

class InsertTextCommand : public QUndoCommand {
    Document* m_document;
    QString m_newText;
    QString m_oldText;
    
public:
    InsertTextCommand(Document* doc, const QString& text, QUndoCommand* parent = nullptr)
        : QUndoCommand(parent), m_document(doc), m_newText(text) {
        m_oldText = doc->content();
        setText(QString("Insert '%1'").arg(text));
    }
    
    void redo() override {
        m_document->setContent(m_oldText + m_newText);
    }
    
    void undo() override {
        m_document->setContent(m_oldText);
    }
};

class ReplaceTextCommand : public QUndoCommand {
    Document* m_document;
    QString m_newText;
    QString m_oldText;
    
public:
    ReplaceTextCommand(Document* doc, const QString& newText, QUndoCommand* parent = nullptr)
        : QUndoCommand(parent), m_document(doc), m_newText(newText) {
        m_oldText = doc->content();
        setText(QString("Replace with '%1'").arg(newText));
    }
    
    void redo() override {
        m_document->setContent(m_newText);
    }
    
    void undo() override {
        m_document->setContent(m_oldText);
    }
};

// Использование:
// QUndoStack undoStack;
// Document doc;
// undoStack.push(new InsertTextCommand(&doc, "Hello "));
// undoStack.push(new InsertTextCommand(&doc, "World!"));
