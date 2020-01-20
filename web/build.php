<?php

//putenv ( string $setting ) : bool

$id=uniqid();
$sourceDir = "/tmp/build-$id";
mkdir($sourceDir);

$return = shell_exec("cp -rv /var/lib/tinysensor/tinySensor $sourceDir");
$sourceDir .= "/tinySensor/";

chdir($sourceDir);

$update_out =  shell_exec("git pull --rebase");
$update_out .= shell_exec("git submodule update -f");

$buildDir = $sourceDir . "/build";
mkdir($buildDir);
chdir($buildDir);

$config_out = shell_exec("$sourceDir/init_avr.sh");
$compilation_out = shell_exec("make -j4");

preg_replace("/(^[\r\n]*|[\r\n]+)[\s\t]*[\r\n]+/", "\n", $config_out);
preg_replace("/(^[\r\n]*|[\r\n]+)[\s\t]*[\r\n]+/", "\n", $compilation_out);

@mkdir("/var/www/html/data");

mkdir("/var/www/html/data/$id", 0777, true);
copy("/tmp/build-$id/tinySensor/build/tinySensor-attiny84a.hex", "/var/www/html/data/$id/tinySensor-attiny84a.hex");
?>
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>jQuery UI Accordion - Default functionality</title>
  <link rel="stylesheet" href="//code.jquery.com/ui/1.12.1/themes/base/jquery-ui.css">
  <link rel="stylesheet" href="/resources/demos/style.css">
  <script src="https://code.jquery.com/jquery-1.12.4.js"></script>
  <script src="https://code.jquery.com/ui/1.12.1/jquery-ui.js"></script>
  <script>
  $( function() {
    $( "#accordion" ).accordion({
      heightStyle: "content"
    });
  } );

  </script>
</head>
<body>

<div style="height: 300px">
<div id="accordion">


  <h3>Code update logs</h3>
  <div><p><pre><?=$update_out ?></pre></p></div>

  <h3>Configuration logs</h3>
  <div><p><pre><?=$config_out ?></pre></p></div>

  <h3>Compilation logs</h3>
  <div><pre><?=$compilation_out ?></pre></div>

  <h3>Produced firmware</h3>
  <div><p><a href="data/<?=$id?>/tinySensor-attiny84a.hex">tinySensor-attiny84a.hex</a></p></div>
</div>
</div>
</body>
</html>
