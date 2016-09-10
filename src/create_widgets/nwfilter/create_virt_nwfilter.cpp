#include "create_virt_nwfilter.h"

NetFilterHelperThread::NetFilterHelperThread(
        QObject *parent, virConnectPtr *connPtrPtr) :
    _VirtThread(parent, connPtrPtr)
{

}
void NetFilterHelperThread::run()
{
    if ( nullptr==ptr_ConnPtr || nullptr==*ptr_ConnPtr ) {
        emit ptrIsNull();
        return;
    };
    if ( virConnectRef(*ptr_ConnPtr)<0 ) {
        sendConnErrors();
        return;
    };
    // something data reading
    virNWFilterPtr *filters = nullptr;
    //extra flags; not used yet, so callers should always pass 0
    unsigned int flags = 0;
    int ret = virConnectListAllNWFilters(
                *ptr_ConnPtr, &filters, flags);
    if ( ret<0 ) {
        sendConnErrors();
        return;
    };
    // therefore correctly to use for() command,
    // because filters[0] can not exist.
    for (int i = 0; i < ret; i++) {
        const char* name = virNWFilterGetName(filters[i]);
        filterList.append(QString::fromUtf8(name));
        virNWFilterFree(filters[i]);
    };
    if (filters) free(filters);
    if ( virConnectClose(*ptr_ConnPtr)<0 )
        sendConnErrors();
}

CreateVirtNWFilter::CreateVirtNWFilter(
        QWidget *parent, TASK _task) :
    QMainWindow(parent), task(_task)
{
    setAttribute(Qt::WA_DeleteOnClose);
    xmlFileName = task.args.path;
    ptr_ConnPtr = task.srcConnPtr;
    setWindowTitle("NWFilter Editor");
    setWindowIcon(QIcon::fromTheme("nwfilter"));
    setEnabled(false);

    name = new QLineEdit(this);
    name->setPlaceholderText("set filter name");
    uuid = new QLineEdit(this);
    uuid->setPlaceholderText("UUID will set automatically if occured");

    tabs = new QTabWidget(this);
    tabs->setTabPosition(QTabWidget::West);
    filterRefs = new FilterrefWidget(this, "Use another filters");
    chainRules = new ChainRules(this);
    tabs->addTab(filterRefs, "Filters");
    tabs->addTab(chainRules, "Chain");
    //tabs->addTab(new QWidget(), "Filter tree");

    about = new QLabel(
    "<a href='https://libvirt.org/formatnwfilter.html'>About</a>",
                this);
    about->setOpenExternalLinks(true);
    about->setToolTip("https://libvirt.org/formatnwfilter.html");
    ok = new QPushButton(
                QIcon::fromTheme("dialog-ok"),
                "Ok",
                this);
    ok->setAutoDefault(true);
    connect(ok, SIGNAL(clicked()),
            this, SLOT(set_Result()));
    cancel = new QPushButton(
                QIcon::fromTheme("dialog-cancel"),
                "Cancel",
                this);
    cancel->setAutoDefault(true);
    connect(cancel, SIGNAL(clicked()),
            this, SLOT(set_Result()));
    buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(about);
    buttonLayout->addWidget(ok);
    buttonLayout->addWidget(cancel);
    buttons = new QWidget(this);
    buttons->setLayout(buttonLayout);

    commonLayout = new QVBoxLayout(this);
    commonLayout->addWidget(name);
    commonLayout->addWidget(uuid);
    commonLayout->addWidget(tabs);
    commonLayout->addWidget(buttons);
    //commonLayout->addStretch(-1);
    commonWdg = new QWidget(this);
    commonWdg->setLayout(commonLayout);
    setCentralWidget(commonWdg);

    xml = new QTemporaryFile(this);
    xml->setAutoRemove(false);
    xml->setFileTemplate(
                QString("%1%2XML_Desc-XXXXXX.xml")
                .arg(QDir::tempPath())
                .arg(QDir::separator()));
    connect(name, SIGNAL(textChanged(const QString&)),
            this, SLOT(setNewWindowTitle(const QString&)));
    hlpThread = new NetFilterHelperThread(this, ptr_ConnPtr);
    connect(hlpThread, SIGNAL(finished()),
            this, SLOT(readCapabilities()));
    connect(hlpThread, SIGNAL(errorMsg(const QString&, const uint)),
            this, SIGNAL(errorMsg(const QString&)));
    hlpThread->start();
}
CreateVirtNWFilter::~CreateVirtNWFilter()
{
    settings.beginGroup("VirtNWFilterControl");
    settings.setValue("NWFilterEditor", saveGeometry());
    //settings.setValue(
    //            "NetCreateShowDesc",
    //            showDescription->isChecked());
    settings.endGroup();
}

void CreateVirtNWFilter::readXmlDescData(const QString &_xmlDesc)
{
    QDomDocument doc;
    doc.setContent(_xmlDesc);
    QDomElement _filter;
    _filter = doc.firstChildElement("filter");
    if ( !_filter.isNull() ) {
        QDomElement _uuid;
        QString _name = _filter.attribute("name");
        name->setText(_name);
        _uuid = _filter.firstChildElement("uuid");
        if ( !_uuid.isNull() )
            uuid->setText(_uuid.text());
        filterRefs->readXmlDescData(_xmlDesc);
        chainRules->readXmlDescData(_xmlDesc);
    };
}
void CreateVirtNWFilter::buildXMLDescription()
{
    QString _xml;
    QStringList data;
    data.append("New NWFilter XML'ed");
    data.append(QString("to <a href='%1'>%1</a>").arg(_xml));
    QString msg = data.join(" ");
    sendMsg(msg);
    // if ( showDescription->isChecked() )
    //     QDesktopServices::openUrl(QUrl(_xml));
    task.args.path = _xml;
}
void CreateVirtNWFilter::sendMsg(const QString &msg)
{
    QString time = QTime::currentTime().toString();
    QString title = QString("Connection '%1'").arg(task.srcConName);
    QString currMsg = QString(
    "<b>%1 %2:</b><br><font color='blue'><b>EVENT</b></font>: %3")
            .arg(time).arg(title).arg(msg);
    emit errorMsg(currMsg);
}

/* private slots */
void CreateVirtNWFilter::closeEvent(QCloseEvent *ev)
{
    if ( ev->type()==QEvent::Close ) {
        QString key = objectName();
        QString msg = QString(
        "'<b>%1</b>' network filter editor closed.")
                .arg(task.object);
        sendMsg(msg);
        emit finished(key);
    };
}
void CreateVirtNWFilter::readCapabilities()
{
    filterRefs->setFilters(hlpThread->filterList);
    if ( !xmlFileName.isEmpty() ) {
        // read for edit existing NWFilter
        QFile *_xml = new QFile(this);
        _xml->setFileName(xmlFileName);
        _xml->open(QIODevice::ReadOnly);
        xmlDesc.append(_xml->readAll().constData());
        _xml->close();
        _xml->deleteLater();
    };
    readXmlDescData(xmlDesc);
    settings.beginGroup("VirtNWFilterControl");
    restoreGeometry(settings.value("NWFilterEditor").toByteArray());
    settings.endGroup();
    setEnabled(true);
}
void CreateVirtNWFilter::set_Result()
{
    this->setEnabled(false);
    if ( sender()==ok ) {
        buildXMLDescription();
        emit addNewTask(task);
    };
    close();
}
void CreateVirtNWFilter::setNewWindowTitle(const QString &_name)
{
    QString connName = task.srcConName;
    setWindowTitle(
                QString("NWFilter Editor / <%1> in [%2]")
                .arg(_name).arg(connName));
}