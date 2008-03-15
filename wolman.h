#ifndef WOLMAN_H
#define WOLMAN_H
#include <QDateTime>
#include <QPushButton>

struct Machine
{
	QByteArray name, ipaddr, macaddr;
	QByteArray wakepass;
	
	QDateTime sent;
	bool bResponded;
	
	QPushButton* button;
};

void loadMachines(QList<Machine>& out);
void saveMachines(const QList<Machine>& in);

#endif
