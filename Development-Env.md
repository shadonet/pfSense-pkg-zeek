# Setting up a Development Environment for `pfSense-pkg-zeek`

This is a step-by-step guide to setting up a development environment
for working on the `pfSense-pkg-zeek` package.

For most people, it will be easiest to work in a FreeBSD VM.

## Steps to create development environment

1. Create a FreeBSD VM on your favorite virtualization platform
1. Install `git:
```
# pkg update
# pkg install git
```
1. Install `zeek` (will automatically install all dependencies):
```
# pkg update
# pkg install zeek
```
1. Clone repository to disk:
```
# git clone https://github.com/shadonet/pfSense-pkg-zeek
```
1. Make any changes/updates
1. To make a build of the updated package:
```
# make DISABLE_VULNERABILITIES=yes package
```
1. The output will be a `.txz` file in the `work/pkg/` directory

## Steps to test a package

1. Create a pfSense VM on your favorite virtualization platform
1. Set the interface IP addresses appropriately
1. Enable Secure Shell (sshd)
1. `scp` the `.txz` package generated earlier to the pfSense VM:
```
$ scp pfSense-pkg-zeek-3.0.6.txz root@<pfSense VM IP>:
```
1. SSH to the pfSense VM
1. Drop to a shell (option 8 at the main menu)
1. (If you haven't already) set `FreeBSD: { enabled: yes }` in `/usr/local/etc/pkg/repos/FreeBSD.conf` and `/usr/local/share/pfSense/pkg/repos/pfSense-repo.conf`
1. Install `zeek`:
```
# pkg update && pkg isntall -y zeek
```
1. Install the package:
```
# pkg install pfSense-pkg-zeek-3.0.6.txz
```
