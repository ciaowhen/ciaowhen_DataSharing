# Libevent库

&nbsp;

## libevent特点

&nbsp;

==开源、精简、跨平台、专注于网络通信==

**源码包安装：**

```c
./configure			检查安装环境 生成makefile
make				生成.o和可执行文件
sudo make install	将必要的资源cp至系统指定目录
进入sample目录，运行demo验证库安装使用情况
编译使用库的.c时，需要加 -levent 选项。
库名 livevent.so --> /usr/local/lib
```

**特性：**

基于“事件”异步通信模型。——回调。

&nbsp;

&nbsp;

## libevent框架

&nbsp;

创建 `event_base`  （底座）

```c
struct event_base *event_base_new（void）;
struct event_base *base = event_base_new（）;
```

创建 事件`event`

* 常规事件 event   `event_new（）`
* `bufferevent`     `buffevent_socket_new（）`;

将事件添加到 `base `上

```c
int event_add（struct event *ev， const struct timeval *tv）
```

循环监听事件满足

```c
int event_base_dispatch（struct event_base *base）;
event_base_dispatch（base）;
```

释放 `event_base`

```c
event_base_free（base）;
```

&nbsp;

## 常规事件 event

&nbsp;

### 创建事件

```c
struct event *ev;
struct event *event_new（struct event_base *base， evutil_socket_t fd， short what， event_callback_fn cb; void *arg）;
```

**base**: event_base_new（）返回值

**fd**: 绑定到 event 上的文件描述符

**what**: 对应的事件（r、w、e）
		EV_READ 		一次读事件
		EV_WRITE		一次写事件
        EV_PERSIST		持续触发。结合 event_base_dispatch函数使用，生效。

**cb**: 一旦事件满足监听条件，回调的函数。

```c
typedef void（*event_callback_fn）（evutil_socket_t fd， short， void *）
```

**arg**: 回调的函数的参数。

**返回值:** 成功创建的event

&nbsp;

### 添加事件

```c
//添加事件到 event_base
int event_add（struct event *ev， const struct timeval *tv）;
ev:	event_new（）的返回值
tv: NULL
```

&nbsp;

### 摘除事件

```c
//从event_base上摘下事件
int event_del（struct event *ev）;
ev: event_new（）的返回值
```

&nbsp;

### 销毁事件

```c
int event_free（struct event *ev）;
ev:event_new（）的返回值&nbsp;&nbsp;
```

&nbsp;

### 未决和非未决

![image-20220627205026081](/home/ciaowhen/Markdown/linux网络编程/image-20220627205026081.png)

**未决：**有资格被处理，但还没有被处理

**非未决：**没有资格被处理

```
event_new --> event --> 非未决 --> event_add --> 未决 -->dispatch（） && 监听事件被触发 --> 激活态 --> 执行回调函数 --> 处理态 --> 非未决 --> event_add && EV_PERSIST --> 未决 --> event_del --> 非未决
```

&nbsp;

&nbsp;

## 带缓冲区的事件 bufferevent

&nbsp;

==#include <event2/bufferevent.h>==

原理：bufferevent 有两个缓冲区：也是队列实现，读走没，先进先出。

读：有数据 ——> 读回调函数被调用 ——>使用bufferevent_read（） ——> 读数据

写：使用 bufferevent_write（） ——> 向写缓冲中写数据 ——> 该缓冲区有数据自动写出 ——> 写完，回调函数被调用

![image-20220629142113505](/home/ciaowhen/Markdown/linux网络编程/image-20220629142113505.png)

&nbsp;

### 创建bufferevent

```c
struct bufferevent *ev;
struct bufferevent *bufferevent_socket_new（struct event_base *base， evutil_socket_t fd，enum buffereven_options options）;

base: event_base
fd: 封装到bufferevent内的fd
options: BEV_OPT_CLOSE_ON_FREE
返回：成功创建的bufferevent事件对象。
```

&nbsp;

### 销毁bufferevent

```c
void  bufferevent_socket_free（struct bufferevent *ev）;
```

&nbsp;

### 给读写缓冲区设置回调

给bufferevent设置回调：
对比event：	event_new（ fd， callback ）;  	event_add（） -- 挂到 event_base 上。
bufferevent_socket_new（fd）  bufferevent_setcb（ callback ）

