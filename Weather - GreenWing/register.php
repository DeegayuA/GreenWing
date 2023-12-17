<?php
require 'config.php';
define('PEPPER', 'GreeWingPepper');
echo "register.php";
// Create connection
$conn = new mysqli(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $username = $conn->real_escape_string($_POST['username']);
    $email = $conn->real_escape_string($_POST['email']);
    $password = $conn->real_escape_string($_POST['password']);

    // Peppering and hashing the password
    $passwordPeppered = hash_hmac('sha256', $password, PEPPER);
    $passwordHash = password_hash($passwordPeppered, PASSWORD_DEFAULT);

    // Prepare and bind
    $stmt = $conn->prepare("INSERT INTO tbl_users (username, email, password) VALUES (?, ?, ?)");
    $stmt->bind_param("sss", $username, $email, $passwordHash);

    // Execute and check if successful
    if ($stmt->execute()) {
        echo "New record created successfully";
    } else {
        echo "Error: " . $stmt->error;
    }

    $stmt->close();
}

$conn->close();
?>
