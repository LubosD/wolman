#include "NetIface.h"
#include <QProcess>
#include <QRegExp>
#include <QtDebug>
#include "Exception.h"

#ifndef ETHERTYPE_WOL
#	define ETHERTYPE_WOL 0x0842
#endif

QMap<QByteArray,NetIface*> NetIface::m_instances;

NetIface::NetIface(QByteArray src_ip, QByteArray iface)
	: m_libnet(0), m_interface(iface)
{
	m_instances[iface] = this;
	m_source = libnet_name2addr4(m_libnet, src_ip.data(), LIBNET_RESOLVE);
	initLibnet();
}

NetIface::~NetIface()
{
	if(m_libnet)
		libnet_destroy(m_libnet);
}

NetIface* NetIface::getIface(QByteArray src_ip, QByteArray iface)
{
	if(m_instances.contains(iface))
		return m_instances[iface];
	else
	{
		NetIface* n = new NetIface(src_ip, iface);
		return n;
	}
}

NetIface* NetIface::getIface(QByteArray dst_ip)
{
	QByteArray iface, src_ip;
	getRoutingInfo(dst_ip, iface, src_ip);
	return getIface(src_ip, iface);
}

void NetIface::getRoutingInfo(QString dst_ip, QByteArray& iface, QByteArray& src_ip)
{
	QStringList args;
	QProcess proc;
	
	args << "route" << "get" << dst_ip;
	
	proc.start("/sbin/ip", args, QIODevice::ReadOnly);
	proc.waitForFinished();
	
	if(proc.exitCode())
		throw Exception(QString("/sbin/ip failed, exit code ") + proc.exitCode());
	
	QRegExp regexp("dev ([^ ]+)");
	QByteArray data = proc.readAllStandardOutput();
	
	int in = regexp.indexIn(data);
	
	if(in < 0)
		throw Exception("/sbin/ip parsing failed #1");
	
	QRegExp regip("src ([^ ]+)");
	in = regip.indexIn(data);
	
	if(in < 0)
		throw Exception("/sbin/ip parsing failed #2");
	
	iface = regexp.cap(1).toLatin1();
	src_ip = regip.cap(1).toLatin1();
}

void NetIface::initLibnet()
{
	char errbuf[LIBNET_ERRBUF_SIZE];
	
	if(m_libnet)
		libnet_destroy(m_libnet);
	
	m_libnet = libnet_init(LIBNET_LINK, m_interface.data(), errbuf);
	
	if(!m_libnet)
		throw Exception(QString("libnet_init() failed: ")+errbuf);
	
	libnet_ether_addr* data;
	data = libnet_get_hwaddr(m_libnet);
	if(!data)
		throw Exception("libnet_get_hwaddr() failed");
	
	memcpy(&m_ourmac, data, 6);
}

void NetIface::arping(QByteArray dst)
{
	libnet_ptag_t arp = 0, eth = 0;
	
	initLibnet();
	
	qint32 dest = libnet_name2addr4(m_libnet, dst.data(), LIBNET_RESOLVE);
	
	u_int8_t ethnull[6], ethfull[6];
	memset(ethnull, 0, 6);
	
	arp = libnet_build_arp(ARPHRD_ETHER, ETHERTYPE_IP, 6 /* MAC len */, 4 /* IPv4 len */, ARPOP_REQUEST,
				m_ourmac, (u_int8_t*) &m_source, ethnull,
				(u_int8_t*) &dest, 0, 0, m_libnet, arp);
	
	if(arp == -1)
		throw Exception("libnet_build_arp() failed");
	
	memset(ethfull, 0xff, 6);
	
	eth = libnet_build_ethernet(ethfull, m_ourmac, ETHERTYPE_ARP, NULL, 0, m_libnet, eth);
	
	if(eth == -1)
		throw Exception("libnet_build_ethernet() failed");
	
	if(libnet_write(m_libnet) == -1)
		throw Exception("libnet_write() failed");
}

void NetIface::wake(QByteArray mac, QByteArray password)
{
	libnet_ptag_t eth = 0;
	u_int8_t data[18*6];
	int nums[6];
	int bytes = 17*6;
	
	initLibnet();
	
	sscanf(mac.data(), "%x:%x:%x:%x:%x:%x", &nums[0], &nums[1], &nums[2], &nums[3], &nums[4], &nums[5]);
	for(int i=0;i<6;i++)
		data[6+i] = nums[i] & 0xff;
	
	memset(data, 0xff, 6);
	for(int i=2;i<17;i++)
		memcpy(&data[i*6], &data[6], 6);
	
	if(!password.isEmpty())
	{
		if(password.size() > 6)
			password.resize(6);
		bytes += password.size();
		memcpy(&data[17*6], password.data(), 6);
	}
	
	eth = libnet_build_ethernet(&data[6], m_ourmac, ETHERTYPE_WOL, data, bytes, m_libnet, eth);
	
	if(eth == -1)
		throw Exception("libnet_build_ethernet() failed");
	
	if(libnet_write(m_libnet) == -1)
		throw Exception("libnet_write() failed");
}
