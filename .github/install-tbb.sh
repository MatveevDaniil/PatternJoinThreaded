#!/bin/bash
set -eu

curl -fLsS --retry 3 \
  https://github.com/uxlfoundation/oneTBB/archive/refs/tags/v2022.0.0.tar.gz \
  -o /tmp/tbb.tar.gz
echo 'e8e89c9c345415b17b30a2db3095ba9d47647611662073f7fbf54ad48b7f3c2a  /tmp/tbb.tar.gz' \
  | sha256sum -c -
tar -xzf /tmp/tbb.tar.gz -C /tmp
cmake -S /tmp/oneTBB-2022.0.0 -B /tmp/tbb-build \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
  -DTBB_TEST=OFF -DTBB_STRICT=OFF -DTBBMALLOC_BUILD=OFF
cmake --build /tmp/tbb-build --parallel 2
cmake --install /tmp/tbb-build
ldconfig
