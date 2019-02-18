#!/bin/bash
nc -zv $1 1024-65535 2>&1 | grep -- "Connection to" > ports
