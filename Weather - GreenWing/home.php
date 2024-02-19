<?php
require 'config.php';

// Retrieve the 'id' parameter from the GET request
$id = $_GET['id'];

$sql = "SELECT username, profile_picture FROM tbl_users WHERE id = $id";
$result = $db->query($sql);
if (!$result) {
  { echo "Error: " . $sql . "<br>" . $db->error; }
}

$row = $result->fetch_assoc();

$rows = $result -> fetch_all(MYSQLI_ASSOC);
// print_r($row);

header('Content-Type: application/json');
echo json_encode($row);
?>
