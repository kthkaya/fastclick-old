DECHO  :: DuoEcho();
FromDPDKDevice(0) -> Strip(14) -> DECHO -> EtherEncap(0x0800, 1:1:1:1:1:1, 2:2:2:2:2:2) -> ToDPDKDevice(1);
FromDPDKDevice(1) -> [1]DECHO[1] -> ToDPDKDevice(0);

