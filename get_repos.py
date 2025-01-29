import json
import os
import subprocess


BASE_DIR = 'repos'

def clone_repository(project_name, repo_url):
    target_dir = os.path.join(BASE_DIR, project_name)
    
    # Skip if already cloned
    if os.path.exists(target_dir):
        print(f"Repository {project_name} already exists, skipping...")
        return True
    
    try:
        subprocess.run(
            ['git', 'clone', repo_url, target_dir],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        return True
    except subprocess.CalledProcessError as e:
        print(f"Failed to clone {project_name}: {e}")
        return False


# with open('final_ids.txt', 'r') as f:
#     ids = f.read().splitlines()[1:]

with open('/home/cdilgren/project_benchmark/filter_logs/samples_each_step.json', 'r') as f:
    samples_each_step = json.load(f)
ids = samples_each_step[3]['sample ids present']

projects = []
urls = []

existing_repos = os.listdir('/home/cdilgren/project_benchmark/repos')
have_61 = ["openthread", "mruby", "jsoncpp", "coturn", "spice-usbredir", "fluent-bit", "ndpi", "libarchive", "yara", "matio", "jbig2dec", "libvips", "htslib", "php-src", "libzmq", "libass", "selinux", "wolfssl", "sleuthkit", "oniguruma", "gdal", "lcms", "openvswitch", "hunspell", "imagemagick", "libjxl", "libdwarf", "zstd", "file", "assimp", "libssh2", "flatbuffers", "libexif", "ots", "openexr", "libgit2", "skia", "open62541", "libxslt", "libpsl", "fribidi", "wolfmqtt", "gpac", "flac", "libredwg", "wpantund", "libsndfile", "binutils-gdb", "wireshark", "aom", "exiv2", "libplist", "harfbuzz", "lldpd", "c-blosc2", "espeak-ng", "librawspeed", "hiredis", "libxml2", "ffmpeg", "pcapplusplus"]

for id in ids:
    with open(f'/home/cdilgren/project_benchmark/ARVO-Meta/meta/{id}.json', 'r') as f:
        meta = json.load(f)
    project = meta['project']
    if project not in projects and project not in existing_repos and project in have_61:
        url = meta['repo_addr']
        projects.append(project)
        urls.append(url)
        clone_repository(project, url)

# print(projects)
# print(len(projects))