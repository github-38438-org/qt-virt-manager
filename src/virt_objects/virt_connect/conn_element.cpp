#include "conn_element.h"

ElemConnect::ElemConnect(QObject *parent) :
    QObject(parent)
{
    connect(this, SIGNAL(connectState(bool)), this, SLOT(setConnectState(bool)));
    //connect(this, SIGNAL(readyRead()), this, SLOT(sendMessage()));
    timerId = 0;
    waitTimerId = 0;
    _diff = 0;

    connAliveThread = new ConnAliveThread(this);
    //connect(connAliveThread, SIGNAL(started()), this, SLOT(connectStarted()));
    connect(connAliveThread, SIGNAL(finished()), this, SLOT(connectFinished()));
    connect(connAliveThread, SIGNAL(connMsg(QString)), this, SLOT(receiveConnMessage(QString)));
    connect(connAliveThread, SIGNAL(connected()), this, SLOT(connectOpened()));
}
ElemConnect::~ElemConnect()
{
    //disconnect(connAliveThread, SIGNAL(started()), this, SLOT(connectStarted()));
    disconnect(connAliveThread, SIGNAL(finished()), this, SLOT(connectFinished()));
    disconnect(connAliveThread, SIGNAL(connMsg(QString)), this, SLOT(receiveConnMessage(QString)));
    disconnect(connAliveThread, SIGNAL(connected()), this, SLOT(connectOpened()));
    disconnect(this, SIGNAL(connectState(bool)), this, SLOT(setConnectState(bool)));
    //disconnect(this, SIGNAL(readyRead()), this, SLOT(sendMessage()));

    if ( connAliveThread!=NULL ) {
        connAliveThread->setKeepAlive(false);
        connAliveThread->quit();
        delete connAliveThread;
        connAliveThread = 0;
    };
}
void ElemConnect::setItemReference(ConnItemModel *model, ConnItemIndex *idx)
{
    own_model = model;
    own_index = idx;
    name = idx->getName();
    conn_Status.insert("availability", QVariant(AVAILABLE));
    conn_Status.insert("isRunning", QVariant(STOPPED));
    conn_Status.insert("initName", QVariant(name));
    idx->setData(conn_Status);
    settings.beginGroup("Connects");
    settings.beginGroup(name);
    bool atStart = settings.value("AtStart", QVariant(false)).toBool();
    settings.endGroup();
    settings.endGroup();
    if ( atStart ) openConnect();
}
void ElemConnect::buildCommand()
{
    settings.beginGroup("Connects");
    settings.beginGroup(name);
    checkTimeout = settings.value("TimeOut", TIMEOUT).toInt();
    QString Driver = settings.value("Driver", "").toString();
    QString Transport = settings.value("Transport", "").toString();
    Host = settings.value("Host", "").toString();
    QString Extra = settings.value("Extra", "").toString();
    settings.endGroup();
    settings.endGroup();

    // URI building
    QStringList _uri;
    _uri.append(Driver.toLower());
    if ( !Transport.isEmpty() ) {
        _uri.append("+");
        _uri.append(Transport.toLower());
    };
    _uri.append("://");
    if ( !Host.isEmpty() ) {
        _uri.append(Host);
    };
    _uri.append("/");
    if ( !Extra.isEmpty() ) {
        //_uri.append("?");
        _uri.append(Extra);
    };
    URI = _uri.join("");
}
void ElemConnect::connectStarted()
{
    emit connectState(RUNNING);
}
void ElemConnect::connectOpened()
{
    _diff = checkTimeout + 1;
}
void ElemConnect::connectFinished()
{
    emit connectState(STOPPED);
}
void ElemConnect::openConnect()
{
    conn_Status.insert("availability", QVariant(NOT_AVAILABLE));
    conn_Status.insert("isRunning", QVariant(RUNNING));
    own_index->setData(conn_Status);
    buildCommand();
    connAliveThread->setData(URI);
    connAliveThread->start();
    if (!waitTimerId) {
        waitTimerId = startTimer(1000);
    } else {
        addMsgToLog(QString("Connect '%1'").arg(name),
            QString("Wait Timer is running: %1").arg(waitTimerId));
    };
}
void ElemConnect::showConnectData()
{
    virConnect *conn = NULL;
    conn = connAliveThread->getConnect();
    //qDebug()<<"showConnectData:"<<name<<QVariant((conn!=NULL)?true:false).toString()<<conn;
    emit connPtr(conn, name);
}
void ElemConnect::closeConnect()
{
    connAliveThread->setKeepAlive(false);
}
void ElemConnect::setConnectState(bool status)
{
  if ( status ) {
      conn_Status.insert("isRunning", QVariant(RUNNING));
  } else {
      if (waitTimerId) {
          killTimer(waitTimerId);
          waitTimerId = 0;
      };
      if (timerId) {
          killTimer(timerId);
          timerId = 0;
      };
      Host.clear();
      Host.append("-");
      conn_Status.insert("isRunning", QVariant(STOPPED));
  };
  conn_Status.insert("availability", QVariant(AVAILABLE));
  own_index->setData(conn_Status);
  int row = own_model->connItemDataList.indexOf(own_index);
  QString data;
  for (int i=0; i<own_model->columnCount(); i++) {
      switch (i) {
      case 0:
          data = name;
          break;
      case 1:
          data = Host;
          break;
      case 2:
          data = (status) ? "OPEN":"CLOSE";
          break;
      default:
          break;
      };
      own_model->setData(own_model->index(row, i), data, Qt::EditRole);
  };
}
void ElemConnect::timerEvent(QTimerEvent *event)
{
    int percent = 0;
    int _timerId = event->timerId();
    if ( _timerId && timerId==_timerId ) {
        // TODO: use to check alive connect the metod from connAliveThread
        if ( !connAliveThread->getKeepAlive() ) {
            emit connectState(STOPPED);
        };
    } else if ( _timerId && waitTimerId==_timerId ) {
        if ( checkTimeout - _diff + 1 ) {
            percent = int ((float(_diff)/checkTimeout)*100.0);
            QModelIndex _idx = own_model->index( own_model->connItemDataList.indexOf( own_index ), 2 );
            own_model->setData(_idx, QString::number(percent), Qt::EditRole);
            _diff++;
        } else {
            killTimer(waitTimerId);
            waitTimerId = 0;
            _diff = 0;
            emit connectState(RUNNING);
            if (!timerId) timerId = startTimer(1000);
            else addMsgToLog(QString("Connect '%1'").arg(name), QString("Wait Timer is running: %1").arg(waitTimerId));
        };
    };
}
void ElemConnect::receiveConnMessage(QString msg)
{
    addMsgToLog( QString("Connect '%1'").arg(name), msg );
}
void ElemConnect::addMsgToLog(QString title, QString msg)
{
    QString time = QTime::currentTime().toString();
    QString errorMsg = QString("<b>%1 %2:</b><br>%3").arg(time).arg(title).arg(msg);
    sendWarning(errorMsg);
    mainWindowUp();
}
void ElemConnect::sendWarning(QString &msg)
{
    emit warning(msg);
}
void ElemConnect::mainWindowUp()
{
    emit warningShowed();
}