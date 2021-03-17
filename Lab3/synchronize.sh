#!/bin/bash
echo "* * * * * "rsync --archive $1 $2:~/Backup | crontab
