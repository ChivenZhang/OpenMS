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
- [x] Support [C++20 coroutine](https://en.cppreference.com/w/cpp/language/coroutines) coding
- [x] Support [Reactor](https://en.wikipedia.org/wiki/Reactor_pattern) network model
- [x] Support [Actor](https://en.wikipedia.org/wiki/Erlang_(programming_language)) messaging model
- [x] Support tcp, udp, [kcp](https://github.com/skywind3000/kcp), rpc, http, mysql, redis
- [x] Support [Microservice](https://microservices.io/patterns/index.html) cluster
- [x] Support Windows, macOS, Linux
- [ ] Support Hot module replacement

## How to install
* Download and install the [VCPKG](https://learn.microsoft.com/zh-cn/vcpkg/get_started/overview) package manager.
* Set yours "VCPKG_HOME" in [CMakePresets.json](https://github.com/ChivenZhang/OpenMS/tree/master/CMakePresets.json).
* Use [CLion](https://www.jetbrains.com.cn/en-us/clion/) or [Visual Studio](https://visualstudio.microsoft.com/) to open the project.
* See [Sample](https://github.com/ChivenZhang/OpenMS/tree/master/Sample) for more details about how to use.

```c++
// Master service instance
class Master : public MasterService
{
protected:
	void onInit() override;
	void onExit() override;
};
OPENMS_RUN(Master)
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
<div><img src="Framework.png" style="width:100%;"  alt="missing"/></div>
