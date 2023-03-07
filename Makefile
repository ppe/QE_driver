CC = qdos-gcc
ZIP = qlzip
# Prevent A6 to be used by gcc since it's special to QDOS
OMAN_BINARIES = $(DRIVER_BIN) $(DHCPC_EXE) $(DHCP_EXT) $(TFTP_EXE)
OMAN_ZIP = oman_bin.zip
CFLAGS = -nostartfiles -fomit-frame-pointer -ffixed-a6
DHCP_COMMON_OBJS = debug.o heap.o clock-arch.o timer.o dhcpc.o socket.o w5300-access.o w5300-ops.o
DRIVER_OBJS = chan_ops.o resolv.o qedrv.o qe_access.s $(DHCP_COMMON_OBJS)
DHCPC_OBJS =  dhcpc_main.o $(DHCP_COMMON_OBJS)
DHCPEXT_OBJS = dhcpext.o dhcpbas.s $(DHCP_COMMON_OBJS)
TFTP_OBJS = tftp.o timer.o clock-arch.o socket.o w5300.o debug.o heap.o w5300-access.o w5300-ops.o
RM = /bin/rm
DRIVER_BIN = qedrv_bin
DHCPC_EXE = dhcpc_exe
DHCP_EXT = dhcp_cde
TFTP_EXE = tftp
SRCS = dhcpext.c heap.c clock-arch.c timer.c chan_ops.c debug.c dhcpc.c dhcpc_main.c resolv.c socket.c qedrv.c w5300.c tftp.c w5300-access.c w5300-ops.c
all: $(OMAN_BINARIES)
$(DRIVER_BIN): $(DRIVER_OBJS)
	$(CC) -o $(DRIVER_BIN) -Wl,-ms -Wl,-screspr.o $(DRIVER_OBJS) -lgcc
	@grep "Undefined Symbol:" $(addsuffix .MAP,$@) || true
# patch the binary with dataspace size so it can be LRESPR:d
	@printf "+%d" 0x`tail -c 4 $@ | xxd -l 32 -p|sed 's/^0*//'` | xargs -I X truncate -s X $@
$(DHCPC_EXE): $(DHCPC_OBJS)
	$(CC) -o $(DHCPC_EXE) -Wl,-ms $(DHCPC_OBJS) -lgcc
	@grep "Undefined Symbol:" $(addsuffix .MAP,$@) || true
$(DHCP_EXT): $(DHCPEXT_OBJS)
	$(CC) -o $(DHCP_EXT) -Wl,-ms -Wl,-screspr.o $(DHCPEXT_OBJS) -lgcc
	@grep "Undefined Symbol:" $(addsuffix .MAP,$@) || true
# patch the binary with dataspace size so it can be LRESPR:d
	@printf "+%d" 0x`tail -c 4 $@ | xxd -l 32 -p|sed 's/^0*//'` | xargs -I X truncate -s X $@
$(TFTP_EXE): $(TFTP_OBJS)
	$(CC) -o $(TFTP_EXE) -Wl,-ms $(TFTP_OBJS) -lgcc
	@grep "Undefined Symbol:" $(addsuffix .MAP,$@) || true
$(OMAN_ZIP): $(OMAN_BINARIES)
	$(ZIP) -q -u oman_bin.zip $(OMAN_BINARIES)
.PHONY: clean
clean:
	$(RM) -f *.o *.MAP $(OMAN_BINARIES)

depend: .depend

.depend: $(SRCS)
	$(RM) -f ./.depend
	$(CC) $(CFLAGS) -MM $^ > ./.depend;

include .depend
