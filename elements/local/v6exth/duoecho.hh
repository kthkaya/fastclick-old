#ifndef CLICK_DUOECHO_HH_
#define CLICK_DUOECHO_HH_
#include <click/batchelement.hh>
//#include <click/hashtable.hh>
#include <click/bighashmap.hh>
#include <click/ip6flowid.hh>
CLICK_DECLS

typedef union {
	IP6Address _v6;
	IPAddress _v4;
} netAddress;

class DuoEcho : public BatchElement {
public:
	class Mapping;
	//typedef HashContainer<IP6FlowID> Map;
	typedef HashMap<IP6FlowID, Mapping *> Map6;
	typedef HashMap<IPFlowID, Mapping *> Map4;
	IPAddress mappedv4Address;
	DuoEcho();
	~DuoEcho();
	const char *class_name() const { return "DuoEcho"; }
	const char *port_count() const { return "2/1-2"; }
	const char *processing() const { return PUSH; }
	int configure(Vector<String> & , ErrorHandler *) CLICK_COLD;
	void push(int port, Packet *p);

#if HAVE_BATCH
	void push_batch(int port, PacketBatch *batch) override;
#endif

private:
	Map6 _transMap;
	Map6 _departingMap;
	Map4 _returnMap;
	unsigned short _nextPort;
	Packet* oneToOne(Packet *p);
	Packet* twoToTwo(Packet *p);
	//void translate(Packet *p, click_ip v4l3h, click_ip6 v6l3h,  bool direction, Mapping *addressAndPort);
	Packet* translate64(Packet *p, const click_ip6 *v6l3h, const click_tcp *l4h, Mapping *addressAndPort);
	Packet* translate46(Packet *p, const click_ip *v4l3h, const click_tcp *l4h, Mapping *addressAndPort);
	Packet* testingPush(Packet *p);
};


class DuoEcho::Mapping {

	union address{
			IP6Address _v6;
			IPAddress _v4;
			address() {memset(this,0,sizeof(address));};
		};

public:
	Mapping() CLICK_COLD;
	void initializeV4(const IPAddress &address, const unsigned short &port) { mappedAddress._v4 = address; _mappedPort = port;}
	void initializeV6(const IP6Address &address, const unsigned short &port) { mappedAddress._v6 = address; _mappedPort = port;}

protected:

	address mappedAddress;

	unsigned short _mappedPort;

	friend class DuoEcho;
};


CLICK_ENDDECLS
#endif
