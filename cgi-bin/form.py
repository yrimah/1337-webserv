import cgi, cgitb 
import re
import requests

form = cgi.FieldStorage() 

first_name = form.getvalue('f_name')
last_name  = form.getvalue('l_name')

print ("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n")

# while (1):
#     pass
if not first_name or not last_name:
    print('<html>')
    print('<head>')
    print('<script type="text/javascript">')
    print('window.location.href = "/cgi-bin/index.php";')
    print('</script>')
    print('</head>')
    print('<body>')
    print('</body>')
    print('</html>')
else:
    print ("<html>")
    print ("<head>")
    print ("<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.1.3/dist/css/bootstrap.min.css\" integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\" crossorigin=\"anonymous\">")
    print ("<style>")
    print ("@import url(https://fonts.googleapis.com/earlyaccess/amiri.css);")
    print ("body{")
    print ("font-family: 'Amiri', serif;font-size: 35px;")
    print ("margin: 15px;")
    print ("}")
    print ("</style>")
    print ("<title>FORM WITH PYTHON</title>")
    print ("</head>")
    print ("<body>")
    print ("<h2><center>Welcome %s %s to pyhton cgi</center></h2>" % (first_name, last_name))
    print ("<center><button onclick=\"window.location.href='/cgi-bin/index.php'\" class=\"btn btn-outline-secondary\">Go back</button><center>")
    print ("</body>")
    print ("</html>")