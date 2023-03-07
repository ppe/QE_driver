#! /bin/sh

docker run --platform linux/amd64 --rm -it -v $PWD:/qdos/QE_driver xora/qdos-devel-a6toa5 /bin/bash -c "cd QE_driver; make"

# Detect the platform (similar to $OSTYPE)
OS="`uname`"
case $OS in
  'Linux')
    OS='Linux'
    ./zip2-lin -Q4 ethernet.zip dhcp_cde dhcpc_exe qedrv_bin tftp
    ;;
  'FreeBSD')
    OS='FreeBSD'
    alias ls='ls -G'
    ;;
  'WindowsNT')
    OS='Windows'
    ;;
  'Darwin')
    OS='Mac'
    ;;
  'SunOS')
    OS='Solaris'
    ;;
  'AIX') ;;
  *) ;;
esac
