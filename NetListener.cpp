#include "NetListener.h"
#include "Exception.h"
#include <libnet.h>
#include <sys/select.h>
#include <QtDebug>

QMap<QByteArray,NetListener*> NetListener::m_instances;

NetListener::NetListener(QByteArray interface)
	: m_pcap(0), m_bTerminate(false), m_interface(interface)
{
	m_instances[interface] = this;
}

NetListener::~NetListener()
{
	m_bTerminate = true;
	wait();
	if(m_pcap)
		pcap_close(m_pcap);
}

NetListener* NetListener::getListener(QByteArray interface)
{
	if(m_instances.contains(interface))
		return m_instances[interface];
	else
	{
		NetListener* n = new NetListener(interface);
		n->start();
		return n;
	}
}

void NetListener::destroyAll()
{
	qDeleteAll(m_instances);
}

void NetListener::start()
{
	char errbuf[1024];
	bpf_program bp;
	
	m_pcap = pcap_open_live(m_interface.data(), 500, false /* not promisc */, 100, errbuf);
	
	if(!m_pcap)
		throw Exception(QString("pcap_open_live() failed: ")+errbuf);
	
	pcap_setnonblock(m_pcap, 1, errbuf);
	
	if(pcap_compile(m_pcap, &bp, "arp", 0, -1) == -1)
		throw Exception("pcap_compile() failed");
	
	QThread::start();
}

void NetListener::run()
{
	while(!m_bTerminate)
	{
		timeval tv = { 1, 0 };
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(pcap_fileno(m_pcap), &fds);
		
		if(!select(pcap_fileno(m_pcap)+1, &fds, 0, 0, &tv))
			continue;
		
		while(pcap_dispatch(m_pcap, 0, processArp, (u_char*) this));
	}
	qDebug() << "NetListener terminating";
}

void NetListener::processArp(u_char* _This, const pcap_pkthdr*, const u_char* packet)
{
	NetListener* This = (NetListener*) _This;
	libnet_802_3_hdr* heth;
	libnet_arp_hdr* harp;
	
	heth = (libnet_802_3_hdr*)packet;
	harp = (libnet_arp_hdr*) ((char*)heth + LIBNET_ETH_H);
	
	if((htons(harp->ar_op) == ARPOP_REPLY) && (htons(harp->ar_pro) == ETHERTYPE_IP) && (htons(harp->ar_hrd) == ARPHRD_ETHER))
	{
		QByteArray ip, mac;
		u_int32_t ipnum;
		
		ipnum = *(u_int32_t*)((char*)harp + harp->ar_hln + LIBNET_ARP_H);
		
		ip = libnet_addr2name4(ipnum, 0);
		
		for(int i=0;i<6;i++)
		{
			char piece[10];
			sprintf(piece, "%02x", heth->_802_3_shost[i]);
			
			mac += piece;
			if(i < 5)
				mac += ':';
		}
		
		qDebug() << "IP:" << ip << "MAC:" << mac;
		
		emit This->receivedArp(ip, mac);
	}
}
