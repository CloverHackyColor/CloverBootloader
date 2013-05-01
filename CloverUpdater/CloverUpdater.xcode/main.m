//
//  main.m
//  CloverUpdater
//
//  Created by Sergey on 29.04.13.
//  Copyright 2013 Paritet. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Arguments.h"

char* arg1;
char* arg2;

int main(int argc, char *argv[])
{
    arg1 = argv[1];
    arg2 = argv[2];
    return NSApplicationMain(argc,  (const char **) argv);
}
