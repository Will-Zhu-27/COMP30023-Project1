/**
 * COMP30023 Project 1
 * Main program 
 * Name: Yuqiang Zhu. ID: 853912
 * Email: yuqiangz@student.unimelb.edu.au
 *  
 */

#include <stdio.h>

#include "imageTaggerServer.h"

int main(int argc, char **argv) {
    // check whether input IP and port before start
    if (argc < 3) {
        fprintf(stderr, "usage: %s ip port\n", argv[0]);
        return 0;
    }
    
    runServer(argv[0], argv[1], argv[2]);
    return 0;
}


