qemu-system-x86_64 \
    -enable-kvm -m 1024 \
    -smp cpus=1, -cpu host \
    -vga qxl -spice port=5930,disable-ticketing \
    -drive file="/var/lib/libvirt/images/NF-clone.qcow2",if=virtio,aio=threads \
    -chardev socket,id=char1,path=/usr/local/var/run/openvswitch/SRC \
    -netdev tap,id=tapnet0,script=/etc/libvirt/qemu/networks/ifscripts/qemu-ifup,downscript=/etc/libvirt/qemu/networks/ifscripts/qemu-ifdown \
    -netdev type=vhost-user,id=mynet1,chardev=char1,vhostforce \
    -device virtio-net-pci,mac=00:00:11:00:00:01,netdev=tapnet0 \
    -device virtio-net-pci,mac=00:00:00:00:00:01,netdev=mynet1 \
    -object memory-backend-file,id=mem,size=1024M,mem-path=/dev/hugepages,share=on \
    -numa node,memdev=mem -mem-prealloc \
    -debugcon file:debug.log -global isa-debugcon.iobase=0x402

