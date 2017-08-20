#!/bin/bash

token=$1
./c_parser json_parser.h | grep $token
