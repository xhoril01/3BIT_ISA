/**
 * @file feedreader.cpp
 * @author Denis HORIL (xhoril01@stud.fit.vutbr.cz)
 * @brief ISA Projekt - Čtečka novinek ve formátu Atom a RSS s podporou TLS
 * @version 0.1
 * @date 2022-10-07
 * 
 * @copyright Copyright (c) 2022
 * 
*/

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <csignal>

#include "classes.cpp"

using namespace std;

void prog_interrupt(int sig_num)
{
    cout << "\nInterrupted" << endl;
    IM_FREEEEE;
    exit(sig_num);
}

int main (int argc, char** argv)
{   
    signal(SIGINT, prog_interrupt);

    //Processing arguments
    Args args;
    Process process;
    if(args.argsProcesser(argc,argv) == -1) exit(EXIT_FAILURE);
    if(args.isHelp()) exit(EXIT_SUCCESS);
    if(args.reqArgsCheck() == -1) exit(EXIT_FAILURE);

    //URL check
    if(!args.getURL().empty())
    {
        parsedURL myURL;
        if(!checkURL(args.getURL(), &myURL)) exit(EXIT_FAILURE);
        if(process.connect(&myURL, &args, args.getURL()) == -1)
        {
            exit(EXIT_FAILURE); 
        }
    }
    
    //Extracting feeds from file
    if(!args.getFeedFile().empty())
    {
        process.setPath(args.getFeedFile());
        if(process.feedfile2List() == -1)
        {
            exit(EXIT_FAILURE); 
        }

        process.loopConnect(&args);
    }
    
    exit(EXIT_SUCCESS);
}

