stat --printf="%n, %s  bytes\n" $(find $1 -size +$2 -size -$3)
