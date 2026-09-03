#pragma once

// Command pattern via Qt's built-in undo framework (QUndoStack/QUndoCommand):
// each user action becomes an object with redo()/undo(), and QUndoStack
// handles ordering, merging, and the undo/redo history for you.
#include <QObject>
#include <QString>
#include <QUndoCommand>
#include <QUndoStack>

class Document : public QObject {
    Q_OBJECT
    QString m_content;

public:
    explicit Document(QObject* parent = nullptr) : QObject(parent) {}

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

    void redo() override { m_document->setContent(m_oldText + m_newText); }

    void undo() override { m_document->setContent(m_oldText); }
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

    void redo() override { m_document->setContent(m_newText); }

    void undo() override { m_document->setContent(m_oldText); }
};

// Usage:
// QUndoStack undoStack;
// Document doc;
// undoStack.push(new InsertTextCommand(&doc, "Hello "));
// undoStack.push(new InsertTextCommand(&doc, "World!"));
// See examples/console_demo.cpp for a runnable version of this.
