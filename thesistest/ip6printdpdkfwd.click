c :: Classifier(12/86DD, 12/0800, -);
OUT_1 :: ToDPDKDevice(1);
OUT_0 :: ToDPDKDevice(0);

FromDPDKDevice(0, PROMISC true) -> c;
FromDPDKDevice(1, PROMISC true) -> OUT_0; 


c [0] 
  -> Strip(14)
  -> MarkIPHeader
  -> IP6Print(v6, NBYTES 512, CONTENTS true) 
  -> ck :: CheckIP6Header;

ck [0]
  -> Discard;
ck [1]
  -> IP6Print(ip6hl)
  -> OUT_1;

c [1] 
  -> Strip(14)
  -> MarkIPHeader
  // -> IPPrint(v4) 
  -> OUT_1;

c [2]
  -> OUT_1;
