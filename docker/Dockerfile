FROM ubuntu:latest

######################
#Initial Preparations#
######################

RUN echo "Initial Preparations ..."

RUN apt-get update && apt-get install -y \
	build-essential automake git gcc tar wget python-pip libcap-ng-dev libnuma-dev \
	gawk pciutils linux-headers-$(uname -a | awk '{print $3}') nano kmod apt-utils \
	iproute2 zlib1g-dev
RUN pip install -U pip six

#################
#Installing DPDK#
#################
RUN echo "Installing DPDK ..."

ENV INSTALL_PATH "/usr/src"
ENV RTE_SDK "$INSTALL_PATH/dpdk-17.11" 
ENV RTE_TARGET "x86_64-native-linuxapp-gcc" 
ENV DPDK_BUILD "$RTE_DIR/$RTE_TARGET"

RUN cd $INSTALL_PATH; \
wget http://fast.dpdk.org/rel/dpdk-17.11.tar.xz; \
tar xf dpdk-17.11.tar.xz; \
rm dpdk-17.11.tar.xz; \
cd $RTE_SDK; \
make install -j4 T=$RTE_TARGET DESTDIR=install

######################
#Installing fastclick#
######################
RUN echo "Installing fastclick ..."

ENV CLICK_DIR "$INSTALL_PATH/fastclick"
ENV GIT_URL "https://github.com/kthkaya/fastclick"

RUN cd $INSTALL_PATH; \
git clone $GIT_URL; \
cd $CLICK_DIR; \
./build_click.sh

######
#Done#
######
RUN echo "Done."
CMD ["/bin/bash"]


