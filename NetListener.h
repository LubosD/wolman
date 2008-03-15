#ifndef NETLISTENER_H
#define NETLISTENER_H
#include <pcap.h>
#include <QThread>
#include <QMap>

class NetListener : public QThread
{
Q_OBJECT
public:
	NetListener(QByteArray interface);
	~NetListener();
	
	static NetListener* getListener(QByteArray interface);
	static void destroyAll();
	
	void start();
protected:
	virtual void run();
	static void processArp(u_char* This, const pcap_pkthdr *h, const u_char* packet);
signals:
	void receivedArp(QByteArray ip, QByteArray mac);
private:
	pcap_t* m_pcap;
	volatile bool m_bTerminate;
	QByteArray m_interface;
	
	static QMap<QByteArray,NetListener*> m_instances;
};

#endif
