#!/bin/bash
easydb_path="./easydb"
num_inserts=10000
mkfifo pipe
$easydb_path < pipe &

start_time=$(date +%s)

echo "use db" >> pipe

for i in $(seq 1 $num_inserts); do
  echo "insert $i name$i $i item$i@data" >> pipe
done

echo "exit" >> pipe
echo "exit" >> pipe
wait

end_time=$(date +%s)
elapsed_time=$((end_time - start_time))

echo "Insert 100000 records, time: $elapsed_time s"

exec 9>&-
rm pipe
