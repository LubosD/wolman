#include "MainDlg.h"
#include "MachineDlg.h"
#include "Exception.h"
#include "NetIface.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QtDebug>

MainDlg::MainDlg()
	: m_proc(10)
{
	setupUi(this);
	
	connect(pushAbout, SIGNAL(clicked()), this, SLOT(about()));
	connect(pushMachineNew, SIGNAL(clicked()), this, SLOT(machineNew()));
	connect(pushMachineEdit, SIGNAL(clicked()), this, SLOT(machineEdit()));
	connect(pushMachineDelete, SIGNAL(clicked()), this, SLOT(machineDelete()));
	
	QHeaderView* hdr = treeMachines->header();
	treeMachines->setColumnCount(4);
	hdr->hide();
	hdr->resizeSection(0, 20);
	hdr->resizeSection(1, 150);
	hdr->resizeSection(2, 80);
	
	loadMachines();
	
	m_timer.start(1000);
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(process()));
	process();
}

MainDlg::~MainDlg()
{
	saveMachines(m_machines);
}

void MainDlg::about()
{
	QMessageBox box(this);
	box.setText(QString::fromUtf8("<b>WolMan</b><p>Copyright © 2008 Luboš Doležel<br>"
					"<a href=\"http://www.dolezel.info\">http://www.dolezel.info</a><p>"
					"Licensed under the terms of GNU GPLv3."));
	box.setWindowTitle("WolMan");
	box.exec();
}

void MainDlg::loadMachines()
{
	::loadMachines(m_machines);
	
	for(int i=0;i<m_machines.size();i++)
	{
		addItem(m_machines[i]);
	}
}

void MainDlg::process()
{
	if(++m_proc >= 10)
	{
		m_proc = 0;
		
		qDebug() << "Sending pings";
		
		try
		{
			for(int i=0;i<m_machines.size();i++)
			{
				QByteArray ip = m_machines[i].ipaddr;
				NetIface* niface = NetIface::getIface(ip);
				
				NetListener* listener = NetListener::getListener(niface->interface());
				
				// ugly - we should find another way to avoid multiple connections
				disconnect(listener, SIGNAL(receivedArp(QByteArray,QByteArray)), this, SLOT(receivedArp(QByteArray,QByteArray)));
				connect(listener, SIGNAL(receivedArp(QByteArray,QByteArray)), this, SLOT(receivedArp(QByteArray,QByteArray)), Qt::QueuedConnection);
				
				m_machines[i].sent = QDateTime::currentDateTime();
				m_machines[i].bResponded = false;
				niface->arping(ip);
			}
		}
		catch(const Exception& e)
		{
			QMessageBox::warning(this, "WolMan", e.what());
		}
	}
	
	QDateTime cur = QDateTime::currentDateTime();
	for(int i=0;i<m_machines.size();i++)
	{
		if(!m_machines[i].bResponded && m_machines[i].sent.secsTo(cur) > 5)
		{
			// make it red
			QWidget* widget = treeMachines->itemWidget(treeMachines->topLevelItem(i), 0);
			static_cast<QLabel*>(widget)->setPixmap(QPixmap(":/red.png"));
		}
	}
}

void MainDlg::receivedArp(QByteArray ip, QByteArray mac)
{
	for(int i=0;i<m_machines.size();i++)
	{
		if(ip == m_machines[i].ipaddr)
		{
			m_machines[i].macaddr = mac;
			m_machines[i].bResponded = true;
			// make it green
			QWidget* widget = treeMachines->itemWidget(treeMachines->topLevelItem(i), 0);
			static_cast<QLabel*>(widget)->setPixmap(QPixmap(":/green.png"));
			break;
		}
	}
}

void MainDlg::addItem(Machine& mach)
{
	QTreeWidgetItem* item = new QTreeWidgetItem(treeMachines);
	mach.button = new QPushButton(tr("Wake"), treeMachines);
	item->setText(1, mach.name);
	item->setText(2, mach.ipaddr);
	item->setSizeHint(0, QSize(16,24));
	
	connect(mach.button, SIGNAL(clicked()), this, SLOT(wake()));
	
	QLabel* icon = new QLabel(treeMachines);
	icon->setPixmap(QPixmap(":/red.png")); // make it red
	
	treeMachines->addTopLevelItem(item);
	treeMachines->setItemWidget(item, 3, mach.button);
	treeMachines->setItemWidget(item, 0, icon);
}

void MainDlg::wake()
{
	try
	{
		int m = -1;
		for(int i=0;i<m_machines.size();i++)
		{
			if((QObject*)m_machines[i].button == sender())
			{
				m = i;
				break;
			}
		}
		
		if(m != -1)
		{
			NetIface* niface = NetIface::getIface(m_machines[m].ipaddr);
			niface->wake(m_machines[m].macaddr, m_machines[m].wakepass);
		}
	}
	catch(const Exception& e)
	{
		QMessageBox::warning(this, "WolMan", e.what());
	}
}

void MainDlg::machineNew()
{
	MachineDlg dlg(this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	
	m_machines << dlg.m_machine;
	addItem(dlg.m_machine);
	saveMachines(m_machines);
	process();
}

void MainDlg::machineEdit()
{
	int row = treeMachines->currentIndex().row();
	if(row < 0)
		return;
	
	MachineDlg dlg(this);
	dlg.m_machine = m_machines[row];
	
	if(dlg.exec() != QDialog::Accepted)
		return;
	
	m_machines[row] = dlg.m_machine;
}

void MainDlg::machineDelete()
{
	int row = treeMachines->currentIndex().row();
	if(row < 0)
		return;
	
	delete treeMachines->takeTopLevelItem(row);
	m_machines.removeAt(row);
	
	saveMachines(m_machines);
}

