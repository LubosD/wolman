#ifndef MAINDLG_H
#define MAINDLG_H
#include <QDialog>
#include <QTimer>
#include "ui_MainDlg.h"
#include "wolman.h"
#include "NetListener.h"

class MainDlg : public QDialog, Ui_MainDlg
{
Q_OBJECT
public:
	MainDlg();
	~MainDlg();
protected:
	void loadMachines();
	void addItem(Machine& mach);
public slots:
	void about();
	void machineNew();
	void machineEdit();
	void machineDelete();
	void process();
	void receivedArp(QByteArray ip, QByteArray mac);
	void wake();
private:
	QList<Machine> m_machines;
	QTimer m_timer;
	int m_proc;
};

#endif
