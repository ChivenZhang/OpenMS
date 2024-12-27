<div align="center">
<pre>
  ___                   __  __ ____  
 / _ \ _ __   ___ _ __ |  \/  / ___| 
| | | | '_ \ / _ \ '_ \| |\/| \___ \ 
| |_| | |_) |  __/ | | | |  | |___) |
 \___/| .__/ \___|_| |_|_|  |_|____/ 
======|_|============================

:: OpenMS ::                (v1.0.0)
</pre>
</div>

## Description
The Distributed Network Framework based on Microservice Theory.

## Features
- [x] Support reactor network model
- [x] Support tcp, udp, kcp, rpc protocols
- [x] Support Erlang-style message
- [x] Support cpp20 coroutine codes
- [x] Support windows, macOS deployed, linux under way
- [ ] Support microservice cluster
- [ ] Support Hot module replacement

## How to install
* Use git to clone this repository.
* Add "VCPKG_ROOT_CUSTOM" variable to environment.
* Use vcpkg to install third-party on manifest mode.
* Use cmake to open CMakeLists.txt and build.
* See "Sample/DemoService.cpp" for more details.

## Architecture
<div align="center">
  <img src="Framework.png" style="width:100%;" />
</div>
