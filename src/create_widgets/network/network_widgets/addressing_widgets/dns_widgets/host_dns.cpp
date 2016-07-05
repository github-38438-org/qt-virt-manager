#include "host_dns.h"

Host_DNS::Host_DNS(QWidget *parent, QString tag) :
    _List_Widget(parent, tag)
{
    ipLabel = new QLabel("DNS IP:", this);
    hostIP = new QLineEdit(this);
    hostIP->setPlaceholderText("192.168.122.2");
    ipLayout = new QHBoxLayout();
    ipLayout->addWidget(ipLabel);
    ipLayout->addWidget(hostIP);
    ipWidget = new QWidget(this);
    ipWidget->setLayout(ipLayout);
    hostName = new QLineEdit(this);
    hostName->setPlaceholderText("DNS Host Name");
    baseLayout->insertWidget(0, ipWidget);
    panelLayout->insertWidget(1, hostName);
}

/* public slots */
QDomDocument Host_DNS::getDataDocument() const
{
    QDomDocument doc;
    QDomElement _host;
    _host = doc.createElement("host");
    _host.setAttribute("ip", hostIP->text());
    for (int i=0; i<list->count(); i++) {
        QDomElement _el;
        QDomText data;
        data = doc.createTextNode(
                    list->item(i)->text());
        _el = doc.createElement("hostname");
        _el.appendChild(data);
        _host.appendChild(_el);
    };
    doc.appendChild(_host);
    return doc;
}
void Host_DNS::setDataDescription(const QString &_xmlDesc)
{
}
void Host_DNS::addItem()
{
    QString _text = hostName->text();
    if ( !_text.isEmpty() ) {
        bool exist = false;
        for (int i=0; i<list->count(); i++) {
            if ( _text==list->item(i)->text() ) {
                exist = true;
                break;
            }
        };
        if ( !exist ) list->addItem(_text);
        hostName->clear();
    };
}
