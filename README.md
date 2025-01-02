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
- [x] Support tcp, udp, kcp, rpc, http
- [x] Support Cpp20 coroutine coding
- [x] Support Erlang-style message
- [x] Support microservice cluster
- [x] Support Windows, macOS, Linux
- [ ] Support Hot module replacement

## How to install
* Use git to clone this repository.
* Add "VCPKG_ROOT_CUSTOM" to environment.
* Use vcpkg to install third-party on manifest mode.
* Use Clion or Visual studio to open CMakeLists.txt.
* See folder "Sample" for more details.

## Architecture
<div><img src="Framework.png" style="width:100%;"  alt="missing"/></div>