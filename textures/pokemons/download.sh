#!/bin/bash

echo http://www.pokepedia.fr$(curl http://www.pokepedia.fr/Fichier:Miniat_3_r_$(echo $1).gif 2>/dev/null | grep -E 'images\/.*[0-9]\.gif' | sed -r 's/^.*href="//g' | sed -r 's/\.gif.*$/\.gif/g' | head -n 1)

