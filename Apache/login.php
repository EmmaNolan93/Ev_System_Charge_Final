<!-- login.php -->
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Login - EV Charging System</title>
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/foundation/6.3.1/css/foundation.min.css">
    <style>
        body {
            font-family: 'Helvetica', sans-serif;
            background-color: #f4f4f4;
        }

        .login-container {
            max-width: 400px;
            margin: 100px auto;
            padding: 20px;
            background-color: #fff;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            border-radius: 5px;
        }

        h1 {
            color: #333;
            text-align: center;
        }

        label {
            font-weight: bold;
        }

        input {
            margin-bottom: 10px;
            width: 100%;
            padding: 8px;
            box-sizing: border-box;
        }

        .button-group {
            text-align: center;
        }

        .login-button, .register-button {
            width: 48%;
        }

        .signup-link {
            display: block;
            text-align: center;
            margin-top: 10px; /* Adjust the margin for spacing */
        }
	.password{
	   display: block;
 	   text-align: center;
	   margin-top: -4px;
	   font-size: x-small;	
	}
    </style>
</head>
<body>

<div class="login-container">
    <h1>Login - EV Charging System</h1>
	<?php
    // Check if error message is set in the URL
    if (isset($_GET['message'])) {
        $message = $_GET['message'];
        echo "<p style='color: red;'>$message</p>";
    }
    ?>

    <form method="post" action="process_login.php">
        <label for="username">Username:</label>
        <input type="text" id="username" name="username" placeholder="username" autofocus="" autocomplete="off" autocapitalize="off" autocorrect="off"  required>

        <label for="password">Password:</label>
        <input type="password" id="password" name="password" placeholder="Password" autofocus="" autocomplete="off" autocapitalize="off" autocorrect="off"  required>
	<p class="password"><a href="forgotten_password.php">Forgot password?</a></p>

        <div class="button-group">
            <button class="success button login-button" type="submit">Login</button>
        </div>
        <p class="signup-link">Don't have an account? <a href="reg.php">Sign up</a></p>
    </form>
</div>

<script src="https://cdn.jsdelivr.net/foundation/6.3.1/js/vendor/jquery.js"></script>
<script src="https://cdn.jsdelivr.net/foundation/6.3.1/js/vendor/what-input.js"></script>
<script src="https://cdn.jsdelivr.net/foundation/6.3.1/js/foundation.min.js"></script>

<script>
    $(document).foundation();
</script>

</body>
</html>

