#!/bin/bash
chmod +x scripts/*.sh
chmod +x python/smash_it.py
scripts/./findport.sh $1
python/./smash_it.py --ip $1 --key $2 --user $3 --dict $4
rm ports response.html
