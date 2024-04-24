<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Forgot Password - EV Charging System</title>
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/foundation/6.3.1/css/foundation.min.css">
    <style>
        body {
            font-family: 'Helvetica', sans-serif;
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
        }

        .container {
            max-width: 400px;
            padding: 20px;
            background-color: rgba(255, 255, 255, 0.8); 
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            border-radius: 5px;
            height: 250px;
        }

        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 20px;
        }

        label {
            display: block;
            font-weight: bold;
        }

        input[type="username"] {
            width: 100%;
            padding: 8px;
            box-sizing: border-box;
            margin-bottom: 10px;
            border: 1px solid #ccc; /* Add a border */
            border-radius: 5px; /* Add border radius */
            background-color: #f9f9f9; /* Light gray background */
        }

        button[type="submit"], button[type="button"] {
            width: 100%;
            padding: 10px;
            box-sizing: border-box;
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            margin-top: 10px;
        }

        button[type="submit"]:hover, button[type="button"]:hover {
            background-color: #45a049;
        }
    </style>
</head>
<body>
<div class="container">
    <h1>Forgot Password</h1>
	 <?php
        // Check if a message is present in the URL
        if (isset($_GET['message'])) {
            $message = $_GET['message'];
            echo "<p>$message</p>";
        }
        ?>

    <form id="forgotPasswordForm" action="forgot_password.php" method="post" onsubmit="return validateForm()">
        <label for="username">Username:</label>
        <input type="text" id="username" name="username" placeholder="Enter your username" required>
        <br>
        <button type="submit">Reset Password</button>
    </form>

    <form action="login.php" method="get">
        <button type="submit">Back to Login</button>
    </form>
</div>

<script>
function validateForm() {
    var username = document.getElementById("username").value;
    if (username == "") {
        alert("Please enter your username.");
        return false;
    }
    return true;
}
</script>
</body>
</html>

