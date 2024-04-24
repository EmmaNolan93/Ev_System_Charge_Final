<?php
session_start(); // Start the session

include 'database.php';

if(isset($_POST['username'])) {
    // Get the username from the POST request
    $username = $_POST['username'];
    // Generate a random token
    $token = bin2hex(random_bytes(32)); // Generate a 32-byte hexadecimal token

    // Calculate expiry time (e.g., 1 hour from now)
    $expiryTime = time() + (1 * 60 * 60); // 1 hour from now

    // Store the token in a session variable
    $_SESSION['reset_token'] = $token;

    // Prepare SQL statement to select user with given username
    $sql = "INSERT INTO password_reset_tokens (username, token, expiry_time) VALUES ('$username', '$token', '$expiryTime')";
    $result = $conn->query($sql);

    // Check if the SQL query was successful
    if ($result === TRUE) {
        // Redirect to send_reset_email.php
        header("Location: send_reset_email.php");
        exit();
    } else {
        echo "Error: Unable to generate token";
    }
} else {
    echo "Error: Username not provided";
}
?>

