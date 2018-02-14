//------------Declarations-----------
//+++++Commons+++++
DECHO  :: DuoEcho();

//+++++DPDK0 Interface(Inside)+++++
DPDK0_IN :: FromDPDKDevice(0);
DPDK0_OUT ::  ToDPDKDevice(0);

DPDK0_FRAME_CLS  :: Classifier(12/86dd 54/87, 12/86dd 54/88, 12/86dd, -);

DPDK0_NDA :: IP6NDAdvertiser(::/0 00:00:11:00:00:1);
DPDK0_NDS :: IP6NDSolicitor(3ffe:1ce1:2::1, 00:00:11:00:00:1);
 

//+++++DPDK1 Interface(Outside)+++++
DPDK1_IN :: FromDPDKDevice(1); 
DPDK1_OUT :: ToDPDKDevice(1);

AddressInfo(DPDK1_ADD 192.0.2.1/24 00:00:11:00:00:2);
AQ_DPDK1  ::   ARPQuerier(DPDK1_ADD);
AR_DPDK1  ::   ARPResponder(DPDK1_ADD);

DPDK1_FRAME_CLS  :: Classifier(12/0806 20/0001, 12/0806 20/0002, 12/0800, -)

//--------Program Start----------

DPDK0_IN
	-> DPDK0_FRAME_CLS
	-> DPDK0_NDA
	-> DPDK0_OUT;

	DPDK0_FRAME_CLS[1]
		->[1]DPDK0_NDS;

	DPDK0_FRAME_CLS[2]
		-> Strip(14)
		-> DECHO
		-> AQ_DPDK1
		-> DPDK1_OUT;
		
	DPDK0_FRAME_CLS[3]
		-> DPDK1_OUT;

DPDK1_IN
	-> DPDK1_FRAME_CLS
	-> AR_DPDK1
	-> DPDK1_OUT;
	
	DPDK1_FRAME_CLS[1]
		-> [1]AQ_DPDK1;
		
	DPDK1_FRAME_CLS[2]
		-> Strip(14)
		-> [1]DECHO[1]
		-> DPDK0_NDS
		-> DPDK0_OUT;

	DPDK1_FRAME_CLS[3]
		-> DPDK0_OUT;
