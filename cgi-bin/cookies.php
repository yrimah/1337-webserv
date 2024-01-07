<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@4.1.3/dist/css/bootstrap.min.css" integrity="sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO" crossorigin="anonymous">
    <title>Cooooooooookie with PHP</title>
    <style>
        @import url(https://fonts.googleapis.com/earlyaccess/amiri.css);
        body{
            font-family: 'Amiri', serif;
            margin: 50px;
        }
    </style>
</head>
<body>
    <h1>Get cookie</h1>
    <form method="GET" action="/cgi-bin/get_cookie.php">
        Cookie name <input class="form-control type="text" name="cookie">
        <br>
        <input type="submit" class="btn btn-outline-secondary" value="Get cookie" name="get">
    </form>
    <br>
    <br>
    <h1>Set cookie</h1>
    <form method="GET" action="/cgi-bin/set_cookie.php">
        Cookie name <input class="form-control type="text" name="name">
        <br>
        <br>
        Cookie value <input class="form-control type="text" name="value">
        <br>
        <input type="submit" class="btn btn-outline-secondary" value="Set cookie" name="set">
    </form>
</body>
</html>