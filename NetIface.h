#ifndef NETIFACE_H
#define NETIFACE_H
#include <QObject>
#include <libnet.h>
#include <QMap>

class NetIface : public QObject
{
Q_OBJECT
public:
	NetIface(QByteArray src_ip, QByteArray iface);
	~NetIface();
	
	static NetIface* getIface(QByteArray src_ip, QByteArray iface);
	static NetIface* getIface(QByteArray dst_ip);
	static void getRoutingInfo(QString dst_ip, QByteArray& iface, QByteArray& src_ip);
	
	void initLibnet();
	void arping(QByteArray dst);
	void wake(QByteArray mac, QByteArray password);
	
	QByteArray interface() const { return m_interface; }
private:
	libnet_t* m_libnet;
	QByteArray m_interface;
	u_int8_t m_ourmac[6];
	qint32 m_source;
	
	static QMap<QByteArray,NetIface*> m_instances;
};

#endif
