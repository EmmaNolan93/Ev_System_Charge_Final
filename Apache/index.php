<?php
session_start();
// Check if the user is logged in
if (!isset($_SESSION['username'])) {
    header('Location: login.php');
    exit();
}
$username = $_SESSION['username'];
// Logout logic
if (isset($_POST['logout'])) {
    // Unset all of the session variables
    $_SESSION = array();

    // Destroy the session
    session_destroy();

    // Redirect to login page
    header("Location: login.php");
    exit();
}
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>EV Charging System</title>
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/foundation/6.3.1/css/foundation.min.css">
    <style>
        body {
            font-family: 'Helvetica', sans-serif;
            background-color: #f4f4f4;
        }

        .container {
            max-width: 800px;
            margin: 20px auto;
            padding: 20px;
            background-color: #fff;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            border-radius: 5px;
        }

        h1 {
            color: #333;
        }

        button, input[type="text"], input[type="checkbox"] {
            margin-bottom: 10px;
        }

        .notification {
            background-color: #4CAF50;
            color: white;
            padding: 15px;
            margin-bottom: 20px;
            display: none;
        }

        .button-group button {
            margin-right: 10px;
        }

        #chargingParametersForm {
            display: none;
        }

        #chargingHistory, #accountManagement {
            display: none;
        }
        /* CSS for the output container */
        #output {
            overflow-y: auto; /* Make the container scrollable */
            max-height: 250px; /* Limit the height of the container */
            border: 1px solid #ccc;
        }
  #batteryChartContainer {
            margin-top: 20px;
        }
  #chartContainer {
        display: flex; /* Use flexbox for layout */
        align-items: center; /* Vertically center the content */
    }
    </style>
</head>
<body>

<div class="container">
    <h1>EV Charging System</h1>

    <!-- Charging Initiation -->
    <div class="button-group">
        <button class="button" onclick="initiateCharging()">Initiate Charging</button>
    </div>

<form id="chargingParametersForm" method="post">
    <label for="chargingLevel">Charging Level:</label>
    <input type="text" id="chargingLevel" placeholder="Enter charging level">
    <label style="padding-left: 10px;"  for="trickleCharge">Trickle Charge:</label>
    <input type="checkbox" id="trickleCharge">

    <div class="button-group">
        <button class="success button" id="startChargingButton" name="startCharging">Start Charging</button>
    </div>
<div id="chartContainer">
    <div style="width: 300px; height: 300px; padding-right: 10px;">
        <canvas id="batteryChart"></canvas>
    </div>
    <!-- Output container -->
    <div id="output"></div>
</div>
</form>

    <!-- Charging Completion Notification -->
    <div id="chargingCompletionNotification" class="notification"></div>

    <!-- Database and Front-End Application -->
    <div class="button-group">
        <button class="button" onclick="save()">Charging History</button>
        <button class="button" onclick="showAccountManagement()">Account Management</button>
    </div>

    <!-- Charging History -->
    <div id="chargingHistory" class="callout" style="display: none;">
        <!-- Display charging history here -->
    </div>

    <!-- Account Management -->
    <div id="accountManagement" class="callout" style="display: none;">
        <!-- Display account management options here -->
        <form method="post">
            <button type="submit" name="logout" class="button alert">Logout</button>
        </form>
    </div>
<form id="chargingParametersForm" method="post">
    <input type="hidden" name="chargingComplete" id="chargingCompleteInput" value="0">
    <input type="hidden" name="sessionId" id="sessionIdInput" value="">
</form>

</div>

<script src="https://cdn.jsdelivr.net/npm/foundation-sites@6.8.1/dist/js/foundation.min.js"></script>
<script src="https://code.jquery.com/jquery-3.6.0.min.js"></script>
<script src="https://cdn.jsdelivr.net/npm/foundation-sites@6.8.1/dist/js/foundation.min.js"></script>
<script src="https://code.jquery.com/jquery-3.6.0.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/3.7.0/chart.min.js"></script> <!-- Add Chart.js script -->
<script src="https://cdn.jsdelivr.net/npm/foundation-sites@6.8.1/dist/js/foundation.min.js"></script>
<script src="https://code.jquery.com/jquery-3.6.0.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/3.7.0/chart.min.js"></script> <!-- Add Chart.js script -->
<script>
    var batteryChart;
    var isChargingInitiated = false;
    var chargingComplete = false;
