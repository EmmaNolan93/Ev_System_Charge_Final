<?php
session_start();
require 'vendor/autoload.php';
echo "Autoload included successfully!";
use PHPMailer\PHPMailer\PHPMailer;
use PHPMailer\PHPMailer\Exception;
use PHPMailer\PHPMailer\SMTP;
include 'database.php';
$username = $_POST['username'];
$token = $_SESSION['reset_token'];
// Prepare SQL statement to select user with given username
$sql = "SELECT username FROM password_reset_tokens WHERE token = '$token'";
$result = $conn->query($sql);
if ($result->num_rows === 1) {
    // Token exists, retrieve the username associated with it
    $row = $result->fetch_assoc();
    $username = $row['username'];

    // Check if the retrieved username exists in the users table
    $sql = "SELECT * FROM users WHERE username = '$username'";
    $result = $conn->query($sql);

    if ($result->num_rows === 1) {
        // Username exists, retrieve the email
        $row = $result->fetch_assoc();
        $email = $row['email'];

        // Generate the reset link
        $resetLink = "http://169.254.8.149/change_password.php?token=$token"; 
        echo "next step";
        // Compose the email message
        $subject = "Password Reset Request";
        $message = "Hello,\r\n\r\n";
        $message .= "You have requested to reset your password.\r\n";
        $message .= "Please click on the following link to reset your password:\r\n";
        $message .= "$resetLink\r\n\r\n";
        $message .= "If you did not request this, please ignore this email.\r\n";
        echo "Creating PHPMailer instance...<br>";
        try {
            $mail = new PHPMailer();
            echo "PHPMailer instance created successfully!<br>";
        } catch (Exception $e) {
            echo "PHPMailer Exception: " . $e->getMessage();
        } catch (Throwable $t) {
            echo "Throwable: " . $t->getMessage();
        }
        // Create a new PHPMailer instance
        // $mail = new PHPMailer\PHPMailer\PHPMailer();
        echo "mail";
        // Configure SMTP settings
        $mail->isSMTP();
        $mail->Host = 'smtp.gmail.com'; // SMTP server
        $mail->SMTPAuth = true;
        $mail->Username = ''; 
        $mail->Password = ''; 
        $mail->SMTPSecure = 'tls';
        $mail->Port = 587;
        echo "sending mila";
        // Set sender and recipient
        $mail->setFrom('emmanolan2019@gmail.com', 'Emmanolan'); // Sender email and name
        $mail->addAddress($email); // Add recipient email address

        // Set email subject and body
        $mail->Subject = $subject;
        $mail->Body = $message;

        // Send the email
        if ($mail->send()) {
            // Email sent successfully
            echo "Email sent successfully!";
            header('Location: login.php');
            exit;
        } else {
            // Email sending failed
            echo "Email sending failed: " . $mail->ErrorInfo;
        }
    } else {
        // Username not found, handle accordingly (e.g., display error message)
        // Username not found, redirect to forgotten_password.php with message
        header('Location: forgotten_password.php?message=Username not found');
        exit;
    }
} else {
// Debugging: Output the token retrieved from the session
echo "Token from session: $token<br>";

// Debugging: Output the SQL query to select the token
echo "SQL Query: $sql<br>";

// Debugging: Output the number of rows returned by the SQL query
echo "Number of rows: " . $result->num_rows . "<br>";
    // Token not found, redirect to forgotten_password.php with message
//    header('Location: forgotten_password.php?message=Token not found $token');
  //  exit;
}

// Close the database connection
$stmt->close();
$conn->close();
?>

