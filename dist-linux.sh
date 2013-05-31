#! /bin/bash
mkdir -p ./dist/linux/lib
mkdir -p ./dist/linux/bin

for i in `ldd vrok | grep -o "/.* "`
do
cp $i ./dist/linux/lib
done;

cp ./vrok ./dist/linux/bin

echo -e "#! /bin/bash \n./lib/ld-linux-x86-64.so.2 --library-path ./lib ./bin/vrok" > ./dist/linux/vrok

chmod +x ./dist/linux/vrok