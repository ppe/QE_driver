#! /bin/sh

docker run --platform linux/amd64 --rm -it -v $PWD:/qdos/QE_driver xora/qdos-devel-a6toa5 /bin/bash -c "cd QE_driver; make"
./zip2-lin -Q4 ethernet.zip dhcp_cde dhcpc_exe qedrv_bin tftp
