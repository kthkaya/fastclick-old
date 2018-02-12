#include <click/config.h>
#include <click/confparse.hh>
#include <click/args.hh>
#include <clicknet/ip6.h>
#include <clicknet/tcp.h>
#include <click/error.hh>
#include <click/straccum.hh>

#include "duoecho.hh"
CLICK_DECLS

DuoEcho::DuoEcho():_transMap(0){}
DuoEcho::Mapping::Mapping():_port(0), _v6Address(0){}
DuoEcho::~DuoEcho(){}

Packet*
DuoEcho::oneToOne(Packet *p){
	click_chatter("---------------From port 0 -> To port 0--------------");
	//const click_ip6 *ip6h = (click_ip6*) p->ip_header();
    StringAccum sa;


	click_ip6 *ip6h = (click_ip6 *)(p->data()+14);
	click_tcp *tcph = (click_tcp *)(p->data()+54);
	click_chatter("IP version is %x",ntohs(ip6h->ip6_ctlun.ip6_un2_vfc));

	IP6Address ip6_src = IP6Address(ip6h->ip6_src);
	IP6Address ip6_dst = IP6Address(ip6h->ip6_dst);
	String src= ip6_src.unparse_expanded();
	String dst= ip6_dst.unparse_expanded();

	uint16_t sport = tcph->th_sport;
	uint16_t dport = tcph->th_dport;
	click_chatter("Passing SA: %s, SP: %d, DA: %s, DP: %d",src.c_str(),ntohs(sport),dst.c_str(),ntohs(dport));

	const IP6FlowID flowId(ip6_src,sport,ip6_dst,dport);
	click_chatter("Constructed flow id: %s",flowId.unparse().c_str());
	Mapping *result = _transMap.find(flowId);
	if (result){
		click_chatter("Mapping found, mapped int is %d",result->_port);
	}else{
		click_chatter("Mapping not found. Inserting int 555");
		Mapping *newMap = new Mapping;
		click_chatter("Mapping created");
		newMap->initialize(ip6_src,5555);
		click_chatter("Mapping initialized");
		_transMap.insert(flowId,newMap);
	}
	//click_chatter("oneToOne");
	Packet *q = p->clone();
	return q;
}

Packet*
DuoEcho::twoToTwo(Packet *p){
	//click_chatter("twoToTwo");
	WritablePacket *wp = p->uniqueify();
	click_ip6_frag *ip6_frag = 	reinterpret_cast<click_ip6_frag *>(wp->data()+14+40);
	/*
	click_chatter("ip6f_nxt = %d",ip6_frag->ip6f_nxt);
	click_chatter("ip6f reserved = %d",ip6_frag->ip6f_reserved);
	click_chatter("ip6f offlg= %d",ip6_frag->ip6f_offlg);
	click_chatter("ip6f idnet= %d",ip6_frag->ip6f_ident);
	*/
	return wp;
}

int
DuoEcho::configure(Vector<String> &conf, ErrorHandler *errh){
	return 0;
}

void DuoEcho::push(int port, Packet *p){
	if (port == 0){
		p = oneToOne(p);
		if (p)
			output(0).push(p);
	} else {
		p = twoToTwo(p);
		if (p)
			output(1).push(p);
	}
}

#if HAVE_BATCH
void
DuoEcho::push_batch(int port, PacketBatch *batch)
{
	if (port == 0) {
		EXECUTE_FOR_EACH_PACKET_DROPPABLE(oneToOne,batch,[](Packet*p){});
		if (batch)
			output(0).push_batch(batch);
	} else {
		EXECUTE_FOR_EACH_PACKET_DROPPABLE(twoToTwo,batch,[](Packet*p){});
		if (batch)
			output(1).push_batch(batch);
	}
}
#endif


CLICK_ENDDECLS
EXPORT_ELEMENT(DuoEcho)
