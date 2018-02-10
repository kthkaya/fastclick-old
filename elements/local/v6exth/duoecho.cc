#include <click/config.h>
#include <click/confparse.hh>
#include <click/args.hh>
#include <clicknet/ip6.h>
#include <click/error.hh>
#include "duoecho.hh"
CLICK_DECLS

DuoEcho::DuoEcho(){}
DuoEcho::~DuoEcho(){}

Packet*
DuoEcho::oneToOne(Packet *p){
	click_chatter("---------------From port 0 -> To port 0--------------");
	click_ip6 *ip6h = (click_ip6 *)(p->data()+14);
	click_tcp *tcph = (click_tcp *)(p->data()+54);
	IP6FlowID flowId(ip6h->ip6_src,tcph->th_sport,ip6h->ip6_dst,tcph->th_dport);
	click_chatter("Constructed flow id: %s",flowId.unparse());
	int *result = _transMap.find(flowId);
	if (result){
		click_chatter("Mapping found");
	}else{
		int mSp = 5555;
		click_chatter("Mapping not found. Inserting int 5555");
		_transMap.insert(flowId,&mSp);
	}
	//click_chatter("oneToOne");
	Packet *q = p->clone();
	return q;
}

Packet*
DuoEcho::twoToTwo(Packet *p){
	click_chatter("twoToTwo");
	WritablePacket *wp = p->uniqueify();
	click_ip6_frag *ip6_frag = 	reinterpret_cast<click_ip6_frag *>(wp->data()+14+40);
	click_chatter("ip6f_nxt = %d",ip6_frag->ip6f_nxt);
	click_chatter("ip6f reserved = %d",ip6_frag->ip6f_reserved);
	click_chatter("ip6f offlg= %d",ip6_frag->ip6f_offlg);
	click_chatter("ip6f idnet= %d",ip6_frag->ip6f_ident);

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
