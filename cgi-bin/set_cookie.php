<?php
    if (isset($_GET["set"])
        && isset($_GET["name"]) && !empty($_GET["name"])
        && isset($_GET["value"]) && !empty($_GET["value"]))
    {
        setcookie($_GET["name"], $_GET["value"], time() + (60 * 5));
        echo '<link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@4.1.3/dist/css/bootstrap.min.css" integrity="sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO" crossorigin="anonymous">
        <title>Set Cooooooooookie with PHP</title>';
        echo "<style>@import url(https://fonts.googleapis.com/earlyaccess/amiri.css);
        body{
            font-family: 'Amiri', serif;
            margin: 50px;
        }</style>";
        echo "<h2><center>Cookie set successfully</center></h2><br>";
        echo "<center><a href='cookies.php' class='btn btn-outline-secondary'>GO BACK</a></center>";
    }
    else
        echo "<script type=\"text/javascript\"> 
                window.location.assign(\"cookies.php\")
            </script>";
?>