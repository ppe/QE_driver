CC = qdos-gcc
CFLAGS = -nostartfiles -fomit-frame-pointer -O2
OBJS = heap.o clock-arch.o timer.o chan_ops.o debug.o dhcpc.o resolv.o socket.o w5300.o qedrv.o
RM = /bin/rm
DRIVER_BIN = qedrv_bin
SRCS = heap.c clock-arch.c timer.c chan_ops.c debug.c dhcpc.c resolv.c socket.c qedrv.c w5300.c
$(DRIVER_BIN): $(OBJS)
	$(CC) -o $(DRIVER_BIN) -Wl,-ms -Wl,-screspr.o $(OBJS) -lgcc
.PHONY: clean
clean:
	$(RM) -f *.o *.s *.MAP $(DRIVER_BIN)

depend: .depend

.depend: $(SRCS)
	$(RM) -f ./.depend
	$(CC) $(CFLAGS) -MM $^ > ./.depend;

include .depend
