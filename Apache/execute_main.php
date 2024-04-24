<?php
// Set headers for Server-Sent Events
header('Content-Type: text/event-stream');
header('Cache-Control: no-cache');
// Check if the charging level was provided
if (isset($_GET['chargingLevel'])) {
    $chargingLevel = $_GET['chargingLevel'];
    // Add a debug statement to confirm that the charging level was received
    error_log("Received charging level in execute.main: $chargingLevel");
    error_log("Received charging level: ". $_GET['chargingLevel']);
} else {
    // If the charging level was not provided, set it to a default value
    $chargingLevel = 90;
    error_log("Charging level not provided, using default value: $chargingLevel");
} 
// Execute the C program and send output line by line
error_log("Received charging levels now it is: " . $chargingLevel);
error_log("Executing command: /home/pi/phase_2/sessionsetup5 " . $chargingLevel);
$process = popen('/home/pi/phase_2/sessionsetup5 ' . $chargingLevel, 'r');
while (!feof($process)) {
    echo "data: " . fgets($process) . "\n\n";
    flush(); // Flush output buffer to ensure data is sent immediately
    ob_flush(); // Flush PHP output buffer as well
}

// Close the process
pclose($process);

// Send a special event to signal the end of the stream
echo "event: end-of-stream\n";
echo "data: C program execution completed\n\n";

// Flush again to ensure the end-of-stream event is sent immediately
flush();
?>

