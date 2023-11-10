#ifndef BOOMBERMAN_SEVER_H
#define BOOMBERMAN_SEVER_H

class Server {
private:
    int port;

public:
    void Run();

    explicit Server(int port);
};

#endif