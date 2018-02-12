#ifndef CLICK_DUOECHO_HH_
#define CLICK_DUOECHO_HH_
#include <click/batchelement.hh>
//#include <click/hashtable.hh>
#include <click/bighashmap.hh>
#include <click/ip6flowid.hh>
CLICK_DECLS


class DuoEcho : public BatchElement {
public:
	class Mapping;
	//typedef HashContainer<IP6FlowID> Map;
	typedef HashMap<IP6FlowID, Mapping *> Map6;
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
	Packet* oneToOne(Packet *p);
	Packet* twoToTwo(Packet *p);
};


class DuoEcho::Mapping {

public:
	Mapping() CLICK_COLD;
	void initialize(const unsigned short &port) {_port = port;}

protected:
	unsigned short _port;

	friend class DuoEcho;
};




CLICK_ENDDECLS
#endif
