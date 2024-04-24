<?php
// Include your database connection script
include 'database.php';

// Retrieve username from the form
$username = $_POST['username'];

// Prepare SQL statement to select user with given username
    $sql = "SELECT email FROM users WHERE username = '$username'";
    $result = $conn->query($sql);

if ($result->num_rows === 1) {
    // Username exists, retrieve the email
    $row = $result->fetch_assoc();
    $email = $row['email'];

    // Generate a password reset token and store it in the database
   include 'generate_token.php'; // Include the generate_token.php script
    
} else {
    // Username does not exist, handle accordingly (e.g., display error message)
    header('Location:  forgotten_password.php?error=username_not_found');
    exit();
}
?>

