#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QTimer>
#include <QSettings>
#include <QCloseEvent>
#include "log_dock/log_dock.h"
#include "virt_objects/virt_network/virtnet_control.h"
#include "virt_objects/virt_domain/domain_control.h"
#include "virt_objects/virt_storage_vol/storage_vol_control.h"
#include "virt_objects/virt_storage_pool/storage_pool_control.h"
#include "tray/traywidget.h"
#include "layout/conn_list_widget.h"
#include "toolbar/toolbar.h"
#include "wait_thread/wait_thread.h"
#include <QDebug>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:

public slots:

private :
    QSettings                settings;
    ConnectList             *connListWidget;
    TrayIcon                *trayIcon;
    ToolBar                 *toolBar;
    Wait                    *wait_thread = NULL;
    QDockWidget             *logDock;
    LogDock                 *logDockContent;
    QDockWidget             *domainDock;
    VirtDomainControl       *domainDockContent;
    QDockWidget             *networkDock;
    VirtNetControl          *networkDockContent;
    QDockWidget             *storageVolDock;
    VirtStorageVolControl   *storageVolDockContent;
    QDockWidget             *storagePoolDock;
    VirtStoragePoolControl  *storagePoolDockContent;

private slots:
    void closeEvent(QCloseEvent*);
    void closeEvent();
    void changeVisibility();
    void mainWindowUp();
    void initDockWidgets();
    void initTrayIcon();
    void trayIconActivated(QSystemTrayIcon::ActivationReason);
    void initConnListWidget();
    void initToolBar();
    void editCurrentConnect();
    void createNewConnect();
    void deleteCurrentConnect();
    void removeConnectItem(QString&);
    void openCurrentConnect();
    void showCurrentConnect();
    void closeCurrentConnect();
    void closeConnect(int);
    void closeAllConnect();
    bool runningConnectsExist();
    void autoHide();
    void writeToErrorLog(QString&);
    void changeLogViewerVisibility();
    Qt::DockWidgetArea getDockArea(int) const;
    void receiveConnPtr(virConnect*, QString&);
    void stopProcessing();

};

#endif // MAINWINDOW_H