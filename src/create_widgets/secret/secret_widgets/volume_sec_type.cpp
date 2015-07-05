#include "volume_sec_type.h"

/*
 * http://libvirt.org/formatsecret.html#VolumeUsageType
 */

VolumeSecType::VolumeSecType(QWidget *parent, virConnectPtr _conn) :
    _SecType(parent, _conn)
{
    usage = new QLineEdit(this);
    find  = new QPushButton(QIcon::fromTheme("edit-find"), "", this);
    usageLayout = new QHBoxLayout();
    usageLayout->addWidget(usage);
    usageLayout->addWidget(find);
    usageWdg = new QWidget(this);
    usageWdg->setLayout(usageLayout);
    baseLayout->addWidget(usageWdg);
    connect(find,SIGNAL(clicked(bool)),
            this, SLOT(showVolumeChoose()));
}
QDomDocument VolumeSecType::getSecStuff() const
{
    /*
     * <usage type='volume'>
            <volume>/var/lib/libvirt/images/puppyname.img</volume>
       </usage>
     */
    QDomDocument doc;
    QDomElement _usage, _volume;
    QDomText    _text = doc.createTextNode(usage->text());
    _usage = doc.createElement("usage");
    _usage.setAttribute("type", "volume");
    doc.appendChild(_usage);
    _volume = doc.createElement("volume");
    _volume.appendChild(_text);
    _usage.appendChild(_volume);
    qDebug()<<doc.toByteArray(4).data();
    return doc;
}
void VolumeSecType::showVolumeChoose()
{
    VirtVolumeDialog *dialog = new VirtVolumeDialog(
                this, currWorkConnection);
    dialog->exec();
    qDebug()<<dialog->getResult();
    dialog->deleteLater();
}
