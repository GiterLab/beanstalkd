# beanstalkd

Simple and fast general purpose work queue.

See [doc/protocol.md](https://github.com/GiterLab/beanstalkd/blob/master/doc/protocol.md)
for details of the network protocol.

Please note that this project is released with a Contributor
Code of Conduct. By participating in this project you agree
to abide by its terms. See CodeOfConduct.txt for details.

## Quick Start

```bash
make
./beanstalkd
```

also try,

```bash
./beanstalkd -h
./beanstalkd -VVVVV
make CFLAGS=-O2
make CC=clang
make check
make install
make install PREFIX=/usr
```

Requires Linux (2.6.17 or later), Mac OS X, FreeBSD, or Illumos.

Currently beanstalkd is tested with GCC and clang, but it should work
with any compiler that supports C99.

Uses ronn to generate the manual.
See http://github.com/rtomayko/ronn.

## Docker

Build and push the beanstalkd Docker images for amd64 and arm64 platforms

```bash
# For amd64 platform
docker build --no-cache -t tobyzxj/beanstalkd:v2.0-amd64 .
docker push tobyzxj/beanstalkd:v2.0-amd64

# For arm64 platform
docker build --no-cache -t tobyzxj/beanstalkd:v2.0-arm64 .
docker push tobyzxj/beanstalkd:v2.0-arm64
```

Make sure you have the correct Docker buildx setup for multi-platform builds

```bash
./docker_manifest_create.sh tobyzxj/beanstalkd:v2.0 push
```

Run the beanstalkd Docker image

make sure to replace `your_password` with a secure password of your choice. you can also change the volume path to suit your needs.

you can change the `BEANSTALKD_PORT` environment variable to set the port number, default is 11300.

```bash
docker run -p 11300:11300 --restart=always --name beanstalkd \
-e BEANSTALKD_PASSWORD="your_password" \
-v /data/cloud/docker/beanstalkd/data:/giterlab/data \
-d tobyzxj/beanstalkd:v2.0 -b /giterlab/data
```

## Subdirectories

- `adm` - files useful for system administrators
- `ct` - testing tool; vendored from https://github.com/kr/ct
- `doc` - documentation

## Tests

Unit tests are in test*.c. See https://github.com/kr/ct for
information on how to write them.
