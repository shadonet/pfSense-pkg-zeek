# pfSense-pkg-bro

Bro Network Security Monitor package for pfSense router/firewall

<br><br>
![Example](https://user-images.githubusercontent.com/20781471/29499167-c581b802-8602-11e7-89dd-db6be4afc797.gif?raw=true)


# Compatibility
This package has been tested on PfSense 2.3.1 amd64. Maybe it might not work with older versions of pfSense.
<br><br>

# Installation

## Download the generated package through [pfSense-pkg-bro](https://drive.google.com/file/d/0B0NDS8_WgeQMUDJ1MTVHU3BvNnc/view?usp=sharing)

## Copy the package from your local machine to your firewall

```shell
scp ~/Downloads/pfSense-pkg-bro-2.4.1.txz root@firewall-ip-address:/tmp/
```
## Install the package on the firewall via pkg add command
```shell
pkg add pfSense-pkg-bro-2.4.1.txz
```
## Contribution
- **Having an issue**? or looking for support? [Open an issue](https://github.com/shadonet/pfSense-pkg-bro/issues/new) and we will get you the help you need.
- Got a **new feature or a bug fix**? Fork the repo, make your changes, and submit a pull request.

## Support this project
If you find this project useful, please star the repo to let people know that it's reliable. Also, share it with friends and colleagues that might find this useful as well. Thank you :smile:

# License (ESF)

Copyright (C) 2017 Prosper Doko

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
