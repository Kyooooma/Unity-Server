#ifndef Handler_H
#define Handler_H
#include "connect/connect.h"

struct Handler{
    static void handle_moveinfo(char * data, int len);

    static void handle_loginfo(char * data, int len);
};
#endif