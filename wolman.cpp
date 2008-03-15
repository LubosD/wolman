#include <QApplication>
#include <QSettings>
#include "wolman.h"
#include "MainDlg.h"
#include <QMessageBox>
#include <QtDebug>

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	
	QCoreApplication::setOrganizationName("Dolezel");
	QCoreApplication::setOrganizationDomain("dolezel.info");
	QCoreApplication::setApplicationName("wolman");
	
	setuid(0);
	if(getuid())
	{
		QMessageBox::critical(0, "WolMan", "This application doesn't work without root permissions!");
		return 1;
	}
	else
	{
		MainDlg dlg;
		dlg.show();
		
		int retval = app.exec();
		
		NetListener::destroyAll();
		return retval;
	}
}

void loadMachines(QList<Machine>& out)
{
	QSettings settings;
	int size = settings.beginReadArray("machines");
	
	qDebug() << "Reading" << size << "machines";
	
	out.clear();
	
	for(int i=0;i<size;i++)
	{
		Machine mach;
		
		settings.setArrayIndex(i);
		
		mach.name = settings.value("name").toByteArray();
		mach.ipaddr = settings.value("ipaddr").toByteArray();
		mach.macaddr = settings.value("macaddr").toByteArray();
		mach.wakepass = settings.value("wakepass").toByteArray();
		
		out << mach;
	}
	
	settings.endArray();
}

void saveMachines(const QList<Machine>& in)
{
	QSettings settings;
	int size;
	
	settings.beginWriteArray("machines", size = in.size());
	
	qDebug() << "Writing" << size << "machines";
	
	for(int i=0;i<size;i++)
	{
		settings.setArrayIndex(i);
		
		settings.setValue("name", in[i].name);
		settings.setValue("ipaddr", in[i].ipaddr);
		settings.setValue("macaddr", in[i].macaddr);
		settings.setValue("wakepass", in[i].wakepass);
	}
	
	settings.endArray();
}

