//------------Declarations-----------

//+++++DPDK0 Interface(Inside)+++++
//DPDK0_IN :: FromDPDKDevice(0, PROMISC true);
DPDK0_OUT ::  ToDPDKDevice(0);

//+++++DPDK1 Interface(Outside)+++++
DPDK1_IN :: FromDPDKDevice(1, PROMISC true); 
//DPDK1_OUT :: ToDPDKDevice(1);


//--------Program Start----------
InfiniteSource(LENGTH 64, LIMIT 15000, STOP true)
	-> NumberPacket(OFFSET 0)
//	-> replay :: ReplayUnqueue(STOP 1, ACTIVE true, QUICK_CLONE 1)
//	-> UDPIP6Encap(2001:2001:2001:2001::1, 1234, 2001:2001:2001:2001::2, 1234)
        -> UDPIP6Encap(2001:2001:2001:2001::1, 1234, 64:ff9b::192.168.2.5, 1234)
	-> EtherEncap(0x86DD, 00:04:23:D0:93:63, 00:17:cb:0d:f8:db)
	-> record :: RecordTimestamp()
	-> DPDK0_OUT;

DPDK1_IN
	-> CheckNumberPacket(OFFSET 42, COUNT 15000)
	-> diff :: TimestampDiff(OFFSET 42, RECORDER record)
	-> counter :: AverageCounter()
	-> Discard;
//--------Program End----------
DriverManager(wait,read diff.average, read counter.count, read counter.rate)
