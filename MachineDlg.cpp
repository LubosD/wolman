#include "MachineDlg.h"
#include "NetIface.h"
#include "Exception.h"
#include <QRegExp>
#include <QMessageBox>

MachineDlg::MachineDlg(QWidget* parent)
: QDialog(parent)
{
	setupUi(this);
	
	connect(pushDetect, SIGNAL(clicked()), this, SLOT(detect()));
}

int MachineDlg::exec()
{
	int r;
	
	lineName->setText(m_machine.name);
	lineIP->setText(m_machine.ipaddr);
	lineMAC->setText(m_machine.macaddr);
	linePassword->setText(m_machine.wakepass);
	
	r = QDialog::exec();
	if(r == QDialog::Accepted)
	{
		m_machine.name = lineName->text().toLatin1();
		m_machine.ipaddr = lineIP->text().toLatin1();
		m_machine.macaddr = lineMAC->text().toLatin1();
		m_machine.wakepass = linePassword->text().toLatin1();
	}
	
	return r;
}

void MachineDlg::accept()
{
	bool isValid = true;
	QRegExp ip ("(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)");
	QRegExp mac("([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}");
	
	isValid &= !lineName->text().isEmpty();
	isValid &= ip.exactMatch(lineIP->text());
	isValid &= mac.exactMatch(lineMAC->text());
	
	if(isValid)
		QDialog::accept();
}

void MachineDlg::detect()
{
	NetIface* niface = NetIface::getIface(lineIP->text().toLatin1());
	
	m_listener = NetListener::getListener(niface->interface());
	connect(m_listener, SIGNAL(receivedArp(QByteArray,QByteArray)), this, SLOT(receivedArp(QByteArray,QByteArray)));
	
	niface->arping(lineIP->text().toLatin1());
}

void MachineDlg::receivedArp(QByteArray ip, QByteArray mac)
{
	if(ip == lineIP->text() && m_listener)
	{
		lineMAC->setText(mac);
		m_listener->disconnect(this);
		m_listener = 0;
	}
}
