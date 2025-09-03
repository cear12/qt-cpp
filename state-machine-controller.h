class StateMachineController : public QObject {
    Q_OBJECT
    
public:
    StateMachineController(QWidget *parent = nullptr) {
        machine = new QStateMachine(this);
        
        // Создаем состояния
        s1 = new QState();
        s2 = new QState();  
        s3 = new QState();
        
        // Настраиваем переходы
        s1->addTransition(this, &StateMachineController::next, s2);
        s2->addTransition(this, &StateMachineController::next, s3);
        s3->addTransition(this, &StateMachineController::reset, s1);
        
        // Действия при входе в состояния
        connect(s1, &QState::entered, [=]() { 
            emit statusChanged("State 1: Initialization"); 
        });
        connect(s2, &QState::entered, [=]() { 
            emit statusChanged("State 2: Processing"); 
        });
        connect(s3, &QState::entered, [=]() { 
            emit statusChanged("State 3: Finalization"); 
        });
        
        // Настраиваем машину
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
    void statusChanged(const QString &status);
    
private:
    QStateMachine *machine;
    QState *s1, *s2, *s3;
};
