qemu-system-x86_64 \
    -enable-kvm -m 2048 \
    -smp 2, -cpu host \
    -vga qxl -spice port=5931,disable-ticketing \
    -drive file="/var/lib/libvirt/images/NF.qcow2",if=virtio,aio=threads \
    -netdev tap,id=tapnet0,script=/etc/libvirt/qemu/networks/ifscripts/qemu-ifup,downscript=/etc/libvirt/qemu/networks/ifscripts/qemu-ifdown \
    -device virtio-net-pci,mac=00:00:11:00:00:02,netdev=tapnet0 \
    -chardev socket,id=char1,path=/usr/local/var/run/openvswitch/NF-eth0 \
    -netdev type=vhost-user,id=mynet1,chardev=char1,vhostforce \
    -device virtio-net-pci,mac=00:00:00:00:00:02,netdev=mynet1 \
    -chardev socket,id=char2,path=/usr/local/var/run/openvswitch/NF-eth1 \
    -netdev type=vhost-user,id=mynet2,chardev=char2,vhostforce \
    -device virtio-net-pci,mac=00:00:00:00:00:03,netdev=mynet2 \
    -object memory-backend-file,id=mem,size=2048M,mem-path=/dev/hugepages,share=on \
    -numa node,memdev=mem -mem-prealloc \
    -debugcon file:debug.log -global isa-debugcon.iobase=0x402

