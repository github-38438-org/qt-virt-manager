#ifndef CREATE_VIRT_NETWORK_H
#define CREATE_VIRT_NETWORK_H

#include <QMainWindow>
#include <QMap>
#include <QSettings>
#include <QDir>
#include <QTemporaryFile>
#include <QTabWidget>
#include <QPushButton>
#include <QCloseEvent>
#include <QTime>
#include <virt_objects/virt_entity_config.h>
#include <virt_objects/_virt_thread.h>
#include "create_virt_network_adv.h"
#include <QDebug>

class NetHelperThread : public _VirtThread
{
    Q_OBJECT
public:
    explicit NetHelperThread(
            QObject        *parent      = nullptr,
            virConnectPtr  *connPtrPtr  = nullptr);
    void             run();
};

class CreateVirtNetwork : public QMainWindow
{
    Q_OBJECT
public:
    explicit CreateVirtNetwork(
            QWidget *parent = nullptr,
            TASK     _task  = TASK());
    ~CreateVirtNetwork();

signals:
    void             errorMsg(QString&);
    void             finished(QString&);
    void             addNewTask(TASK);

private:
    QSettings        settings;
    virConnectPtr*   ptr_ConnPtr = nullptr;
    QString          xmlFileName, xmlDesc;
    virErrorPtr      virtErrors;

    QTemporaryFile  *xml = nullptr;
    bool             ready = false;
    TASK             task;

    NetHelperThread *helperThread;
    CreateVirtNetwork_Adv
                    *advancedWdg = nullptr;

public slots:
    void             closeEvent(QCloseEvent*);
    void             readCapabilities();
    void             readDataLists();
    bool             buildXMLDescription();
    void             set_Result();
    void             setNewWindowTitle(const QString&);
    void             sendMsg(QString&);
};

#endif // CREATE_VIRT_NETWORK_H
