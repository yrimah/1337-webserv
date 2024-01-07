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
        .form-control{
            padding: 3.3px;
        }
    </style>
</head>
<body>
    <h1>UPLOAD FILES WITH PYTHON</h1>
    <br>
    <form method="POST" action="/cgi-bin/multi_upload.py" enctype="multipart/form-data">
        File 1 <input type="file" class="form-control" name="file_uploaded1" required>
        <br>
        File 2 <input type="file" class="form-control" name="file_uploaded2" required>
        <br>
        <button type="submit" class="btn btn-outline-info" name="Upload">Upload</button>
    </form>
    <br>
    <form method="" action="/upload.html" e>
        <h3 class="text-secondary font-weight-bold">Want to Upload files without using cgi ? <button type="submit" class="btn btn-outline-secondary">Go</button></h3>
    </form>
</body>
</html>