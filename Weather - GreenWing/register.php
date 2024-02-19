<?php

require 'config.php'; // Includes database connection settings

// Temporary error reporting for debugging (remove in production)
// ini_set('display_errors', 1);
// ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

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

    // Hash the password using PHP's recommended method
    $passwordHash = password_hash($password, PASSWORD_DEFAULT);

    // File upload configuration
    $targetDir = "uploads/"; 
    $allowTypes = array('jpg', 'png', 'jpeg', 'gif'); 

    // Handle file upload if a file is present
    if (!empty($_FILES["profile_picture"]["name"])) { 
      $fileName = basename($_FILES["profile_picture"]["name"]);
      $fileNameParts = explode(".", $fileName); // Separate name and extension
      $newFileName = $fileNameParts[0] . "_" . time() . "." . $fileNameParts[1]; 
      $targetFilePath = $targetDir . $newFileName;
        $fileType = pathinfo($targetFilePath, PATHINFO_EXTENSION);

        // Ensure file type is allowed
        if (in_array($fileType, $allowTypes)) {
            // Attempt to move the uploaded file (permissions crucial here)
            if (move_uploaded_file($_FILES["profile_picture"]["tmp_name"], $targetFilePath)) {
                // File uploaded successfully!
                header('Location: ./login.html');
                exit;
            } else {
                echo "Error uploading file. Check your PHP configuration and the permissions of the 'uploads/' folder.";
                exit; // Stop further execution if file upload fails
            }
        } else {
            echo 'Invalid file format. Please upload a JPG, JPEG, PNG, or GIF file.';
            exit; // Stop further execution if invalid file type
        }
    }

    // Prepare SQL query (use a prepared statement for security)
    $stmt = $conn->prepare("INSERT INTO tbl_users (username, email, password, profile_picture) VALUES (?, ?, ?, ?)");

    // Bind parameters (IMPORTANT for preventing SQL injection)
    $stmt->bind_param("ssss", $username, $email, $passwordHash, $targetFilePath);

     // Execute statement
     if ($stmt->execute()) {
      // Redirect to login page upon success or add a success message here
      header('Location: login.html');
      exit;
  } else {
      echo "Error: " . $stmt->error; // No need to print raw SQL for security
  }

    $stmt->close();
    $conn->close(); 
};
?>




<!DOCTYPE html>
<html lang="en" >
<head>
  <meta charset="UTF-8">
  <title>Register data</title>
    <link rel="stylesheet" href="./loader.css">

  <meta http-equiv="refresh" content="1;url=login.html"/>
  <link rel="icon" href="data:image/svg+xml,<svg xmlns=%22http://www.w3.org/2000/svg%22 viewBox=%220 0 100 100%22><text y=%22.9em%22 font-size=%2290%22>🌱</text></svg>">
</head>
<body>
</body>
</html>