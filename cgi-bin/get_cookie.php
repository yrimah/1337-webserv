<?php
    if (isset($_GET["get"]) && isset($_GET["cookie"]) && !empty($_GET["cookie"]))
    {
        echo '<link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@4.1.3/dist/css/bootstrap.min.css" integrity="sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO" crossorigin="anonymous">
        <title>Get Cooooooooookie with PHP</title>';
        echo "<style>@import url(https://fonts.googleapis.com/earlyaccess/amiri.css);
            body{
                font-family: 'Amiri', serif;
                margin: 50px;
            }</style>";
        if (!isset($_COOKIE[$_GET["cookie"]]))
            echo "<h1><center>Cookie not found</center></h1><br>";
        else
            echo "<h1><center>The value of " . $_GET["cookie"] ." cookie is: " . $_COOKIE[$_GET["cookie"]] . "</center></h1><br>";
        echo "<center><a href='cookies.php' class='btn btn-outline-secondary'>GO BACK</a></center>";
    }
    else
        echo "<script type=\"text/javascript\"> 
            window.location.assign(\"cookies.php\")
        </script>";
?>