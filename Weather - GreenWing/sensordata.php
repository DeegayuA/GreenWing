<?php 

require 'config.php';

//----------------------------------------------------------------------------

try{
	if ($_SERVER["REQUEST_METHOD"] == "POST") {
		$api_key = escape_data($_POST["api_key"]);
			print_r($_POST);
			echo "<br>PROJECT_API_KEY: ".PROJECT_API_KEY."<br>";
		//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
		if($api_key == PROJECT_API_KEY) {
			$temperature = escape_data($_POST["temperature"]);
			$humidity = escape_data($_POST["humidity"]);
			$light_intensity = escape_data($_POST["light_intensity"]);
			$rain = escape_data($_POST["rain"]);
			$wind = escape_data($_POST["wind"]);
			$soil_moisture= escape_data($_POST["soil_moisture"]);
			$battery_percentage = escape_data($_POST["battery_percentage"]);
			$battery_status = escape_data($_POST["battery_status"]);
			
			$sql = "INSERT INTO tbl_data (temperature, humidity, light_intensity, rain, wind, soil_moisture, battery_percentage, battery_status, created_date) 
        VALUES ('".$temperature."','".$humidity."','".$light_intensity."','".$rain."','".$wind."','".$soil_moisture."','".$battery_percentage."','".$battery_status."','".date("Y-m-d H:i:s")."')";

			if($db->query($sql) === FALSE)
				{ echo "Error: " . $sql . "<br>" . $db->error; }
	
			echo "OK. INSERT ID: ";
			echo $db->insert_id;
		}
		//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
		else
		{
			echo "Wrong API Key";
		}
		//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
	}
	//----------------------------------------------------------------------------
	else
	{
		echo "No HTTP POST request found";
	}
	//----------------------------------------------------------------------------
	
}catch (Exception $e) {
    echo 'Caught exception: ',  $e->getMessage(), "\n";
}

function escape_data($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}