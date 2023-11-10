#ifndef BOOMBERMAN_CLIENT_H
#define BOOMBERMAN_CLIENT_H


class Client {
private:
    int width;
    int height;

public:
    void Run();

    Client(int width, int height);

    ~Client();
};

#endif