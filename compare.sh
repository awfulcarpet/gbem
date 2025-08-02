#!/bin/sh
rm /tmp/log log
.build/blargg 2 & watch -g cat /tmp/log && pkill blargg
head -n 161502 log > glog
paste -d"\t\t\t\t\t" gblog glog > diff

# less diff
grep 'int' log -m 1 -B 5 -A 5 -n
echo ''
grep 'PC: 00:0050' gblog -m 1 -B 5 -A 5
# head -n 50000 log > tmp
# head -n 50000 ~/opt/Gameboy-logs/EpicLog.txt > tmpl
# diff -y --suppress-common-lines --color=always tmpl tmp | less
# diff -y --color=always tmpl tmp
# diff -l --color log ~/opt/Gameboy-logs/EpicLog.txt
