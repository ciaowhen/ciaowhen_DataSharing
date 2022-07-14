//
// Created by ciaowhen on 2022/7/14.
//

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <signal.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <event2/event.h>
#include <unistd.h>
#include "http_lib.h"

int main(int argc, char *argv[])
{
    if(argc < 3)
    {
        printf("./event_http port path\n");
        return -1;
    }

    event_base *base;
    evconnlistener *listener;
    event *signal_event;

    sockaddr_in sin;
    base = event_base_new();
    if(!base)
    {
        fprintf(stderr, "Could not initialize libevent!\n");
        return -1;
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(atoi(argv[1]));
    int ret = chdir(argv[2]);
    //创建监听的套接字，绑定，监听，接受连接请求
    listener = evconnlistener_new_bind(base, listener_cb,(void *)base, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1, (sockaddr *)&sin,
                                       sizeof(sin));
    if(!listener)
    {
        fprintf(stderr, "Could not  create a listener!\n");
        return 1;
    }
    //创建信号事件，捕捉并处理
    signal_event = evsignal_new(base, SIGINT, signal_cb,(void *)base);
    if(!signal_event || event_add(signal_event, NULL) < 0)
    {
        fprintf(stderr, "Could not create/add a signal event!\n");
        return 1;
    }

    //事件循环
    event_base_dispatch(base);

    //释放监听服务器
    evconnlistener_free(listener);
    //销毁信号事件
    event_free(signal_event);
    //释放event_base
    event_base_free(base);

    printf("done\n");
    return 0;
}