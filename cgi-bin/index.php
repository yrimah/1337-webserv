<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@4.1.3/dist/css/bootstrap.min.css" integrity="sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO" crossorigin="anonymous">
    <title>FORM</title>
    <style>
        @import url(https://fonts.googleapis.com/earlyaccess/amiri.css);
        body{
            font-family: 'Amiri', serif;
            margin: 50px
        }
    </style>
</head>
<body>
    <h1>FORM WITH PYTHON CGI</h1>
    <br>
    <form method="GET" action="/cgi-bin/form.py">
        First name <input type="text" class="form-control" name="f_name">
        <br>
        Last name <input type="text" class="form-control" name="l_name">
        <br>
        <button type="submit" class="btn btn-outline-success" name="print">Submit</button>
    </form>
</body>
</html>