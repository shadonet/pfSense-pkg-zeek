# Setting up a Development Environment for `pfSense-pkg-zeek`

This is a step-by-step guide to setting up a development environment
for working on the `pfSense-pkg-zeek` package.

For most people, it will be easiest to work in a FreeBSD VM.

## Steps to create development environment

1. Create a FreeBSD VM on your favorite virtualization platform
2. Install `git`: 
```
# pkg update && pkg install git
```
3. Install `zeek` (will automatically install all dependencies):
```
# pkg update && pkg install zeek
```
4. Clone repository to disk:
```
# git clone https://github.com/shadonet/pfSense-pkg-zeek
```
5. Make any changes/updates
6. To make a build of the updated package:
```
# make DISABLE_VULNERABILITIES=yes package
```
7. The output will be a `.txz` file in the `work/pkg/` directory

## Steps to test a package

1. Create a pfSense VM on your favorite virtualization platform
2. Set the interface IP addresses appropriately
3. Enable Secure Shell (sshd)
4. `scp` the `.txz` package generated earlier to the pfSense VM:
```
$ scp pfSense-pkg-zeek-3.0.6.txz root@<pfSense VM IP>:
```
5. SSH to the pfSense VM
6. Drop to a shell (option 8 at the main menu)
1. (If you haven't already) set `FreeBSD: { enabled: yes }` in `/usr/local/etc/pkg/repos/FreeBSD.conf` and `/usr/local/share/pfSense/pkg/repos/pfSense-repo.conf`
7. Install `zeek`:
```
# pkg update && pkg isntall -y zeek
```
8. Install the package:
```
# pkg install pfSense-pkg-zeek-3.0.6.txz
```
