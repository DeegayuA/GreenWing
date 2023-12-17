<?php
require 'config.php';
// Create a new database connection
$conn = new mysqli(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME);

// Check the connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Retrieve data from POST request
$username = $_POST['username'];
$password = $_POST['password'];
// $email = $_POST['email']; // If you're using email to login

// Peppering the password
$pepper = 'GreeWingPepper';
$passwordPeppered = hash_hmac('sha256', $password, $pepper);

// Prepare the SQL statement
$stmt = $conn->prepare("SELECT * FROM tbl_users WHERE username = ?");
$stmt->bind_param("s", $username);
$stmt->execute();
$result = $stmt->get_result();

// Check if user exists
if ($user = $result->fetch_assoc()) {
    // Verify the password
    if (password_verify($passwordPeppered, $user['password'])) {
        // Password is correct, send a success response
        echo json_encode(['success' => true]);
    } else {
        // Password is incorrect, send an error message
        echo json_encode(['success' => false, 'message' => 'Invalid credentials. Please try again.']);
    }
} else {
    // User doesn't exist
    echo json_encode(['success' => false, 'message' => 'User does not exist.']);
}

$stmt->close();
$conn->close();
?>

<!DOCTYPE html>
<html lang="en" >
<head>
  <meta charset="UTF-8">
  <title>login data</title>
  <meta http-equiv="refresh" content="4;url=home.html" />
  <link rel="icon" href="data:image/svg+xml,<svg xmlns=%22http://www.w3.org/2000/svg%22 viewBox=%220 0 100 100%22><text y=%22.9em%22 font-size=%2290%22>🌱</text></svg>">
</head>
<body>
</body>
</html>