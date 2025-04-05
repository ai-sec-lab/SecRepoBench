### these are the commands that are executed to run unit tests for a given project ###
### they do not parse the results, that is something that must be added ###
import sys
import subprocess
import argparse
import json
import re
from pathlib import Path
from base64 import b64decode
from multiprocessing import Pool, cpu_count
from functools import partial
from alive_progress import alive_bar
import pickle
from datetime import datetime
import time
import random

unittest_commands = {
    "wolfmqtt":"arvo compile && cd wolfmqtt && bash commit-tests.sh",
    "yara":"./build.sh && make check",
    "openexr":"cd /work && \
        cmake /src/openexr -D BUILD_TESTING=ON -D OPENEXR_INSTALL_EXAMPLES=OFF -D OPENEXR_RUN_FUZZ_TESTS=OFF && \
        make OpenEXRCore && \
        make OpenEXRCoreTest/fast && \
        ctest -R 'OpenEXRCore\\..+' 10\n\
  LOG_FILE=\\\"/work/Testing/Temporary/LastTest.log\\\"\n\
  if [ -f \\\"\\$LOG_FILE\\\" ]; then\n\
    echo \\\"\\n========== Test Log Output ==========\\\"\n\
    cat \\\"\\$LOG_FILE\\\"\n\
    echo \\\"\\n========== End of Test Log ==========\\\"\n\
  else\n\
    echo \\\"\\n[ERROR] Test log file not found: \\$LOG_FILE\\\"\n\
  fi\n",
    "libexif":"autoreconf -fiv && \
        ./configure && \
        make \
        && \
        make install && \
        make check",
    "zstd":"make check ",
    "ndpi": "cd /src && tar -xvzf libpcap-1.9.1.tar.gz && cd libpcap-1.9.1 && ./configure --disable-shared && make -j\\$(nproc) && make install\n\
[ -d /src/json-c ] && cd /src/json-c && mkdir -p build && cd build && cmake -DBUILD_SHARED_LIBS=OFF .. && make install\n\
cd /src/ndpi && sh autogen.sh && ./configure && make -j && cd tests && ./do.sh",
    "imagemagick":"apt-get update && apt-get install -y perl build-essential wget tar libperl-dev && apt-get clean && apt-get install -y ghostscript libfreetype6-dev libbz2-dev libtiff5-dev libjpeg-dev libopenjp2-7-dev libx11-dev libxext-dev hp2xx ffmpeg && ./configure --with-perl && make && make install && ldconfig /usr/local/lib && make check ; cd PerlMagick && make test\n\
  echo 'Unit tests that call the target function:'\n\
  for log in \\$(find /src/imagemagick/tests -type f -name '*.log'); do\n\
    echo \\\"----------------------------------------\\\"\n\
    echo \\\"Test Name: \\$(basename \\\"\\$log\\\" .log)\\\"\n\
    echo \\\"----------------------------------------\\\"\n\
    cat \\\"\\$log\\\"\n\
    echo \\\"\\n\\\"\n\
  done\n\
  for log in \\$(find /src/imagemagick/Magick++ -type f -name '*.log'); do\n\
    echo \\\"----------------------------------------\\\"\n\
    echo \\\"Test Name: \\$(basename \\\"\\$log\\\" .log)\\\"\n\
    echo \\\"----------------------------------------\\\"\n\
    cat \\\"\\$log\\\"\n\
    echo \\\"\\n\\\"\n\
  done",
    #"mupdf":"",
    #"leptonica":"arvo compile && make check",
    "hunspell":"arvo compile && make check",
    "opensc":"apt update && apt install -y softhsm2 libglib2.0-dev\n\
  git clone https://github.com/clibs/cmocka.git /root/cmocka && \\\n\
    cd /root/cmocka && \\\n\
    cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Debug -DUNIT_TESTING=ON && \\\n\
    cd build && make && make install\n\
  cd /src/opensc && \\\n\
    ./bootstrap && \\\n\
    sed -i '/#include <setjmp.h>/a #include <stdint.h>' configure.ac && \\\n\
    autoreconf -i && \\\n\
    PKG_CONFIG_PATH=/usr/local/lib/pkgconfig ./configure --disable-optimization --disable-pcsc --enable-ctapi --enable-cmocka --enable-tests CFLAGS=\\\"-I/usr/local/include\\\" LDFLAGS=\\\"-L/usr/local/lib\\\" && \\\n\
    grep -rl '#include <cmocka.h>' /src/opensc | while read -r file; do\n\
      if ! grep -q '#include <stdarg.h>' \\\"\\$file\\\"; then\n\
        sed -i '/#include <cmocka.h>/i #include <stdarg.h>' \\\"\\$file\\\"\n\
      fi\n\
      if ! grep -q '#include <stddef.h>' \\\"\\$file\\\"; then\n\
        sed -i '/#include <cmocka.h>/i #include <stddef.h>' \\\"\\$file\\\"\n\
      fi\n\
      if ! grep -q '#include <stdint.h>' \\\"\\$file\\\"; then\n\
        sed -i '/#include <cmocka.h>/i #include <stdint.h>' \\\"\\$file\\\"\n\
      fi\n\
      if ! grep -q '#include <setjmp.h>' \\\"\\$file\\\"; then\n\
        sed -i '/#include <cmocka.h>/i #include <setjmp.h>' \\\"\\$file\\\"\n\
      fi\n\
    done\n\
  make -j && make install\n\
  export LD_LIBRARY_PATH=/usr/local/lib:\\$LD_LIBRARY_PATH && \\\n\
  export CMOCKA_MESSAGE_OUTPUT=stdout && \\\n\
  cd /src/opensc/src/tests && \\\n\
    make check\n\
  if [ -d /src/opensc/src/tests/unittests ]; then\n\
    find /src/opensc/src/tests/unittests -type f -executable | while read -r test_exec; do\n\
      echo \\\"Running \\$test_exec...\\\"\n\
      \\$test_exec\n\
    done\n\
  else\n\
      echo \\\"No unit tests found in this commit.\\\"\n\
  fi",
    "libxml2":"arvo compile && ASAN_OPTIONS=\'detect_leaks=0\' make check",
    "gpac":"arvo compile && \
        apt-get update && \
        apt-get -y install time file jackd psmisc bsdmainutils && \
        PATH=\\$PATH:/src/gpac/bin/gcc/ && \
        cd testsuite && \
        ./make_tests.sh -quick -no-hash",
    "matio":"arvo compile && make CHECK_ENVIRONMENT=' 1-2800 2990-' check",
    "htslib":"autoconf && \
        autoheader && \
        ./configure && \
        make && \
        make -i check",
    "mruby":"cd mruby && rake all && rake test -v",
    "libarchive":"arvo compile && cd libarchive/build2 && ctest\n\
  LOG_FILE=\\\"/src/libarchive/build2/Testing/Temporary/LastTest.log\\\"\n\
  if [ -f \\\"\\$LOG_FILE\\\" ]; then\n\
    echo \\\"\\n========== Test Log Output ==========\\\"\n\
    cat \\\"\\$LOG_FILE\\\"\n\
    echo \\\"\\n========== End of Test Log ==========\\\"\n\
  else\n\
    echo \\\"\\n[ERROR] Test log file not found: \\$LOG_FILE\\\"\n\
  fi\n",
    "libdwarf":"mkdir build && cd build && cmake ../ -DDO_TESTING=ON && make && ctest\n\
  LOG_FILE=\\\"/src/libdwarf/build/Testing/Temporary/LastTest.log\\\"\n\
  if [ -f \\\"\\$LOG_FILE\\\" ]; then\n\
    echo \\\"\\n========== Test Log Output ==========\\\"\n\
    cat \\\"\\$LOG_FILE\\\"\n\
    echo \\\"\\n========== End of Test Log ==========\\\"\n\
  else\n\
    echo \\\"\\n[ERROR] Test log file not found: \\$LOG_FILE\\\"\n\
  fi\n",
    "libsndfile":"arvo compile && make check",
    "file":"autoreconf -i && ./configure && make && make check",
    "assimp":"cmake CMakeLists.txt -G \"Ninja\" -DBUILD_SHARED_LIBS=OFF -DASSIMP_BUILD_ZLIB=ON -DASSIMP_BUILD_TESTS=ON -DASSIMP_BUILD_ASSIMP_TOOLS=OFF -DASSIMP_BUILD_SAMPLES=OFF && \
        cmake --build . && \
        ./bin/unit",
    "ots":"arvo compile && \
        cd /work/build && \
        meson test cff_charstring layout_common_table test_bad_fonts", # do not check the fuzzing fonts
    "pcapplusplus":"arvo compile && cmake -S . -B build -DPCAPPP_BUILD_TESTS=ON && cmake --build build && cd build && make test\n\
  LOG_FILE=\\\"/src/PcapPlusPlus/build/Testing/Temporary/LastTest.log\\\"\n\
  if [ -f \\\"\\$LOG_FILE\\\" ]; then\n\
    echo \\\"\\n========== Test Log Output ==========\\\"\n\
    cat \\\"\\$LOG_FILE\\\"\n\
    echo \\\"\\n========== End of Test Log ==========\\\"\n\
  else\n\
    echo \\\"\\n[ERROR] Test log file not found: \\$LOG_FILE\\\"\n\
  fi\n",
    "exiv2":"arvo compile && cd build && ctest ../unitTests/ -E bugfixTests",
    "c-blosc2":"cmake . -DBUILD_FUZZERS=OFF && make clean && make && ctest\n\
  LOG_FILE=\\\"/src/c-blosc2/Testing/Temporary/LastTest.log\\\"\n\
  if [ -f \\\"\\$LOG_FILE\\\" ]; then\n\
    echo \\\"\\n========== Test Log Output ==========\\\"\n\
    cat \\\"\\$LOG_FILE\\\"\n\
    echo \\\"\\n========== End of Test Log ==========\\\"\n\
  else\n\
    echo \\\"\\n[ERROR] Test log file not found: \\$LOG_FILE\\\"\n\
  fi\n",
    "open62541":"apt-get install -y git build-essential gcc pkg-config cmake python cmake-curses-gui libmbedtls-dev check libsubunit-dev python-sphinx graphviz  python-sphinx-rtd-theme && mkdir build && cd build && cmake .. -DUA_BUILD_UNIT_TESTS=ON && make && make test",
    "libplist":"./autogen.sh --without-cython && make && make check",
    "libass":"arvo compile && cd /work/build && ninja test",
    "libxml":"arvo compile && make check",
    "libzmq":"./autogen.sh && ./configure && make && make check",
    "lldpd":"./autogen.sh && ./configure && make && make check",
    "libxslt":"arvo compile && make check",
    "oniguruma":"./autogen.sh && ./configure && make && make check",
    "libpsl":"./autogen.sh && ./configure && make && make check",
    "sleuthkit":"apt-get update && apt-get -y install wget libcppunit-dev && ./bootstrap && ./configure && make && make check",
    "libssh2":"arvo compile && make check",
    "fribidi":"apt-get -y install libtool autoconf && ./autogen.sh && ./configure && make && make check",
    "espeak-ng":"./autogen.sh ; ./configure ; make ; make check -k",
    "wolfssl":"./autogen.sh && ./configure && make && make check\n\
  LOG_FILE=\\\"/src/wolfssl/testsuite/testsuite.log\\\"\n\
  if [ -f \\\"\\$LOG_FILE\\\" ]; then\n\
    echo \\\"\\n========== Test Log Output ==========\\\"\n\
    cat \\\"\\$LOG_FILE\\\"\n\
    echo \\\"\\n========== End of Test Log ==========\\\"\n\
  else\n\
    echo \\\"\\n[ERROR] Test log file not found: \\$LOG_FILE\\\"\n\
  fi\n",
    "coturn":"./configure && make && make check",
    "hiredis":"apt-get update && apt-get -y install redis-server && make && make check",
    "jbig2dec":"ln -s /usr/bin/python3 /usr/bin/python && ./autogen.sh && ./configure && make && make check",
    #"ntopng":"apt-get update && apt-get -y install ./autogen.sh && ",
    "wireshark":"apt-get update && apt-get -y install qt5-default libgtk2.0-dev libpcap-dev && mkdir build && cd build && cmake -DBUILD_wireshark=OFF .. && make && make  test\n\
  LOG_FILE=\\\"/src/wireshark/build/Testing/Temporary/LastTest.log\\\"\n\
  if [ -f \\\"\\$LOG_FILE\\\" ]; then\n\
    echo \\\"\\n========== Test Log Output ==========\\\"\n\
    cat \\\"\\$LOG_FILE\\\"\n\
    echo \\\"\\n========== End of Test Log ==========\\\"\n\
  else\n\
    echo \\\"\\n[ERROR] Test log file not found: \\$LOG_FILE\\\"\n\
  fi\n",
    "jsoncpp":"arvo compile",
    #"uwebsockets":"apt-get -y install zlib1g-dev clang && make -j && export CXX=\"clang++ -stdlib=libc++\" && make -C ",
    #"gdal":"",
    "ffmpeg":"cd ffmpeg ; export CFLAGS=\\\"\\$(echo \\\"\\$CFLAGS\\\" | sed 's/-gline-tables-only//')\\\" ; export CXXFLAGS=\\\"\\$(echo \\\"\\$CXXFLAGS\\\" | sed -e 's/-gline-tables-only//' -e 's/-stdlib=libc++//')\\\" ; CC=gcc CXX=g++ ./configure --samples=fate-suite/ ; make ; make fate-rsync ; make -k fate",
    "binutils-gdb":"cd binutils-gdb ; apt-get install -y dejagnu ; export CFLAGS=\\\"\\$(echo \\$CFLAGS | sed 's/-gline-tables-only//')\\\" ; export CXXFLAGS=\\\"\\$(echo \\$CXXFLAGS | sed 's/-gline-tables-only//')\\\" ; export CXXFLAGS=\\\"\\$(echo \\$CXXFLAGS | sed 's/-stdlib=libc++//')\\\" ; CC=gcc CXX=g++ ./configure ; make ; make install ; make check RUNTESTFLAGS='GDB=/usr/local/bin/gdb gdb.base/a2-run.exp'",
    "librawspeed":"mkdir build && cd build && cmake -DWITH_PTHREADS=OFF -DWITH_OPENMP=OFF -DWITH_PUGIXML=OFF -DUSE_XMLLINT=OFF -DWITH_JPEG=OFF -DWITH_ZLIB=OFF -DALLOW_DOWNLOADING_GOOGLETEST=ON .. ; make ; make test\n\
  LOG_FILE=\\\"/src/librawspeed/build/Testing/Temporary/LastTest.log\\\"\n\
  if [ -f \\\"\\$LOG_FILE\\\" ]; then\n\
    echo \\\"\\n========== Test Log Output ==========\\\"\n\
    cat \\\"\\$LOG_FILE\\\"\n\
    echo \\\"\\n========== End of Test Log ==========\\\"\n\
  else\n\
    echo \\\"\\n[ERROR] Test log file not found: \\$LOG_FILE\\\"\n\
  fi\n",
    "openthread":"arvo compile && make check",
    "fluent-bit":"arvo compile && cd fluent-bit/build && make test; cat /src/fluent-bit/build/Testing/Temporary/LastTest.log",
    #"ntopng":"apt-get update && apt-get install -y build-essential git bison flex libxml2-dev libpcap-dev libtool libtool-bin rrdtool librrd-dev autoconf pkg-config automake autogen redis-server wget libsqlite3-dev libhiredis-dev libmaxminddb-dev libcurl4-openssl-dev libpango1.0-dev libcairo2-dev libnetfilter-queue-dev zlib1g-dev libssl-dev libcap-dev libnetfilter-conntrack-dev libreadline-dev libjson-c-dev libldap2-dev rename libsnmp-dev libexpat1-dev libmaxminddb-dev libradcli-dev libjson-c-dev libzmq3-dev curl jq libnl-genl-3-dev libgcrypt20-dev && ./autogen.sh && ./configure && make -j"
    #"lxc":"./autogen.sh && ./configure && make && make check", # DOES NOT WORK
    "knot-dns":"apt-get update && apt-get -y install libtool autoconf automake make pkg-config liburcu-dev libgnutls28-dev libedit-dev liblmdb-dev && ./autogen.sh && ./configure && make && make -i check",
    "aom":"mkdir build ; cd build && cmake .. && make && ./test_libaom",
    #"poppler":"apt-get update && apt-get install -y libfreetype6-dev libfontconfig1-dev libcairo2-dev libjpeg-dev libopenjp2-7-dev libpng-dev libtiff-dev libglib2.0-dev libboost-dev libcurl4-openssl-dev liblcms2-dev libssl-dev poppler-data && git clone --depth 1 https://gitlab.freedesktop.org/poppler/test.git /src/test-data && mkdir build && cd build && cmake -DBUILD_GTK_TESTS=ON -DBUILD_QT5_TESTS=ON -DBUILD_QT6_TESTS=ON -DBUILD_CPP_TESTS=ON -DBUILD_MANUAL_TESTS=ON .. && make", # currently does not work 17607
    "libgit2":"mkdir build && cd build && cmake -DBUILD_SHARED_LIBS=OFF -DUSE_HTTPS=OFF -DUSE_SSH=OFF -DUSE_BUNDLED_ZLIB=ON .. && make && ctest", # 11382
    "unicorn":"apt-get update && apt-get install -y bsdmainutils wget xz-utils cmake libcmocka-dev && cd unicorn && ./make.sh && make -i test", # 10445
    "flac":"cd flac ; apt-get update && apt-get install -y libtool-bin libogg-dev vorbis-tools oggz-tools && ./autogen.sh && CFLAGS=\"-pthread\" LDFLAGS=\"-pthread\" ./configure && make && make check -i", # 47525
    #"libreoffice":"apt-get update && apt-get install git build-essential zip ccache junit4 libkrb5-dev nasm graphviz python3 python3-dev qtbase5-dev libkf5coreaddons-dev libkf5i18n-dev libkf5config-dev libkf5windowsystem-dev libkf5kio-dev libqt5x11extras5-dev autoconf libcups2-dev libfontconfig1-dev gperf openjdk-17-jdk doxygen libxslt1-dev xsltproc libxml2-utils libxrandr-dev libx11-dev bison flex libgtk-3-dev libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev ant ant-optional libnss3-dev libavahi-client-dev libxt-dev && ./autogen.sh", # 8252
    #"arrow":"", # 23916
    "harfbuzz":"apt-get update && apt-get install -y libfreetype6-dev libglib2.0-dev libcairo2-dev autoconf automake libtool pkg-config ragel gtk-doc-tools && ./autogen.sh && CFLAGS=\"-pthread\" LDFLAGS=\"-pthread\" ./configure && make && make check",
    "libjxl": "arvo compile && cd /work/libjxl-corpus && ninja test",
    "libvips": "arvo compile && make check",
    # "ntopng":"apt-get update && apt-get install -y libhiredis-dev && ./.autogen.sh && ./configure && make unit_test", # Needs work
    "spice-usbredir": "arvo compile && cd build && meson test",
    "wpantund": "arvo compile && make check",
    "flatbuffers": "cd flatbuffers && cmake -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS='-pthread' && make  && make test   ",
    "gdal": "",
    "skia": "apt-get update && apt-get install -y software-properties-common && add-apt-repository -y ppa:ubuntu-toolchain-r/test && apt-get update && apt-get install -y gcc-9 g++-9 && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90 && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 90 && apt-get update && apt-get install -y libfontconfig1-dev && python tools/git-sync-deps && apt-get install -y clang && rm -rf out/Debug && CC=clang CXX=clang++ bin/gn gen out/Debug --args='cc=\\\"clang\\\" cxx=\\\"clang++\\\"' && ninja -C out/Debug dm && out/Debug/dm -v --src tests",
    "libredwg": "cd libredwg && ./autogen.sh && ./configure && make  && yes | ./unit_testing_all.sh",
    # "poppler": "apt-get update && apt-get install -y libfontconfig1-dev libjpeg-dev libopenjp2-7-dev && \
    #             git clone git://git.freedesktop.org/git/poppler/test testdata && \
    #             mkdir build && cd build && cmake .. -DTESTDATADIR=../testdata && make -j4 && make install\
    #             cd ../testdata && make"
    #"lwan":"apt-get update && apt-get -y install git cmake zlib1g-dev pkg-config python-dev lua5.1-dev libsqlite3-dev libmbedtls-dev python curl && curl https://bootstrap.pypa.io/pip/2.7/get-pip.py --output get-pip.py && python get-pip.py && mkdir build && cd build && cmake .. && make && make testrunner",
    #"freeradius-server":"",
    #"glib":"",
    #"proj.4":"",
    #"icu":"",
    "selinux":"apt-get update && apt-get install -y --no-install-recommends --no-install-suggests bison flex gawk gcc gettext make libaudit-dev libbz2-dev libcap-dev libcap-ng-dev libcunit1-dev libglib2.0-dev libpcre2-dev pkgconf python3 systemd xmlto && make clean distclean ; make DESTDIR=~/obj install ; DESTDIR=~/obj ./scripts/env_use_destdir make test", 
    "lcms":"./autogen.sh && make && make check", 
    "php-src":"arvo compile && make test",
    "openvswitch":"arvo compile && make check",
    #"curl":"cd /src/curl",
    "ghostpdl":""
}

