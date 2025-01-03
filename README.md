<div style="text-align: center;">
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
- [x] Support [reactor](https://en.wikipedia.org/wiki/Reactor_pattern) network pattern
- [x] Support tcp, udp, [kcp](https://github.com/skywind3000/kcp), rpc, http
- [x] Support [c++20 coroutine](https://en.cppreference.com/w/cpp/language/coroutines) coding
- [x] Support [Erlang](https://en.wikipedia.org/wiki/Erlang_(programming_language))-like message
- [x] Support [IOC](https://en.wikipedia.org/wiki/Inversion_of_control) collection mechanism
- [x] Support [microservice](https://microservices.io/patterns/index.html) cluster
- [x] Support Windows, macOS, Linux
- [ ] Support Hot module replacement

## How to install
* Set the environment variable "VCPKG_ROOT_CUSTOM".
* Use [vcpkg](https://learn.microsoft.com/zh-cn/vcpkg/get_started/overview) to install dependency on manifest mode.
* Use [CLion](https://www.jetbrains.com.cn/en-us/clion/) or [Visual Studio](https://visualstudio.microsoft.com/) to open folder where contains CMakeLists.txt .
* See directory [./Sample](https://github.com/ChivenZhang/OpenMS/tree/master/Sample) for more details.

```c++
// Master service instance
class Master : public MasterService
{
protected:
	void onInit() override;
	void onExit() override;
};
OPENMS_RUN(Master)

// Apply config in Application.json
// "master": {
//      "server": {
//          "ip": "127.0.0.1",
//          "port": 8080
//      }
// }
```
```c++
// First cluster instance joined to master
class Cluster1 : public ClusterService
{
protected:
	void onInit() override;
	void onExit() override;
};
OPENMS_RUN(Cluster1)
```
```c++
// Second cluster instance joined to master
class Cluster2 : public ClusterService
{
protected:
	void onInit() override;
	void onExit() override;
};
OPENMS_RUN(Cluster2)
```

## Architecture
<div><img src="Architecture.png" style="width:100%;text-align: center;"  alt="missing"/></div>