#ifndef MACHINEDLG_H
#define MACHINEDLG_H
#include <QDialog>
#include "ui_MachineDlg.h"
#include "NetListener.h"
#include "wolman.h"

class MachineDlg : public QDialog, Ui_MachineDlg
{
Q_OBJECT
public:
	MachineDlg(QWidget* parent);
	int exec();
	void accept();
public slots:
	void detect();
	void receivedArp(QByteArray ip, QByteArray mac);
public:
	Machine m_machine;
	NetListener* m_listener;
};

#endif
