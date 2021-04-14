#include <QApplication>
#include <QDir>
#include <QMetaType>
#include "MainWindow.h"
#include <GSLAM/core/GSLAM.h>

using namespace GSLAM;

Q_DECLARE_METATYPE(Svar);

int run(Svar config){
    GSLAM::MainWindow* mainWindow=nullptr;

    std::string dataset=config.arg<std::string>("dataset","","The dataset going to play.");

    bool byMessenger=false;
    Subscriber subStop=messenger.subscribe("messenger/stop",0,[&](bool stop){
        byMessenger=true;
        messenger.publish("qviz/ui_thread_run",SvarFunction([]()->void{
            LOG(INFO)<<"Hello from ui";
        }));
        if(mainWindow)
            mainWindow->shutdown();
    });

    Subscriber sub_dataset_status=messenger.subscribe("dataset/status",[&mainWindow](int status){
        std::cout << "Dataset status updated to "<< status << std::endl;;
        // DLOG(INFO)<<"Dataset status updated to "<<status;
        if(mainWindow) mainWindow->datasetStatusUpdated(status);
    });
    Subscriber sub_panel=messenger.subscribe("qviz/add_panel",[&mainWindow](QWidget* panel)
    {
        if(!mainWindow) return;
        mainWindow->addPanel(panel);
    });

    Subscriber sub_tab=messenger.subscribe("qviz/add_tab",[&mainWindow](QWidget* tab){
            if(!mainWindow) return;
            mainWindow->addTab(tab);
    });

    Subscriber sub_menu=messenger.subscribe("qviz/add_menu",[&mainWindow](Svar menu){
        if(!mainWindow) return;
        mainWindow->addMenu(menu);
    });

    Subscriber sub_tool=messenger.subscribe("qviz/add_tool",[&mainWindow](Svar tool){
        if(!mainWindow) return;
        mainWindow->addTool(tool);
    });

    Subscriber sub_run=messenger.subscribe("qviz/ui_thread_run",[&mainWindow](Svar func){
        if(!mainWindow) return;
        mainWindow->uiRun(func);
    });

    if(config.get("help",false)){
        Publisher  pub_gui=messenger.advertise<std::string>("dataset/control",0);
        Publisher  pub_draw=messenger.advertise<Svar>("qviz/gl_draw");

        config["__usage__"]=messenger.introduction();
        return config.help();
    }

    QApplication app(config.GetInt("argc"),
                     config.get<char**>("argv",nullptr));

    qRegisterMetaType<Svar>("Svar");
    mainWindow=new GSLAM::MainWindow(nullptr,config);
    mainWindow->show();
    messenger.publish("qviz/ready",true);

    int ret= app.exec();
    if(!byMessenger)
        messenger.publish("messenger/stop",true);
    return ret;
}

GSLAM_REGISTER_APPLICATION(qviz,run);
