for i in $(find $1 -size +$2 -size -$3) 
do
printf $i", "
stat --printf="%s bytes\n" $i
done
