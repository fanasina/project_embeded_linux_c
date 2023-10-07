#!/bin/bash

gcc client_project.c tools.c -o lient_proj


gcc server_main_project.c  tools.c handler_msg.c -o main_srv


