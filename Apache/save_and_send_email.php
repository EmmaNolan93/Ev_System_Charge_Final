<?php
session_start();
require 'vendor/autoload.php';
echo "Autoload included successfully!";
// Create a new PHPMailer instance
//use phpmailer\phpmailer\src\PHPMailer;
use PHPMailer\PHPMailer\PHPMailer;
use PHPMailer\PHPMailer\Exception;
use PHPMailer\PHPMailer\SMTP;
include 'database.php';
error_log("yes i reach here");
if (isset($_GET['username'])) {
    $username = $_GET['username'];
    // Add a debug statement to confirm that the charging level was received
    error_log("Received charging level in emaill: $username");
    error_log("Email: ". $_GET['username']);
} else {
    // If the charging level was not provided, set it to a default value
    $username = 'Emma';
    error_log("user not provided, using default value: $username");
}

// Accessing sessionID parameter from the URL
if (isset($_GET['sessionID'])) {
    $sessionID = $_GET['sessionID'];
    // Add a debug statement to confirm that the sessionID was received
    error_log("Received sessionID in email: $sessionID");
} else {
    // Handle if sessionID is not provided
    error_log("user not provided, using default value: $sessionID");

}

// Accessing chargingLevel parameter from the URL
if (isset($_GET['chargingLevel'])) {
    $chargingLevel = $_GET['chargingLevel'];
    // Add a debug statement to confirm that the chargingLevel was received
    error_log("Received charging level in email: $chargingLevel");
} else {
    // Handle if chargingLevel is not provided
  $chargingLevel = 100;
    error_log("user not provided, using default value: $chargingLevel");

}

// Prepare SQL statement to select user with given username
$sql = "SELECT * FROM users WHERE username = '$username'";
$result = $conn->query($sql);
    if ($result->num_rows === 1) {
        // Username exists, retrieve the email
	$username = $row['username'];
        $row = $result->fetch_assoc();
        $email = $row['email'];
        echo "next step";
        // Compose the email message
        $subject = "Password Reset Request";
        $message = "Hello,\r\n\r\n";
        $message .= "Your charging is completed.\r\n";
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
        } else {
            // Email sending failed
            echo "Email sending failed: " . $mail->ErrorInfo;
        }
    } 
$conn->close();
?>


