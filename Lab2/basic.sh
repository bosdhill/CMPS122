#!/bin/bash
nc -zv 192.168.1.10 1024-65535 2>&1 | tee out | grep -- "Connection to" 
