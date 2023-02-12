CC = qdos-gcc
CFLAGS = -nostartfiles -fomit-frame-pointer
DHCP_COMMON_OBJS = debug.o heap.o clock-arch.o timer.o dhcpc.o socket.o w5300.o
DRIVER_OBJS = chan_ops.o resolv.o qedrv.o $(DHCP_COMMON_OBJS)
DHCPC_OBJS =  dhcpc_main.o $(DHCP_COMMON_OBJS)
DHCPEXT_OBJS = dhcpext.o dhcpbas.s $(DHCP_COMMON_OBJS)
TFTP_OBJS = tftp.o timer.o clock-arch.o  socket.o w5300.o debug.o heap.o
RM = /bin/rm
DRIVER_BIN = qedrv_bin
DHCPC_EXE = dhcpc_exe
DHCP_EXT = dhcp_cde
TFTP_EXE = tftp
SRCS = dhcpext.c heap.c clock-arch.c timer.c chan_ops.c debug.c dhcpc.c dhcpc_main.c resolv.c socket.c qedrv.c w5300.c tftp.c
all: $(DRIVER_BIN) $(DHCPC_EXE) $(DHCP_EXT)
$(DRIVER_BIN): $(DRIVER_OBJS)
	$(CC) -o $(DRIVER_BIN) -Wl,-ms -Wl,-screspr.o $(DRIVER_OBJS) -lgcc
$(DHCPC_EXE): $(DHCPC_OBJS)
	$(CC) -o $(DHCPC_EXE) -Wl,-ms $(DHCPC_OBJS) -lgcc
$(DHCP_EXT): $(DHCPEXT_OBJS)
	$(CC) -o $(DHCP_EXT) -Wl,-ms -Wl,-screspr.o $(DHCPEXT_OBJS) -lgcc
$(TFTP_EXE): $(TFTP_OBJS)
	$(CC) -o $(TFTP_EXE) -Wl,-ms $(TFTP_OBJS) -lgcc
.PHONY: clean
clean:
	$(RM) -f *.o *.MAP $(DRIVER_BIN) $(DHCPC_EXE) $(DHCP_EXT)

depend: .depend

.depend: $(SRCS)
	$(RM) -f ./.depend
	$(CC) $(CFLAGS) -MM $^ > ./.depend;

include .depend
