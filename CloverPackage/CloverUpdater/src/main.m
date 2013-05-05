//
//  main.m
//  CloverUpdater
//
//  Created by Slice on 29.04.13.
//

#import <Cocoa/Cocoa.h>
#import "Arguments.h"

char* arg1;
char* arg2;

int main(int argc, char *argv[])
{
    arg1 = argc >= 2 ? argv[1] : "Unknown";
    arg2 = argc >= 3 ? argv[2] : "Unknown";
    return NSApplicationMain(argc,  (const char **) argv);
}
