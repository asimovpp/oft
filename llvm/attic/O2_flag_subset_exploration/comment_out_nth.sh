TARGET=$1
LINE=$2
sed -i $LINE's/^/\#/' $TARGET