// Function to toggle visibility of charging parameters form and associated elements
    function toggleChargingParameters() {
        var chargingParametersForm = document.getElementById('chargingParametersForm');
        var chartContainer = document.getElementById('chartContainer');
        var output = document.getElementById('output');

        if (isChargingInitiated) {
            // If charging initiation is already done, hide the elements
            chargingParametersForm.style.display = 'none';
            chartContainer.style.display = 'none';
            output.innerHTML = ''; // Clear the output
        } else {
            // If charging initiation is not done, show the elements
            chargingParametersForm.style.display = 'block';
            chartContainer.style.display = 'flex'; // Show chart container as flex again
        }

        // Toggle the flag value
        isChargingInitiated = !isChargingInitiated;
    }
 // Function to initiate charging
    function initiateCharging() {
        // Toggle visibility of charging parameters form and associated elements
        toggleChargingParameters();
    }

    // Function to show charging history
    function showChargingHistory() {
        // Implement logic to fetch and display charging history
        document.getElementById('chargingHistory').style.display = 'block';
    }
var isAccountManagementShown = false; // Flag to track if account management is already shown

    // Function to toggle visibility of account management section
    function toggleAccountManagement() {
        var accountManagement = document.getElementById('accountManagement');

        if (isAccountManagementShown) {
            // If account management is already shown, hide it
            accountManagement.style.display = 'none';
        } else {
            // If account management is not shown, show it
            accountManagement.style.display = 'block';
        }

        // Toggle the flag value
        isAccountManagementShown = !isAccountManagementShown;
    }

    // Function to show account management options
    function showAccountManagement() {
        // Implement logic to display account management options
        toggleAccountManagement();
    }
    // Function to fetch output and update battery level
    function fetchOutput() {
var chargingLevel = document.getElementById('chargingLevel').value;
    if (chargingLevel === '') {
        chargingLevel = 100;
    }
    var eventSource = new EventSource('execute_main.php?chargingLevel=' + chargingLevel);
        eventSource.onmessage = function(event) {
            var output = event.data.trim();
            // Check for end-of-stream event
            if (output === '+++Terminate V2G Client / Service example for charging successfully.+++') {
		$.ajax({
     url: 'save_and_send_email.php?username=<?php echo urlencode($_SESSION['username']); ?>', // PHP script to handle sending email
    type: 'POST',
    success: function(response) {
        // Handle success
        console.log(response); // Log the response
    },
    error: function(xhr, status, error) {
        // Handle error
        console.error(xhr.responseText); // Log the error
    }
});
                eventSource.close();
            	document.getElementById('chargingCompleteInput').value = '1'; // Set chargingComplete flag
            	return;
            }
                 // Check if the output contains battery level information
            if (output.includes('Header SessionID')) {
            $('#output').append(output + "<br>");
            // Scroll to bottom to see latest output
            $('#output').scrollTop($('#output')[0].scrollHeight);
            var sessionId = parseFloat(output.split('=')[1].trim()); // Convert to percentage
            document.getElementById('sessionIdInput').value = sessionId; // Set sessionId value
            }
            // Check if the output contains battery level information
            if (output.includes('Battery leveli')) {
                // Append the data to the output container
                $('#output').append(output + "<br>");
                // Scroll to bottom to see latest output
                $('#output').scrollTop($('#output')[0].scrollHeight);

                // Extract battery level value from the output string
                var batteryLevel = parseFloat(output.split('=')[1].trim()); // Convert to percentage
                // Update battery level and chart
                updateBatteryLevel(batteryLevel);
            } else {
		document.getElementById('chargingCompleteInput').value = '0'; 
                // Append other output messages to the output container
                $('#output').append(output + "<br>");
                $('#charginglevel').append( chargingLevel + "<br>");
                // Scroll to bottom to see latest output
                $('#output').scrollTop($('#output')[0].scrollHeight);
            }
        };
    }
// Add event listener to the "Start Charging" button click event
$('#startChargingButton').click(function(event) {
    // Prevent default button behavior (form submission)
    event.preventDefault();
        resetChartData();
    // Call fetchOutput() function to initiate charging
    fetchOutput();
});

    // Battery Level Chart
    var batteryData = {
        labels: ['Battery Level'],
        datasets: [{
            label: 'Battery Level',
             backgroundColor: 'rgba(46, 204, 113, 0.5)', // Green color
        borderColor: 'rgba(46, 204, 113, 1)', // Green color,
            borderWidth: 1,
            data: [0] // Initial battery level
        }]
    };
// Ensure that the DOM is ready before creating the chart
$(document).ready(function() {
    var ctx = document.getElementById('batteryChart').getContext('2d');
    batteryChart = new Chart(ctx, {
        type: 'bar',
        data: batteryData,
  options: {
            aspectRatio: 1,
            scales: {
                y: {
                    max: 100
                }
            }
        }
    });
});

    // Function to update battery level on the chart
    function updateBatteryLevel(level) {
        batteryChart.data.datasets[0].data[0] = level;
        batteryChart.update();
    }
// Function to reset the chart data
function resetChartData() {
    // Set the initial battery level
    var initialBatteryLevel = 0;
    // Update the chart data with the initial battery level
    updateBatteryLevel(initialBatteryLevel);
}
</script>
</body>
</html>






