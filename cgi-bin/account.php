<?php
    session_start();
    $ss_file_read = @fopen(".stored_session.txt", "r");
    if (!$ss_file_read)
    {
        echo "<script type=\"text/javascript\"> 
                window.location.assign(\"login.php\")
            </script>";
    }
    else
    {
        $_SESSION["username"] = @fgets($ss_file_read);
        $_SESSION["passwd"] = @fgets($ss_file_read);
        $_SESSION["sid"] = @fgets($ss_file_read);
        fclose($ss_file_read);
    }
    if (isset($_GET["logout"]))
    {
        session_unset();
        session_destroy();
        unlink(".stored_session.txt");
        echo "<script type=\"text/javascript\"> 
                window.location.assign(\"login.php\")
            </script>";
    }
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@4.1.3/dist/css/bootstrap.min.css" integrity="sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO" crossorigin="anonymous">
    <title>Account</title>
    <style>
        @import url(https://fonts.googleapis.com/earlyaccess/amiri.css);
        body{
            font-family: 'Amiri', serif;
            margin: 50px;
        }
    </style>
</head>
<body>
    <h2><center>Welcome <?php if (isset($_SESSION["username"])) echo $_SESSION["username"] ?> to your account</center></h2>
    <br>
    <form action="" method="get">
        <h5><center>Want to logout ? <input type="submit" class="btn btn-outline-danger" value="LOGOUT" name="logout"></center></h5>
    </form>
</body>
</html>