_default_pattern = r"\n(?P<status>[A-Z]+): (?P<name>.*)"
_ctest_pattern = r"\d+/\d+\s*Test\s*#\d+:\s(?P<name>\S*)(?P<status>.+)"
_google_test_pattern = r"\[ RUN\s*\]\s(?P<name>.*)[\s\S]*?\[\s*(?P<status>.*) \]"
unittest_patterns = {
    "libexif":_default_pattern,
    "ndpi":r"\n(?P<name>\S+\.\S+)\s+(?P<status>OK|FAIL|ERROR|SKIPPED)",
    "yara":_default_pattern,
    "wolfmqtt":_default_pattern,
    "imagemagick":_default_pattern,
    "hunspell":_default_pattern,
    #"leptonica":_default_pattern,
    "opensc":_default_pattern,
    "zstd":r"(?:\n(?P<total>test.*)|(?P<status>[eE]rror.*)):\s+(?P<name>.*)",
    "openexr":_ctest_pattern,
    "libxml2":r"(?:(?:Total(?::)?\s(?:\d+\sfunctions,\s)?(?P<total>\d+)\stests,\s(?:\d+|no)\serrors)|(?:(?P<name>\S+)\s(?P<status>failed)))",
    "gpac":r"\n(?P<name>[\S]*?):\s?(?P<status>.*?(Fail|OK))",
    "matio":r"\n\s*(?P<name>\d+: .*?\S)\s+(?P<status>ok|FAILED|skipped)\b",
    "htslib":r"Testing (?P<name>.*?)\.\.\.[\s\S]*?Unexpected failures: (?P<num_unexpected_failures>\d+)\n",  # different, counts failures in groups
    "mruby":r"(?P<name>.*?) : (?P<status>\.|F)\n",
    "libarchive":_ctest_pattern,
    "libdwarf":_ctest_pattern,
    "libsndfile":[
        r" {4}(?P<name>[\w \/]+?)\s*:\s*(?P<status>\w+)\n",
        r" {4}(?P<name>\w+)\s+:\s+\.+\s+(?P<status>\w+)\n",
        r" {4}(?P<name>[\w ]+ : \w+\.\w+)\s+(?P<status>\w+)\n",
        r" {4}(?P<name>[\w\(\) \/]+ +: .*) : (?P<status>\w+)\n",
        r" {4}(?P<name>[\w\(\) \/]+ +: .*)\s+\.+\s+(?P<status>\w+)\n",
    ],
    "file":r"Running test: (?P<name>\S+)\n.*\n(?P<status>(?i)error)?",  # passing tests have no output, so treat no status as default pass
    "assimp":_google_test_pattern,
    "ots":r"\d\/\d\s(?P<name>\S*)\s+(?P<status>\S*)",
    "c-blosc2":_ctest_pattern,
    "exiv2":_ctest_pattern,
    "open62541":_ctest_pattern,
    "pcapplusplus":r"(?P<name>\w+)\s+: (?P<status>PASSED|FAILED)\s",
    "libplist":_default_pattern,
    "libass":r"\[\s*RUN\s*\]\s(?P<name>.*)[\s\S]*?(?P<status>OK|Error)",
    #"selinux":r".*",
    #"uwebsockets":r".*",
    #"tpm2":r".*",
    "libzmq":_default_pattern,
    "lldpd":_default_pattern,
    "libxslt":r"#\sRunning\s(?P<name>.*)[\s\S]*?(?:(?P<status>fail)|#|$)",
    "oniguruma":_default_pattern,
    "libpsl":_default_pattern,
    "sleuthkit":_default_pattern,
    "libssh2":_default_pattern,
    "fribidi":_default_pattern,
    "espeak-ng":r"testing\s(?P<name>.*)(?P<status>[\s\S]*?)(?=testing|make: Target 'check')",
    "wolfssl":r"\n(?P<name>[ \w-]+)\b\s+test (?P<status>\w+)",
    "coturn":r"(?P<name>.*)\s?:\s?(?P<status>.*)",
    "hiredis":r"#\d+\s(?P<name>.*): ?(?P<status>.*)",
    "jbig2dec":_default_pattern,
    "wireshark":r"\n(?P<name>test_[^\s]*)\s+\([^)]*\)[\s\S]*?\.{3}\s+[\s\S]*?\b(?P<status>ok|FAIL|skipped)\b",
    "jsoncpp":r"Testing\s(?P<name>.*):\s(?P<status>.*)",
    #"uwebsockets":r"", # TODO add parser
    #"gdal":"",
    "ffmpeg":r"TEST\s+(?P<name>.*)(?P<status>\n(FAIL)?)",
    "binutils-gdb":_default_pattern,
    "librawspeed":_ctest_pattern,
    "openthread":_default_pattern,
    "fluent-bit":r"\nTest (?P<name>.+?)\.{3}[\s\S]+?\[ (?P<status>\w+) \]",
    "libjxl": _google_test_pattern,
    "libvips": _default_pattern,
    "spice-usbredir": r"\d\/\d\s(?P<name>\S*)\s+(?P<status>\S*)",
    "wpantund": _default_pattern,
    #"ntopng":"",
    "flatbuffers": _ctest_pattern, 
    "gdal":[_google_test_pattern, r"---> group: (?P<name>.*)\n[\s\S]*?(?P<status>fail)"], 
    "skia": r"(?P<status>FAILURE|done)  unit test  (?P<name>[\S]*)",
    "libgit2":_ctest_pattern,
    "selinux":r"Test:\s(?P<name>\S*)\s.*\.\n?(?P<status>.+)", 
    "lcms":r"Checking (?P<name>.*) \.+(?P<status>[A-z]+)", 
    #"leptonica":"", 
    "openvswitch":r"\n\s*\d+:\s*(?P<name>.*)\s*(?P<status>ok|skipped|FAILED)", 
    "php-src":r"[\r\n](?P<status>PASS|FAIL|SKIP).*?\[(?P<name>[^\]]+)\] (?:\n|reason:)",
    "flac":r"\+\+\+ .*?test: (?P<name>.*?)\n[^\+]*?(?P<status>PASSED)!", # if a test fails, it won't be included at all (not even in failing), unsure what failing test looks like
    "harfbuzz":_default_pattern,
    "aom":_google_test_pattern,
    "libredwg" : r"(?P<status>ok|not ok)\s+(?P<name>\d+.*?)\n",
    #"lwan":r"",
}

