#include <QGuiApplication>
//***************************
//#include <QApplication>
//***************************
#include <QQmlApplicationEngine>
#include <Model/ViewModel/mainviewmodel.h>
#include <Model/ViewModel/jogviewmodel.h>
#include <Model/ViewModel/positionviewmodel.h>
#include <Model/ViewModel/rightviewmodel.h>
#include <Model/ViewModel/interpreterviewmodel.h>
#include <Model/ViewModel/iohandlingviewmodel.h>
#include <Model/ViewModel/scopeviewmodel.h>
#include <Model/ViewModel/iomonitoringviewmodel.h>
#include <Model/ViewModel/teachpointviewmodel.h>
#include <Model/ViewModel/scoordinatesviewmodel.h>
#include <QtQml>
#include <qqmlcontext.h>


int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    Controller *ctrl = Controller::getInstance();
    // Hokmabadi
    //QApplication app(argc, argv);
    // Hokmabadi

    MainViewModel *mvm = new MainViewModel();
    mvm->Initialize();
    //JogViewModel *jvm = new JogViewModel();
    //PositionViewModel *pm = new PositionViewModel();


    qmlRegisterType<MainViewModel>("MainViewModel",1,0,"MainViewModel");

    //******************************
    qmlRegisterType<InterpreterViewModel>("InterpreterViewModel",1,0,"InterpreterViewModel");
    //******************************
    qmlRegisterType<JogViewModel>("JogViewModel",1,0,"JogViewModel");
    qmlRegisterType<PositionViewModel>("PositionViewModel",1,0,"PositionViewModel");
    qmlRegisterType<RightViewModel>("RightViewModel",1,0,"RightViewModel");

    //*******************************
    qmlRegisterType<iohandlingviewmodel>("IOHandlingViewModel",1,0,"IOHandlingViewModel");
    qmlRegisterType<iomonitoringviewmodel>("IOMonitoringViewModel",1,0,"IOMonitoringViewModel");
    qmlRegisterType<teachpointviewmodel>("TeachPointViewModel",1,0,"TeachPointViewModel");
    qmlRegisterType<scoordinatesviewmodel>("ScoordinatesViewModel",1,0,"ScoordinatesViewModel");
    qmlRegisterType<scopeviewmodel>("ScopeViewModel",1,0,"ScopeViewModel");
    //qmlRegisterType<CustomPlotItem>("CustomPlot", 1, 0, "CustomPlotItem");
    //*******************************

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/View/MainView.qml")));

    QQmlContext *ctxt = engine.rootContext();
    ctrl->ctxt = ctxt;
    ctrl->Initialize();

    if (engine.rootObjects().isEmpty())
        return -1;



    return app.exec();
}
