// enp0s3 interface - Inbound and Outbound
//------------------------------------------------------------
ENP0S3_IN   ::  FromDPDKDevice(0, PROMISC true);
ENP0S3_OUT  ::  ToDPDKDevice(0);

// enp0s8 interface - Inbound and Outbound
//------------------------------------------------------------
ENP0S8_IN  ::  FromDPDKDevice(1, PROMISC true);
ENP0S8_OUT ::  ToDPDKDevice(1);

//-------------Program----------------- 
ENP0S3_IN -> ENP0S8_OUT;
ENP0S8_IN -> ENP0S3_OUT;

