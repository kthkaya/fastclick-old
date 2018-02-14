#include <click/config.h>
#include <click/confparse.hh>
#include <click/args.hh>
#include <clicknet/ip6.h>
#include <clicknet/tcp.h>
#include <click/error.hh>
#include <click/straccum.hh>

#include "duoecho.hh"
CLICK_DECLS

DuoEcho::DuoEcho():_transMap(0),_departingMap(0),_returnMap(0),mappedv4Address("192.0.2.1"),_nextPort(1025){}
DuoEcho::Mapping::Mapping():_mappedPort(0), mappedAddress(){}
DuoEcho::~DuoEcho(){}

/*
void DuoEcho::translate(WritablePacket *wp, bool direction, Mapping *addressAndPort){
	// Direction=1 Translate 6 to 4. Direction=0 Translate 4 to 6.
	if (direction) {
		WritablePacket *wp = p->push(20)
		addressAndPort->mappedAddress._v6;
		addressAndPort->_mappedPort;
	} else {

	}

}
 */

Packet*
DuoEcho::testingPush(Packet *p){
	click_ip6 ip6h;
	WritablePacket *wp= p->push(20);
	/*
	wp->t
	memmove(wp->data(),wp->data()+20,14);
	memcpy(wp->data()+14,ip6h,40);
	 */
}


Packet*
DuoEcho::translate64(Packet *p, const click_ip6 *v6l3h, const click_tcp *l4h, Mapping *addressAndPort){

	click_ip *ip;
	click_tcp *tcph;
	click_udp *udph;
	WritablePacket *wp = Packet::make(sizeof(*ip) + ntohs(v6l3h->ip6_plen));
	unsigned char *start_of_p= (unsigned char *)(v6l3h+1);
	if (wp==0) {
		click_chatter("can not make packet!");
		assert(0);
	}
	//Initialize
	memset(wp->data(), '\0', wp->length());
	ip = (click_ip *)wp->data();
	tcph = (click_tcp *)(ip+1);
	udph = (click_udp *)(ip+1);

	//set ipv4 header
	ip->ip_v = 4;
	ip->ip_hl =5;
	ip->ip_tos =0;
	ip->ip_len = htons(sizeof(*ip) + ntohs(v6l3h->ip6_plen));

	ip->ip_id = htons(0);
	//need to change
	//ip->ip_id[0]=ip6->ip_flow[1];
	//ip->ip_id[1]=ip6->ip_flow[2];

	//set Don't Fragment flag to true, all other flags to false,
	//set fragement offset: 0
	ip->ip_off = htons(IP_DF);
	//need to deal with fragmentation later

	//we do not change the ttl since the packet has to go through v4 routing table
	ip->ip_ttl = v6l3h->ip6_hlim;

	//set the src and dst address
	ip->ip_src = addressAndPort->mappedAddress._v4;
	ip->ip_dst = IP6Address(v6l3h->ip6_dst).ip4_address();
	//Set the annotation, other elements down the pipeline may need it
	wp->set_dst_ip_anno(ip->ip_dst);

	//copy the actual payload of packet
	memcpy((unsigned char *)tcph, start_of_p, ntohs(v6l3h->ip6_plen));
	//set the tcp header checksum
	//The tcp checksum for ipv4 packet is include the tcp packet, and the 96 bits
	//TCP pseudoheader, which consists of Source Address, Destination Address,
	//1 byte zero, 1 byte PTCL, 2 byte TCP length.

	if (v6l3h->ip6_nxt == 6) //TCP
	{
		ip->ip_p = v6l3h->ip6_nxt;
		//set the ip header checksum
		ip->ip_sum = 0;
		tcph->th_sum = 0;
		tcph->th_sport = htons(addressAndPort->_mappedPort);

		uint16_t tlen = ntohs(v6l3h->ip6_plen);
		uint16_t csum = click_in_cksum((unsigned char *) tcph, tlen);
		tcph->th_sum = click_in_cksum_pseudohdr(csum, ip, tlen);
		ip->ip_sum = click_in_cksum((unsigned char *)ip, sizeof(click_ip));

	}
	else if (v6l3h->ip6_nxt ==17) //UDP
	{
		ip->ip_p = v6l3h->ip6_nxt;

		//set the ip header checksum
		ip->ip_sum=0;
		//udph->uh_sum = 0;
		//udp->uh_sport = addressAndPort->_mappedPort;

		uint16_t tlen = ntohs(v6l3h->ip6_plen);
		uint16_t csum = click_in_cksum((unsigned char *) udph, tlen);
		//udp->uh_sum = click_in_cksum_pseudohdr(csum, ip, tlen);
		ip->ip_sum = click_in_cksum((unsigned char *)ip, sizeof(click_ip));

	}
	else if (v6l3h->ip6_nxt== 58)
	{
		ip->ip_p = 1;
		//icmp 4->6 translation is dealt by caller
	}

	else
	{
		// will deal with other protocols later
	}
	return wp;

}

