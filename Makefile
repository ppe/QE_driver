CC = qdos-gcc
CFLAGS = -nostartfiles -fomit-frame-pointer
OBJS = heap.o clock-arch.o timer.o chan_ops.o debug.o dhcpc.o resolv.o socket.o w5300.o qedrv.o
DHCPC_OBJS = heap.o clock-arch.o timer.o dhcpc.o dhcpc_main.o socket.o w5300.o
RM = /bin/rm
DRIVER_BIN = qedrv_bin
DHCPC_BIN = dhcpc_exe
SRCS = heap.c clock-arch.c timer.c chan_ops.c debug.c dhcpc.c dhcpc_main.c resolv.c socket.c qedrv.c w5300.c
$(DRIVER_BIN): $(OBJS)
	$(CC) -o $(DRIVER_BIN) -Wl,-ms -Wl,-screspr.o $(OBJS) -lgcc
$(DHCPC_BIN): $(DHCPC_OBJS)
	$(CC) -o $(DHCPC_BIN) -Wl,-ms $(DHCPC_OBJS) -lgcc
.PHONY: clean
clean:
	$(RM) -f *.o *.s *.MAP $(DRIVER_BIN)

depend: .depend

.depend: $(SRCS)
	$(RM) -f ./.depend
	$(CC) $(CFLAGS) -MM $^ > ./.depend;

include .depend
