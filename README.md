# pfSense-pkg-bro

Bro Network Security Monitor package for pfSense router/firewall

<br><br>
![Example](https://user-images.githubusercontent.com/20781471/29499167-c581b802-8602-11e7-89dd-db6be4afc797.gif?raw=true)


# Compatibility
This package has been tested on PfSense 2.3.2 amd64. Maybe it might not work with older versions of pfSense.
<br><br>

# Installation

## Download the generated package through [pfSense-pkg-bro](https://github.com/shadonet/pfSense-pkg-bro/files/1248279/pfSense-pkg-bro-2.4.1.zip)

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

# License (Apache 2.0)

 Copyright (c) 2018 Prosper Doko
 All rights reserved.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
