<?php
require 'config.php';
error_reporting(E_ALL);
ini_set('display_errors', 1);
// Create connection
$conn = new mysqli(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

header('Content-Type: application/json');

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $username = $conn->real_escape_string($_POST['username']);
    $password = $conn->real_escape_string($_POST['password']);

    // Prepare and bind
    $stmt = $conn->prepare("SELECT password FROM tbl_users WHERE username = ?");
    $stmt->bind_param("s", $username);
    $stmt->execute();
    $result = $stmt->get_result();

    if ($result->num_rows > 0) {
        $row = $result->fetch_assoc();
        $hashedPassword = $row['password'];

        // Verifying the password
        if (password_verify($password, $hashedPassword)) {
            echo json_encode(['success' => true]);
            

        } else {
            echo json_encode(['success' => false, 'message' => 'Invalid credentials. Please try again.']);
        }
    } else {
        echo json_encode(['success' => false, 'message' => 'User does not exist.']);
    }

    $stmt->close();
}

$conn->close();
?>
