#!/bin/bash
chmod +x scripts/*.sh
chmod +x python/checkports.py
scripts/./findport.sh $1
python/./checkports.py --ip $1 --key $2 --user $3 --dict $4
rm ports response.html
