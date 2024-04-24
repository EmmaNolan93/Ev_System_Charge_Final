<?php
include 'database.php'; 
session_start();

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Retrieve registration form data
    $username = $_POST['username'];
    $email = $_POST['email'];
    $password = $_POST['password'];

    // Hash the password
    $hashedPassword = password_hash($password, PASSWORD_DEFAULT);

    // Prepare SQL statement to check if the username already exists
    $checkUsernameQuery = "SELECT * FROM users WHERE username = '$username'";
    $checkUsernameResult = mysqli_query($conn, $checkUsernameQuery);

    if (!$checkUsernameResult) {
        echo "Error executing query: " . mysqli_error($conn);
        exit(); // Exit if there's an error
    }

    if (mysqli_num_rows($checkUsernameResult) > 0) {
        header('Location: reg.php?error=username_exists');
        exit();
    }

    // Insert the user data into the database
    $insertQuery = "INSERT INTO users (username, email, password) VALUES ('$username', '$email', '$hashedPassword')";
    $insertResult = mysqli_query($conn, $insertQuery);

    if (!$insertResult) {
        echo "Error executing query: " . mysqli_error($conn);
        exit(); // Exit if there's an error
    }

    // Registration successful
    $_SESSION['username'] = $username;
    header('Location: index.php');
    exit();
} else {
    // Redirect to the registration page if accessed directly without submitting the form
    header('Location: reg.php');
    exit();
}
?>

