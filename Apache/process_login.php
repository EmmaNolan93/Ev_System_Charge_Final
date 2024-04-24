<?php
include 'database.php';
session_start();

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $username = $_POST['username'];
    $password = $_POST['password']; 

    // Prepare SQL statement to select user with given username
    $sql = "SELECT * FROM users WHERE username = '$username'";
    $result = $conn->query($sql);

    if ($result === FALSE) {
        echo "Error executing query: " . $conn->error;
    } else {
        // Check if user exists
        if ($result->num_rows > 0) {
            $row = $result->fetch_assoc();
            
            // Verify if password matches
            if (password_verify($password, $row['password'])) {
                // Successful login
                $_SESSION['username'] = $row['username'];
                header('Location: index.php');
                exit();
            } else {
                // Incorrect password
                $message = "Incorrect password!";
            }
        } else {
            // User does not exist
            $message = "User does not exist!";
        }
    }
}

// Redirect to login page with error message
header('Location: login.php?message=' . urlencode($message));
exit();
?>

