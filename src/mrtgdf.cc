// Copyright © 2013, 2014, 2017 Richard Kettlewell
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
// 02110-1301, USA.

#if HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdlib>
#if HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif
#if HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#if HAVE_SYS_MOUNT_H
# include <sys/mount.h>
#endif
#include <sys/stat.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <cerrno>
#include <getopt.h>

static std::string cacheDir;

// syscall wrappers

static void stat(const std::string &path, struct stat &sb) {
  if(stat(path.c_str(), &sb) < 0)
    throw std::runtime_error("stat " + path + ": " + strerror(errno));
}

static void statfs(const std::string &path, struct statfs &sf) {
  if(statfs(path.c_str(), &sf) < 0)
    throw std::runtime_error("statfs " + path + ": " + strerror(errno));
}

static void uname(struct utsname &u) {
  if(uname(&u) < 0)
    throw std::runtime_error("uname: " + std::string(strerror(errno)));
}

// Return the directory name containing PATH
static std::string dirname(const std::string &path) {
  // bizarro API
  char buffer[path.size() + 1];
  strcpy(buffer, path.c_str());
  return dirname(buffer);
}

// Return true if PATH is a mount point, else false
static bool isMountPoint(const std::string &path) {
  struct stat sb, sbparent;
  stat(path, sb);
  stat(dirname(path), sbparent);
  if(sb.st_dev != sbparent.st_dev)
    return true;                // definitely a mount point
  if(sb.st_ino == sbparent.st_ino)
    return true;                // root
  return false;                 // not a mount point
}

// Return COUNT/MAX as a percentage
template<typename T>
static int percent(T count, T max) {
  if(!max)
    return 0;
  return (int)round(100.0 * count / max);
}

// Encode PATH into a basename
static std::string encode(const std::string &path) {
  std::stringstream ss;
  ss << std::uppercase << std::hex << std::setfill('0');
  for(auto c: path) {
    if(c == '/' || c <= ' ' || c > 0x7E)
      ss << '%' << std::setw(2) << (int)(unsigned char)c;
    else
      ss << c;
  }
  return ss.str();
}

// Return the cache location for PATH
static std::string cachePath(const std::string &path) {
  return cacheDir + "/" + encode(path);
}

// Return the cached statfs data for PATH to SF
static void retrieve(const std::string &path, struct statfs &sf) {
  std::string cp = cachePath(path);
  int fd = open(cp.c_str(), O_RDONLY);
  if(fd < 0)
    throw std::runtime_error("open " + cp + ": " + strerror(errno));
  ssize_t rc = read(fd, &sf, sizeof sf);
  if(rc < 0)
    throw std::runtime_error("reading " + cp + ": " + strerror(errno));
  if(rc != sizeof sf)
    throw std::runtime_error("reading " + cp + ": truncated");
  if(close(fd) < 0)
    throw std::runtime_error("closing " + cp + ": " + strerror(errno));
}

// Save statfs data for PATH from SF to cache, if it has changed
static void stash(const std::string &path, const struct statfs &sf) {
  std::string cp = cachePath(path);
  try {
    struct statfs cached;
    retrieve(path, cached);
    if(cached.f_blocks == sf.f_blocks
       && cached.f_bfree == sf.f_bfree
       && cached.f_bavail == sf.f_bavail
       && cached.f_files == sf.f_files
       && cached.f_ffree == sf.f_ffree)
      return;
  } catch(std::runtime_error &) {
  }
  mkdir(cacheDir.c_str(), 0777);
  int fd = open(cp.c_str(), O_CREAT|O_WRONLY|O_TRUNC, 0666);
  if(fd < 0)
    throw std::runtime_error("open " + cp + ": " + strerror(errno));
  if(write(fd, &sf, sizeof sf) < 0)
    throw std::runtime_error("writing " + cp + ": " + strerror(errno));
  if(close(fd) < 0)
    throw std::runtime_error("closing " + cp + ": " + strerror(errno));
}

static const struct option options[] = {
  { "help", no_argument, 0, 'h' },
  { "version", no_argument, 0, 'V' },
  { 0, 0, 0, 0 }
};

static void help(void) {
  if(printf("Usage:\n"
            "  mrtgdf [OPTIONS] [--] PATH\n"
            "Options:\n"
            "  -h, --help          Display usage message\n"
            "  -V, --version       Display version string\n"
            "Displays block and inode percentage used for filesystem at PATH,\n"
            "or a cached value if it is not mounted.\n"
            "\n") < 0
     || fflush(stdout) < 0)
    throw std::runtime_error("writing to stdout: " + std::string(strerror(errno)));
}

static void version(void) {
  if(puts(VERSION) < 0
     || fflush(stdout) < 0)
    throw std::runtime_error("writing to stdout: " + std::string(strerror(errno)));
}

int main(int argc, char **argv) {
  try {
    int n;
    while((n = getopt_long(argc, argv, "hV", options, 0)) >= 0) {
      switch(n) {
      case 'h':
        help();
        return 0;
      case 'V':
        version();
        return 0;
      default:
        return 1;
      }
    }
    if(argc - optind != 1)
      throw std::runtime_error("usage: mrtgdf [OPTIONS] [--] PATH");
    const std::string path = argv[optind];
    cacheDir = std::string(getenv("HOME")) + "/.mrtgdf";
    struct statfs sf;
    struct utsname u;
    uname(u);
    if(isMountPoint(path)) {
      statfs(path, sf);
      stash(path, sf);
    } else {
      try {
        retrieve(path, sf);
      } catch(std::runtime_error &) {
        printf("UNKNOWN\nUNKNOWN\n-\n%s\n", u.nodename);
        throw;
      }
    }
    if(printf("%d\n%d\n-\n%s\n",
              percent((sf.f_blocks - sf.f_bavail), sf.f_blocks),
              percent((sf.f_files - sf.f_ffree), sf.f_files),
              u.nodename) < 0
       || fflush(stdout) < 0)
      throw std::runtime_error("writing stdout: "
                               + std::string(strerror(errno)));
  } catch(std::runtime_error &e) {
    fprintf(stderr, "ERROR: %s\n", e.what());
    return 1;
  }
  return 0;
}