```c
void bufferevent_setcb（struct bufferevent * bufev，
			bufferevent_data_cb readcb，
			bufferevent_data_cb writecb，
			bufferevent_event_cb eventcb，
			void *cbarg ）;

bufev： bufferevent_socket_new（） 返回值

readcb： 设置 bufferevent 读缓冲，对应回调  read_cb{bufferevent_read（） 读数据  }

writecb： 设置 bufferevent 写缓冲，对应回调 write_cb {  } -- 给调用者，发送写成功通知。  可以 NULL

eventcb： 设置 事件回调。   也可传NULL
```

#### 回调函数

```c
typedef void （*bufferevent_event_cb）（struct bufferevent *bev，  short events， void *ctx）;
		void event_cb（struct bufferevent *bev，  short events， void *ctx）
		{

			。。。。。
		}
		events： BEV_EVENT_CONNECTED
	cbarg：	上述回调函数使用的 参数。
```

#### read回调函数类型

```c
typedef void （*bufferevent_data_cb）（struct bufferevent *bev， void*ctx）;
		void read_cb（struct bufferevent *bev， void *cbarg ）
		{
			.....
			bufferevent_read（）;   --- read（）;
		}
	bufferevent_read（）函数的原型：
		size_t bufferevent_read（struct bufferevent *bev， void *buf， size_t bufsize）;
```

#### write回调函数类型

```c
int bufferevent_write（struct bufferevent *bufev， const void *data，  size_t size）; 
```

&nbsp;

&nbsp;

### 启动、关闭bufferevent的缓冲区

```c
void bufferevent_enable（struct bufferevent *bufev， short events）;   启动	
events： EV_READ、EV_WRITE、EV_READ|EV_WRITE
默认、write 缓冲是 enable、read 缓冲是 disable
bufferevent_enable（evev， EV_READ）;		-- 开启读缓冲。
```

&nbsp;

### 连接客户端

```c
socket（）;connect（）;

int bufferevent_socket_connect（struct bufferevent *bev， struct sockaddr *address， int addrlen）;

bev: bufferevent 事件对象（封装了fd）
address、len：等同于 connect（） 参2/3
```

&nbsp;

### 创建监听服务器

```c
------ socket（）;bind（）;listen（）;accept（）;

struct evconnlistener * listner;

struct evconnlistener *evconnlistener_new_bind （	
	struct event_base *base，
	evconnlistener_cb cb， 
	void *ptr， 
	unsigned flags，
	int backlog，
	const struct sockaddr *sa，
	int socklen）;

base： event_base
cb: 回调函数。 一旦被回调，说明在其内部应该与客户端完成， 数据读写操作，进行通信。
ptr： 回调函数的参数
flags： LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE
backlog： listen（） 2参。 -1 表最大值
sa：服务器自己的地址结构体
socklen：服务器自己的地址结构体大小。

返回值：成功创建的监听器。
```

&nbsp;

### 释放监听服务器

```c
void evconnlistener_free（struct evconnlistener *lev）;
```

&nbsp;

&nbsp;

### 服务器端 libevent 创建TCP连接

1. 创建event_base
2. 创建bufferevent事件对象。bufferevent_socket_new（）;
3. 使用bufferevent_setcb（） 函数给 bufferevent的 read、write、event 设置回调函数。
4. 当监听的 事件满足时，read_cb会被调用， 在其内部 bufferevent_read（）;读
5. 使用 evconnlistener_new_bind 创建监听服务器， 设置其回调函数，当有客户端成功连接时，这个回调函数会被调用。
6. 封装 listner_cb（） 在函数内部。完成与客户端通信。
7. 设置读缓冲、写缓冲的 使能状态 enable、disable
8. 启动循环 event_base_dispath（）;
9. 释放连接。

&nbsp;

### 客户端 libevent 创建TCP连接

1. 创建event_base
2. 创建bufferevent事件对象。bufferevent_socket_new（）;
3. 使用bufferevent_socket_connect（）连接服务器
4. 使用bufferevent_setcb（） 函数给 bufferevent的 read、write、event 设置回调函数。
5. 设置读缓冲、写缓冲的 使能状态 enable、disable
6. 接收、发送数据bufferevent_read（） / bufferevent_write（）
7. 启动循环 event_base_dispath（）;
8. 释放资源。

&nbsp;

&nbsp;

# Libevent实现HTTP服务

该程序使用了Libevent网络库来实现HTTP服务，编译运行main程序后，只要输入相关参数（端口号，工作目录），就可以显示该工作目录下的所有文件信息，包括目录项；能够有效访问该目录下的所有文件，包括文本文件，图片，视频和音频等等。访问错误时会显示错误页面。