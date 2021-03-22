now=$(date +"%m-%d-%Y_%T")
path=$1"_"$now
mkdir -p $path
cp -r $1 $path
rsync -a $path $2:~/Backup
rm -rf $path
