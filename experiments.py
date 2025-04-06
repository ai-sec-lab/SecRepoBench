from projects import *
import sys

def get_meta(sample_id):
    with open(f'ARVO-Meta/meta/{sample_id}.json', 'r') as f:
        meta = json.load(f)
    return meta

debug_commands = {
    "assimp" : "cmake CMakeLists.txt -G \"Ninja\" -DBUILD_SHARED_LIBS=OFF -DASSIMP_BUILD_ZLIB=ON -DASSIMP_BUILD_TESTS=ON \
                -DASSIMP_BUILD_ASSIMP_TOOLS=OFF -DASSIMP_BUILD_SAMPLES=OFF -DCMAKE_BUILD_TYPE=Debug  -DCMAKE_CXX_FLAGS=\"-fno-inline -fno-inline-functions\" \
                -DCMAKE_C_FLAGS=\"-fno-inline -fno-inline-functions\" && cmake --build .",
    "c-blosc2": "cmake .  -DBUILD_FUZZERS=OFF -DCMAKE_CXX_FLAGS=\"-fno-inline -fno-inline-functions\" -DCMAKE_C_FLAGS=\"-fno-inline -fno-inline-functions\" -DCMAKE_BUILD_TYPE=Debug && make clean && make",
    "ffmpeg": "cd ffmpeg ; export CFLAGS=$(echo \"$CFLAGS\" | sed 's/-gline-tables-only//')' -g'; export CXXFLAGS=$(echo \"$CXXFLAGS\" | sed -e 's/-gline-tables-only//' -e 's/-stdlib=libc++//')' -g'; CC=gcc CXX=g++ ./configure --samples=fate-suite/ ; make ; make fate-rsync",
    "file": "autoreconf -i && CFLAGS=\"-g\" CXXFLAGS=\"-g\" ./configure && make && make check", # TODO: Fix
    "flac": "cd flac ; apt-get update && apt-get install -y libtool-bin libogg-dev vorbis-tools oggz-tools && ./autogen.sh && CFLAGS=\"-pthread -g\" CXXFLAGS=\"-g\" LDFLAGS=\"-pthread\" ./configure && make && make check -i",
    "fluent-bit": "CFLAGS=\"-g -O0\" CXXFLAGS=\"-g -O0\" arvo compile && cd fluent-bit/build",
    "gpac": "CFLAGS=\"-g -O0\" CXXFLAGS=\"-g -O0\" arvo compile && apt-get update && apt-get -y install time file jackd psmisc bsdmainutils && PATH=$PATH:/src/gpac/bin/gcc/ && cd testsuite"
}

def get_binary_file(test_name, project_name):
    if project_name == "assimp":
        return "lldb ./bin/unit" # check later
    elif project_name == "c-blosc2": # line numbers off?
        command = ""
        if test_name.startswith('test_example'):
            test_name = test_name[13:]
            command = f'./examples/{test_name}'
        elif test_name.startswith('test_compat'):
            test_name = test_name[11:]
            command = f'./compat/{test_name}'
        else:
            command = f'./tests/{test_name}'
        return f'gdb --args {command}'
    elif project_name == "ffmpeg":
        return f'./tests/{test_name}' # fix
    elif project_name == "file":
        test_name = test_name.split('/')[-1]
        return f"LD_LIBRARY_PATH=src/.libs MAGIC=magic/magic gdb --args src/.libs/file {test_name}"
    elif project_name == "flac":
        return f"gdb --args src/test_libFLAC++/.libs/lt-test_libFLAC++"
    elif project_name == "fluent-bit": # uses file name
        command = ""
        if test_name == "http_buffer_increase":
            command = "./bin/flb-it-http_client"
        elif test_name == "json_time_lookup":
            command = "./bin/flb-it-parser"
        elif test_name == "mode_docker":
            command = f"./bin/flb-it-multiline"
        return f"LSAN_OPTIONS=\"verbosity=1:log_threads=1\" ASAN_OPTIONS=\"detect_leaks=0\" gdb --args {command}"
      
sample_id = sys.argv[1]
metadata = get_meta(sample_id)
project_name = metadata["project"]
print(unittest_commands[project_name])
print(metadata["fix"])
print(debug_commands[project_name])