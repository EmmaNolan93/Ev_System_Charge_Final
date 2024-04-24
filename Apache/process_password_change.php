<?php
session_start(); // Start or resume the session

include 'database.php';

// Check if the form is submitted
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $newPassword = $_POST['new_password'];
    $confirmPassword = $_POST['confirm_password'];
    echo $newPassword;
    echo $confirmPassword;
    // Retrieve the username from the session
echo "just nefore tokem";
   // $token = $_SESSION['token'];
 $token = $_GET['token'];
    echo $token;
    $sql = "SELECT username FROM password_reset_tokens WHERE token = '$token'";
    $result = $conn->query($sql);
    if ($result->num_rows === 1) {
        $row = $result->fetch_assoc();
        $username = $row['username'];

        // Prepare SQL statement to select the user's password
        $sql = "SELECT password FROM users WHERE username = '$username'";
        $result = $conn->query($sql);

        if ($result->num_rows === 1) {
            // User found, retrieve the password
            $row = $result->fetch_assoc();

            // Verify if the new password and confirmation match
            if ($newPassword === $confirmPassword) {
                // Hash the new password
                $hashedNewPassword = password_hash($newPassword, PASSWORD_DEFAULT);

                // Update the user's password in the database
                $updateSql = "UPDATE users SET password = '$hashedNewPassword' WHERE username = '$username'";
                if ($conn->query($updateSql) === TRUE) {
                    // Password updated successfully
                    echo "Password updated successfully!";
		    // Redirect to login.php
    		    header("Location: login.php");
    		    exit; // Make sure to exit after redirection
                } else {
                    // Error updating password
                    echo "Error updating password: " . $conn->error;
                }
            } else {
                // New password and confirmation do not match
                echo "New password and confirmation do not match!";
            }
        } else {
            // User not found
            echo "User not found!";
        }
    }
    // Close the database connection
    $conn->close();
}
?>

