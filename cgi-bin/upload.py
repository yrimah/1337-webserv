#!/usr/bin/python3

import sys
import os
import uuid
import signal
import time

def timeout_handler(signum, frame):
    for file_path in uploaded_files:
        if os.path.exists(file_path):
            print("timeout")
            os.remove(file_path)
    time.sleep(8)

signal.signal(signal.SIGALRM, timeout_handler)

signal.alarm(30)
if os.environ.get('REQUEST_METHOD', '') != 'POST':
    print()
    print('<html>')
    print('<head>')
    print('<script type="text/javascript">')
    print('window.location.href = "/cgi-bin/upload.php";')
    print('</script>')
    print('</head>')
    print('<body>')
    print('</body>')
    print('</html>')
    sys.exit()

env_max_body = os.environ.get('MAX_BODY', '0')
max_body = int(env_max_body)

cur_size = 0
uploaded_files = []

upload_dir = './cgi-bin/uploaded'
if not os.path.exists(upload_dir):
    os.makedirs(upload_dir)

random = str(uuid.uuid4())[:8]

filename = os.path.join(upload_dir, random + '_uploaded_file' + os.environ.get('FILE_EXT', ''))
uploaded_files.append(filename)
#
chunk_size = 1024 * 1024
with open(filename, 'wb') as file:
    while True:
        data_chunk = sys.stdin.buffer.read(chunk_size)
        if not data_chunk:
            break
        file.write(data_chunk)
#
os.rename(os.environ.get('UP_FILE', ''), filename)

if os.path.exists(filename):
    cur_size += os.path.getsize(filename)

if cur_size > max_body:
    if os.path.exists(filename):
        os.remove(filename)
    print("413")
signal.alarm(0)
    