Packet*
DuoEcho::translate46(Packet *p, const click_ip *v4l3h, const click_tcp *l4h, Mapping *addressAndPort){

	WritablePacket *wp = Packet::make(sizeof(click_ip6)-sizeof(click_ip)+ntohs(v4l3h->ip_len));
	unsigned char *start_of_p= (unsigned char *)(v4l3h+1);
	if (wp==0) {
		click_chatter("can not make packet!");
		assert(0);
	}
	memset(wp->data(), '\0', wp->length());
	click_ip6 *ip6=(click_ip6 *)wp->data();
	click_tcp *tcph = (click_tcp *)(ip6+1);
	click_udp *udph = (click_udp *)(ip6+1);

	//set ipv6 header
	ip6->ip6_flow = 0;	/* must set first: overlaps vfc */
	ip6->ip6_v = 6;
	ip6->ip6_plen = htons(ntohs(v4l3h->ip_len)-sizeof(click_ip));
	ip6->ip6_hlim = v4l3h->ip_ttl + 0x40-0xff;
	//Translate IP addresses
	//Append a WKP prefix to the v4 address and construct the embedded v6 address
	ip6->ip6_src = IP6Address("64:ff9b::"+IPAddress(v4l3h->ip_src).unparse());
	ip6->ip6_dst = addressAndPort->mappedAddress._v6;
	SET_DST_IP6_ANNO(wp,ip6->ip6_dst);

	memcpy((unsigned char *)tcph, start_of_p, ntohs(ip6->ip6_plen));

	if (v4l3h->ip_p == 6) //TCP
	{
		ip6->ip6_nxt = v4l3h->ip_p;
		tcph->th_dport = htons(addressAndPort->_mappedPort);

		uint16_t tlen = ntohs(v4l3h->ip_len);
		uint16_t csum = click_in_cksum((unsigned char *) tcph, tlen);
		tcph->th_sum = 0;
		tcph->th_sum = htons(in6_fast_cksum(&ip6->ip6_src, &ip6->ip6_dst, ip6->ip6_plen, ip6->ip6_nxt, tcph->th_sum, (unsigned char *)tcph, ip6->ip6_plen));

		//tcph->th_sum = click_in_cksum_pseudohdr(csum, v4l3h, tlen);
		//tcph->th_sum = htons(in6_fast_cksum(&ip6->ip6_src, &ip6->ip6_dst, ip6->ip6_plen, ip6->ip6_nxt, tcph->th_sum, start_of_p, ip6->ip6_plen));
	}

	else if (v4l3h->ip_p == 17) //UDP
	{
		ip6->ip6_nxt = v4l3h->ip_p;
		//udph->uh_sum = htons(in6_fast_cksum(&ip6->ip6_src, &ip6->ip6_dst, ip6->ip6_plen, ip6->ip6_nxt, udp->uh_sum, start_of_p, ip6->ip6_plen));
	}

	else if (v4l3h->ip_p == 1)
	{
		ip6->ip6_nxt=0x3a;
		//icmp 6->4 translation is dealt by caller.
	}

	else
	{
		//will deal other protocols later
	}

	return wp;
}


