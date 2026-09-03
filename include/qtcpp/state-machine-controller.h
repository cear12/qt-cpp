#pragma once

// A tiny 3-state cycle (Init -> Processing -> Finalization -> Init -> ...)
// built on Qt's state-machine framework (Qt6::StateMachine).
#include <QObject>
#include <QState>
#include <QStateMachine>
#include <QString>

class StateMachineController : public QObject {
    Q_OBJECT

public:
    // NOTE on a real bug in the original version of this file: the
    // constructor took a QWidget* parent but never passed it to QObject(),
    // so the parent argument was silently discarded and this controller was
    // always constructed parentless regardless of what callers passed in.
    // Fixed by taking (and actually forwarding) a QObject* parent, which is
    // also the more accurate type -- this class owns no widget.
    explicit StateMachineController(QObject* parent = nullptr) : QObject(parent) {
        machine = new QStateMachine(this);

        s1 = new QState();
        s2 = new QState();
        s3 = new QState();

        s1->addTransition(this, &StateMachineController::next, s2);
        s2->addTransition(this, &StateMachineController::next, s3);
        s3->addTransition(this, &StateMachineController::reset, s1);

        // [this] rather than the original's [=]: C++20 deprecates implicit
        // by-value capture of `this` via a bare `[=]`, and these lambdas
        // only touch `this` anyway.
        connect(s1, &QState::entered, this, [this]() { emit statusChanged("State 1: Initialization"); });
        connect(s2, &QState::entered, this, [this]() { emit statusChanged("State 2: Processing"); });
        connect(s3, &QState::entered, this, [this]() { emit statusChanged("State 3: Finalization"); });

        machine->addState(s1);
        machine->addState(s2);
        machine->addState(s3);
        machine->setInitialState(s1);
        machine->start();
    }

public slots:
    void triggerNext() { emit next(); }
    void triggerReset() { emit reset(); }

signals:
    void next();
    void reset();
    void statusChanged(const QString& status);

private:
    QStateMachine* machine;
    QState *s1, *s2, *s3;
};
