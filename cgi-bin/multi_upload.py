# #!/usr/bin/python3

import cgi
import os
import sys
import uuid
import signal
import time

def timeout_handler(signum, frame):
    for file_path in uploaded_files:
        if os.path.exists(file_path):
            print("timeout")
            os.remove(file_path)
    time.sleep(8)
    # sys.exit()

signal.signal(signal.SIGALRM, timeout_handler)

# try:
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

form = cgi.FieldStorage()

signal.alarm(30)
if "file_uploaded1" in form:
    fileitem1 = form["file_uploaded1"]

    if fileitem1.filename:
        upload_dir1 = './cgi-bin/uploaded'
        os.makedirs(upload_dir1, exist_ok=True)
        random1 = str(uuid.uuid4())[:8]
        filepath1 = os.path.join(upload_dir1, os.path.basename(random1 + '_' + fileitem1.filename))
        uploaded_files.append(filepath1)
        
        with open(filepath1, 'wb') as f:
            while True:
                chunk = fileitem1.file.read(1024)
                if not chunk:
                    break
                f.write(chunk)
        if os.path.exists(filepath1):
            cur_size += os.path.getsize(filepath1)
if cur_size > max_body:
    if os.path.exists(filepath1):
        os.remove(filepath1)
    print("413")
if "file_uploaded2" in form:
    fileitem2 = form["file_uploaded2"]

    if fileitem2.filename:
        upload_dir2 = './cgi-bin/uploaded'
        os.makedirs(upload_dir2, exist_ok=True)
        random2 = str(uuid.uuid4())[:8]
        filepath2 = os.path.join(upload_dir2, os.path.basename(random2 + '_' + fileitem2.filename))
        uploaded_files.append(filepath2)

        with open(filepath2, 'wb') as f:
            while True:
                chunk = fileitem2.file.read(1024)
                if not chunk:
                    break
                f.write(chunk)
        if os.path.exists(filepath2):
            cur_size += os.path.getsize(filepath2)
    
    if cur_size > max_body:
        if os.path.exists(filepath1):
            os.remove(filepath1)
        if os.path.exists(filepath2):
            os.remove(filepath2)
        print("413")
    else:
        print("OK") 
else:
    print("OK")
os.remove(os.environ.get('UP_FILE', ''))
signal.alarm(0)