//Author: Huseyin Kayahan
//Partial configuration inherited from my own work in SDN course
//eth1 is considered to be the outside interface that the NAPT is performed

// eth0 interface - Inbound and Outbound
//------------------------------------------------------------
E0_IN   ::  FromDPDKDevice(0, PROMISC true);
E0_OUT  ::  ToDPDKDevice(0);

// eth1 interface - Inbound and Outbound
//------------------------------------------------------------
E1_IN  ::  FromDPDKDevice(1, PROMISC true);
E1_OUT ::  ToDPDKDevice(1);

// Address Info container elements for the Interfaces
//------------------------------------------------------------
AddressInfo(E0_ADD 10.0.0.1/24 00:00:46:aa:bb:01);
AddressInfo(E1_ADD 20.0.0.1/24 00:00:46:aa:bb:02);

// Arp Querier and Responder elements for E0 & E1 interfaces
//------------------------------------------------------------------------
AQ_E0  ::   ARPQuerier(E0_ADD) -> E0_OUT;
AQ_E1  ::   ARPQuerier(E1_ADD) -> E1_OUT;

// Proxy-arp for P2P link behavior
AR_E0  ::   ARPResponder(0.0.0.0/0 00:00:46:aa:bb:01) -> E0_OUT;
AR_E1  ::   ARPResponder(0.0.0.0/0 00:00:46:aa:bb:02) -> E1_OUT;

// Incoming frame classifiers for both the E0 and E1 interfaces
//--------------------------------------------------------------------------
E0_FRAME_CLS  :: Classifier(12/0806 20/0001, 12/0806 20/0002, 12/0800, -)
E1_FRAME_CLS  :: Classifier(12/0806 20/0001, 12/0806 20/0002, 12/0800, -)

// IP packet classifiers for ICMP packets destined to E0 and E1 interface IPs
//--------------------------------------------------------------------------
E0_PKT_CLS  :: IPClassifier(dst host 10.0.0.1 and icmp type echo, icmp type echo, -)
E1_PKT_CLS  :: IPClassifier(dst host 20.0.0.1 and icmp type echo, icmp type echo-reply, -)

//------------------------------------------------------------
//Establish routing table for directly connected routes
RIB  :: StaticIPLookup(10.0.0.0/8 1,
                       10.0.0.255/32 1,
                       10.0.0.1/32 1,
                       20.0.0.0/8 0,
                       20.0.0.255/32 0,
                       20.0.0.1/32 0);
RIB[1] -> AQ_E0;
RIB[0] -> AQ_E1;


//------------------------------------------------------------

PING_RW :: ICMPPingRewriter(pattern 20.0.0.1 - 0-65535# 0 1, drop)
IP_RW   :: IPRewriter(pattern 20.0.0.1 1024-65535# - - 0 1, drop)

//----------Processing frames arriving at E0--------------------
E0_IN

        -> E0_FRAME_CLS
        -> AR_E0;

//---If it is an ARP response, it might be a response to a previous ARP query of AQ_E0, so feed it back                                                    to it on port 1
        E0_FRAME_CLS[1]
                -> [1]AQ_E0;

//---If it is an IP packet, strip the frame header and check IP header for errors
        E0_FRAME_CLS[2]
                -> Strip(14)
                -> CheckIPHeader
                -> DecIPTTL
                -> E0_PKT_CLS
                                -> ICMPPingResponder
                                -> AQ_E0;

//----------If is an echo request to an outside host, rewrite source IP then route
                   E0_PKT_CLS[1]
                                -> PING_RW
                                -> RIB;

//----------Rest of the packets to outside, rewrite source IP then route
                   E0_PKT_CLS[2] 
                                -> IP_RW
                                -> RIB;


//---Discard non-IP frames
        E0_FRAME_CLS[3]
                -> Discard;

//----------Processing frames arriving at E1--------------------
E1_IN

        -> E1_FRAME_CLS
        -> AR_E1;

//---If it is an ARP response, it might be a response to a previous ARP query of AQ_E1, so feed it back                                                    to it on port 1
        E1_FRAME_CLS[1]
                -> [1]AQ_E1;

//---If it is an IP packet, strip the frame header and check IP header for errors
        E1_FRAME_CLS[2]
                -> Strip(14)
                -> CheckIPHeader
                -> DecIPTTL
                -> E1_PKT_CLS
                                -> ICMPPingResponder
                                -> AQ_E1;

//----------Check if the echo replies belong to an entry in the rewrite (NAPT state) table
                   E1_PKT_CLS[1]
                            -> [1] PING_RW [1]
                                -> RIB;

//----------Rest of the IP packets, rewrite the original source back then route them according to RIB
                   E1_PKT_CLS[2]
                            -> [1] IP_RW [1]
                                -> RIB;

//---Discard non-IP frames
        E1_FRAME_CLS[3]
                -> Discard;

