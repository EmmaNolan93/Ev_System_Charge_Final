<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Register - EV Charging System</title>
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/foundation/6.3.1/css/foundation.min.css">
    <style>
        body {
            font-family: 'Helvetica', sans-serif;
            background-color: #f4f4f4;
        }

        .register-container {
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

        .register-button {
            width: 100%;
        }

        .login-link {
            display: block;
            text-align: center;
            margin-top: 10px; /* Adjust the margin for spacing */
        }
    </style>
</head>
<body>
<div class="register-container">
    <h1>Register - EV Charging System</h1>

    <form method="post" action="process_reg.php" onsubmit="return validateForm()">
        <label for="username">Username:</label>
        <input type="text" id="username" name="username" placeholder="Enter your username" autofocus="" autocomplete="off" autocapitalize="off" autocorrect="off" required>

        <label for="email">Email:</label>
        <input type="email" id="email" name="email" placeholder="Enter your email address" autofocus="" autocomplete="off" autocapitalize="off" autocorrect="off" required>

        <label for="password">Password:</label>
        <input type="password" id="password" name="password" placeholder="Enter your password" autofocus="" autocomplete="off" autocapitalize="off" autocorrect="off" required pattern="(?=.*\d)(?=.*[a-z])(?=.*[A-Z]).{8,}" title="Password must contain at least one number, one lowercase and one uppercase letter, and at least 8 or more characters">

        <div class="button-group">
            <button class="success button register-button" type="submit">Register</button>
        </div>
        <p class="login-link">Already have an account? <a href="login.php">Login</a></p>
    </form>
</div>

<script src="https://cdn.jsdelivr.net/foundation/6.3.1/js/vendor/jquery.js"></script>
<script src="https://cdn.jsdelivr.net/foundation/6.3.1/js/vendor/what-input.js"></script>
<script src="https://cdn.jsdelivr.net/foundation/6.3.1/js/foundation.min.js"></script>

<script>
    $(document).foundation();

    function validateForm() {
        var username = document.getElementById('username').value;
        var email = document.getElementById('email').value;
        var password = document.getElementById('password').value;

        // Check if all fields are filled
        if (username === "" || email === "" || password === "") {
            alert("Please fill in all fields.");
            return false;
        }

        // Email format validation
        var emailPattern = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
        if (!emailPattern.test(email)) {
            alert("Please enter a valid email address.");
            return false;
        }

        // Password format validation (already handled by HTML5 pattern attribute)
        
        return true;
    }
</script>
</body>
</html>
