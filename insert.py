import subprocess
import time
import os

easydb_path = "./easydb"
num_inserts = 10000
os.mkfifo("pipe")
easydb_process = subprocess.Popen([easydb_path], stdin=open("pipe", "w"))

start_time = time.time()

with open("pipe", "w") as f:
    f.write("use db\n")
    for i in range(1, num_inserts + 1):
        f.write(f"insert {i} name{i} {i} item{i}@data\n")
    f.write("exit\n")
    f.write("exit\n")

easydb_process.wait()

end_time = time.time()
elapsed_time = end_time - start_time

print(f"Insert {num_inserts} records, time: {elapsed_time:.2f} s")

os.unlink("pipe")
