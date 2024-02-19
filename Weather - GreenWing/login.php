<?php
require 'config.php';
error_reporting(E_ALL);
ini_set('display_errors', 1);

// Start the session
session_start();

// Create connection
$conn = new mysqli(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// No need for setting Content-Type header here

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    // Check if username and password are set
    if (isset($_POST['username']) && isset($_POST['password'])) {
        $username = $conn->real_escape_string($_POST['username']);
        $password = $conn->real_escape_string($_POST['password']);

        // Prepare and bind
        $stmt = $conn->prepare("SELECT id, password FROM tbl_users WHERE username = ?");
        $stmt->bind_param("s", $username);
        $stmt->execute();
        $result = $stmt->get_result();

        if ($result->num_rows > 0) {
            $row = $result->fetch_assoc();
            $hashedPassword = $row['password'];
            $userId = $row['id'];

            // Verifying the password
            if (password_verify($password, $hashedPassword)) {
                // Store user ID in the session for access on other pages
                $_SESSION['userId'] = $userId;
                
                // Echoing the session ID directly, no need for JSON encoding
                echo $_SESSION['userId'];
            } else {
                // Echoing error message directly, no need for JSON encoding
                echo 'Invalid credentials. Please try again.';
            }
        } else {
            // Echoing error message directly, no need for JSON encoding
            echo 'User does not exist.';
        }
        $stmt->close();
    } else {
        // Echoing error message directly, no need for JSON encoding
        // echo 'Username and/or password not provided.';
        if (isset($_SESSION['userId'])) {
            echo $_SESSION['userId'];
        } else {
            echo "Session UserID not set.";
        }
    }
}

$conn->close();
?>
