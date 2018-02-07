// External interface - Inbound and Outbound
//------------------------------------------------------------
EXT_IN  ::  FromDevice(NF-eth1, METHOD LINUX);
EXT_OUT ::  Queue(200) -> ToDevice(NF-eth1);

// Internal interface - Inbound and Outbound
//------------------------------------------------------------
INT_IN  ::  FromDevice(NF-eth0, METHOD LINUX);
INT_OUT ::  Queue(200) -> ToDevice(NF-eth0);


DECHO  :: DuoEcho();
INT_IN -> DECHO -> EXT_OUT;
EXT_IN -> [1]DECHO[1] -> INT_OUT;

//INT_IN -> EXT_OUT;
//EXT_IN -> INT_OUT;