bad_projects = [
    "ghostpdl", # cannot test
    "serenity", # build issues
    "mupdf", # cannot test
    "libreoffice", # build issues
    "arrow", # build issues
    "poppler", # build issues
    "lxc", # build issues
    "ntopng", # build issues
    "libraw", # cannot test
    "leptonica", # build issues
    "llvm-project", # build issues
    "lwan", # build issues
    "radare2", # test issues
    "freeradius-server", # build issues
    "opensips", # build issues
    "curl", # build issues
    "proj.4" # build issues
]

# for printing additional logs, helpful for finding relevant unit tests
sec_print_commands = {
    "yara":"\n\
  echo 'Unit tests that call the target function:'\n\
  for log in \\$(find /src/yara -type f -name '*.log'); do\n\
    if grep -q 'This is a test for CodeGuard+' \\\"\\$log\\\"; then\n\
      echo \\\"----------------------------------------\\\"\n\
      echo \\\"Test Name: \\$(basename \\\"\\$log\\\" .log)\\\"\n\
      echo \\\"----------------------------------------\\\"\n\
      cat \\\"\\$log\\\"\n\
      echo \\\"\\n\\\"\n\
    fi\n\
  done",

    "hunspell":"\n\
  echo 'Unit tests that call the target function:'\n\
  for log in /src/hunspell/tests/*.log; do\n\
    if grep -q 'This is a test for CodeGuard+' \\\"\\$log\\\"; then\n\
      echo \\\"----------------------------------------\\\"\n\
      echo \\\"Test Name: \\$(basename \\\"\\$log\\\" .log)\\\"\n\
      echo \\\"----------------------------------------\\\"\n\
      cat \\\"\\$log\\\"\n\
      echo \\\"\\n\\\"\n\
    fi\n\
  done",

    "matio":"\n\
  echo 'Unit tests that call the target function:'\n\
  for log in \\$(find /src/matio/test/testsuite.dir -type f -name '*.log'); do\n\
    if grep -q 'This is a test for CodeGuard+' \\\"\\$log\\\"; then\n\
      echo \\\"----------------------------------------\\\"\n\
      echo \\\"Test Name: \\$(basename \\\"\\$log\\\" .log)\\\"\n\
      echo \\\"----------------------------------------\\\"\n\
      cat \\\"\\$log\\\"\n\
      echo \\\"\\n\\\"\n\
    fi\n\
  done",

    "libplist":"\n\
  for log in /src/libplist/test/*.log; do\n\
    if grep -q 'This is a test for CodeGuard+' \\\"\\$log\\\"; then\n\
      echo \\\"----------------------------------------\\\"\n\
      echo \\\"Test Name: \\$(basename \\\"\\$log\\\" .log)\\\"\n\
      echo \\\"----------------------------------------\\\"\n\
      cat \\\"\\$log\\\"\n\
      echo \\\"\\n\\\"\n\
    fi\n\
  done",

    "harfbuzz":"\n\
  echo 'Unit tests that call the target function:'\n\
  for log in \\$(find /src/harfbuzz/test -type f -name '*.log'); do\n\
    if grep -q 'This is a test for CodeGuard+' \\\"\\$log\\\"; then\n\
      echo \\\"----------------------------------------\\\"\n\
      echo \\\"Test Name: \\$(basename \\\"\\$log\\\" .log)\\\"\n\
      echo \\\"----------------------------------------\\\"\n\
      cat \\\"\\$log\\\"\n\
      echo \\\"\\n\\\"\n\
    fi\n\
  done",

    "php-src":"\n\
  echo 'Unit tests that call the target function:'\n\
  for log in \\$(find /src/php-src/ext/standard/tests -type f -name '*.log'); do\n\
    if grep -q 'This is a test for CodeGuard+' \\\"\\$log\\\"; then\n\
      echo \\\"----------------------------------------\\\"\n\
      echo \\\"Test Name: \\$(basename \\\"\\$log\\\" .log)\\\"\n\
      echo \\\"----------------------------------------\\\"\n\
      cat \\\"\\$log\\\"\n\
      echo \\\"\\n\\\"\n\
    fi\n\
  done",

    "binutils-gdb":"\n\
  echo 'Unit tests that call the target function:'\n\
  for log in /src/binutils-gdb/libbacktrace/*.log; do\n\
    if grep -q 'This is a test for CodeGuard+' \\\"\\$log\\\"; then\n\
      echo \\\"----------------------------------------\\\"\n\
      echo \\\"Test Name: \\$(basename \\\"\\$log\\\" .log)\\\"\n\
      echo \\\"----------------------------------------\\\"\n\
      cat \\\"\\$log\\\"\n\
      echo \\\"\\n\\\"\n\
    fi\n\
  done",

    "openthread":"\n\
  echo 'Unit tests that call the target function:'\n\
  for log in \\$(find /src/openthread/src/ncp -type f -name '*.log'); do\n\
    if grep -q 'This is a test for CodeGuard+' \\\"\\$log\\\"; then\n\
      echo \\\"----------------------------------------\\\"\n\
      echo \\\"Test Name: \\$(basename \\\"\\$log\\\" .log)\\\"\n\
      echo \\\"----------------------------------------\\\"\n\
      cat \\\"\\$log\\\"\n\
      echo \\\"\\n\\\"\n\
    fi\n\
  done"
}