Packet*
DuoEcho::oneToOne(Packet *p){
	click_chatter("---------------From port 0 -> To port 0--------------");
	//const click_ip6 *ip6h = (click_ip6*) p->ip_header();
	click_ip6 *ip6h = (click_ip6 *)(p->data());
	click_tcp *tcph = (click_tcp *)(p->data()+40);
	IP6Address ip6_src = IP6Address(ip6h->ip6_src);
	IP6Address ip6_dst = IP6Address(ip6h->ip6_dst);
	String src= ip6_src.unparse_expanded();
	String dst= ip6_dst.unparse_expanded();
	if (ip6_dst.has_wellKnown_prefix()) {
		click_chatter("Has WKP true");
		click_chatter("Destination v4 mapped %s",ip6_dst.ip4_address().unparse().c_str());

		uint16_t sport = tcph->th_sport;
		uint16_t dport = tcph->th_dport;

		const IP6FlowID departingFlowID(ip6_src,sport,ip6_dst,dport);

		click_chatter("Constructed flow id: %s",departingFlowID.unparse().c_str());
		Mapping *departingMapping = _departingMap.find(departingFlowID);

		if (departingMapping){

			click_chatter("Mapping found, mapped IP is %s and port is %d",departingMapping->mappedAddress._v4.unparse().c_str(), departingMapping->_mappedPort);


		}else{
			click_chatter("Mapping not found. Inserting next port %d", _nextPort);
			//Create mapping for departing traffic
			departingMapping = new Mapping;
			departingMapping->initializeV4(mappedv4Address,_nextPort);
			click_chatter("Mapping initialized");
			_departingMap.insert(departingFlowID,departingMapping);

			//Create mapping for the return traffic
			const IPFlowID returnFlowID(ip6_dst.ip4_address(),dport,mappedv4Address,htons(_nextPort));
			click_chatter("Inserting for return traffic: Flow id: %s",returnFlowID.unparse().c_str());
			Mapping *returnMapping = new Mapping;
			returnMapping->initializeV6(ip6_src,htons(sport));
			_returnMap.insert(returnFlowID,returnMapping);
			_nextPort++;

		}
		return translate64(p,ip6h,tcph,departingMapping);
	}
	//click_chatter("oneToOne");
	return p->clone();
	//return translate64(p,ip6h,tcph,departingMapping);
}

Packet*
DuoEcho::twoToTwo(Packet *p){
	click_chatter("---------------From port 1 -> To port 1--------------");
	//click_chatter("twoToTwo");
	click_ip *iph = (click_ip *)(p->data());
	click_tcp *tcph = (click_tcp *)(p->data()+20);
	//String v4wkpEmbed = "64:ff9b::" + IPAddress(iph->ip_src).unparse();
	//IP6Address *adr = new IP6Address("64:ff9b::" + IPAddress(iph->ip_src).unparse());
	//click_chatter("Did it work? %s",adr->unparse_expanded().c_str());
	const IPFlowID returnFlowID(iph->ip_src,tcph->th_sport,iph->ip_dst,tcph->th_dport);
	click_chatter("Constructed flow id: %s",returnFlowID.unparse().c_str());
	Mapping *returnMapping = _returnMap.find(returnFlowID);
	if (returnMapping){
		click_chatter("Mapping found, mapped IP is %s and port is %d",returnMapping->mappedAddress._v6.unparse().c_str(), returnMapping->_mappedPort);
		return translate46(p,iph,tcph,returnMapping);
	}
	click_chatter("Mapping not found. Killing packet");
	//Drop the packet if no existing maping is found
	p->kill();
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
