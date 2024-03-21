#!/bin/bash
set -x

docker run --rm -it --privileged -v $(pwd):/build -u $UID -w /build -e HOME=/tmp espressif/idf:release-v5.2 idf.py build

