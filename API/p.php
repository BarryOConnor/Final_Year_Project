<?php
/*----------

Response Codes:

001 - OK.  Sensor is authorised and data has been saved
010 - FAIL.  Sensor is authorised but data has not been saved
100 - FAIL. Sensor ID is not on the authorised list

---------- */



$sensor_id = htmlspecialchars($_GET["d"]);
$internal = htmlspecialchars($_GET["i"]);
$external = htmlspecialchars($_GET["e"]);
$humidity = htmlspecialchars($_GET["h"]);
$pressure = htmlspecialchars($_GET["p"]);
$rain = htmlspecialchars($_GET["r"]);
$wind = htmlspecialchars($_GET["w"]);

$host = "10.169.0.207";
$user = "barryoco_weather";
$pass = "k1nd3r3gg";
$db = "barryoco_weather";

$dsn = "mysql:host=$host;dbname=$db;";
$options = [
    PDO::ATTR_ERRMODE            => PDO::ERRMODE_EXCEPTION,
    PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
    PDO::ATTR_EMULATE_PREPARES   => false,
];
try {
     $pdo = new PDO($dsn, $user, $pass, $options);
} catch (\PDOException $e) {
     throw new \PDOException($e->getMessage(), (int)$e->getCode());
}

$auth_stmt = $pdo->prepare("SELECT COUNT(id) FROM authorised WHERE id = ?");
$auth_stmt->execute([$sensor_id]);
$count = $auth_stmt->fetchColumn();

if ($count > 0) {
     $stmt = $pdo->prepare("INSERT INTO weather_data (sensor_id, recorded_at, temp_internal, temp_external, humidity, pressure, rain, windspeed) VALUES (?, DEFAULT, ?, ?, ?, ?, ?, ?)");
     $result = $stmt->execute([$sensor_id, $internal, $external, $humidity, $pressure, $rain, $wind]);
     if ($result) {
          echo "001";
     } else {
          echo "010";
     }
} else {
     // 
     echo "100";
}
$conn = null;
?>