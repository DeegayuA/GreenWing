<?php require 'config.php'; ?>

<!DOCTYPE html>
<html>
<body>

<h2>GreenWing Ground Station</h2>

<form method="POST" action="<?php echo POST_DATA_URL;?>">
  <label for="apikey">Api Key:</label><br>
  <input type="text" id="api_key" name="api_key" value="<?php echo PROJECT_API_KEY;?>"><br>
  <label for="temperature">Temperature:</label><br>
  <input type="text" id="temperature" name="temperature" value="29.2"><br>
  <label for="humidity">Humidity:</label><br>
  <input type="text" id="humidity" name="humidity" value="89"><br>
  <label for="light_intensity">Light intensity:</label><br>
  <input type="text" id="light_intensity" name="light_intensity" value="26000"><br>
  <label for="soil_moisture">Soil Moisture:</label><br>
  <input type="text" id="soil_moisture" name="soil_moisture" value="90"><br>
  <label for="rain">Rain:</label><br>
  <input type="text" id="rain" name="rain" value="8"><br>
  <label for="wind">Wind:</label><br>
  <input type="text" id="wind" name="wind" value="18"><br>
  <label for="battery_percentage">Battery Percentage:</label><br>
  <input type="text" id="battery_percentage" name="battery_percentage" value="95"><br>
  <label for="battery_status">Battery Status:</label><br>
  <input type="text" id="battery_status" name="battery_status" value="Not-Charging"><br><br>
  <input type="submit" value="Submit">
</form> 

</body>
</html>