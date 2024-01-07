<?php
    session_start();
    $ac_or_login = @fopen(".stored_session.txt", "r");
    if ($ac_or_login)
    {
        fclose($ac_or_login);
        echo "<script type=\"text/javascript\"> 
                window.location.assign(\"account.php\")
            </script>";
    }
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@4.1.3/dist/css/bootstrap.min.css" integrity="sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO" crossorigin="anonymous">
    <title>LOGIN PAGE</title>
    <style>
        @import url(https://fonts.googleapis.com/earlyaccess/amiri.css);
        body{
            font-family: 'Amiri', serif;
            margin: 50px;
        }
    </style>
</head>
<body>
    <form method="GET" action="">
        Username <input type="text" class="form-control" name="username">
        <br>
        Password <input type="password" class="form-control" name="password">
        <br>
        <input type="submit" class="btn btn-outline-secondary" value="LOGIN" name="login">
    </form>
    <?php
        if (isset($_GET['login']))
        {
            if (isset($_GET["username"]) && isset($_GET["password"]) && !empty($_GET["password"]) && !empty($_GET["username"]))
            {
                $_SESSION["username"] = $_GET["username"];
                $_SESSION["password"] = hash('sha256', $_GET["password"]);
                $_SESSION["sid"] = session_id();

                $ss_file = fopen(".stored_session.txt", "w");
                $values = $_SESSION["username"] . "\n" . $_SESSION["password"] . "\n" . $_SESSION["sid"];
                fwrite($ss_file, $values);
                fclose($ss_file);
                echo "<script type=\"text/javascript\"> 
                        window.location.assign(\"account.php\")
                    </script>";
            }
            else
                echo "<br><h2 class=\"text-danger\">Invalid username/password</h2>";
        }
    ?>
</body>
</html>