c :: Classifier(12/86DD, 12/0800, -);

FromDPDKDevice(0, PROMISC true) -> c;

c [0] 
  -> Strip(14)
  -> MarkIPHeader
  -> IP6Print(v6, NBYTES 512, CONTENTS true) 
  -> ck :: CheckIP6Header;

ck [0]
  -> Discard;
ck [1]
  -> IP6Print(ip6hl)
  -> Discard;

c [1] 
  -> Strip(14)
  -> MarkIPHeader
  // -> IPPrint(v4) 
  -> Discard;

c [2]
  -> Discard;
