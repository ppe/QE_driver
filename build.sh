#! /bin/sh

docker run --platform linux/amd64 --rm -it -v $PWD:/qdos/QE_driver xora/qdos-devel-a6toa5 /bin/bash -c "cd QE_driver; make"

