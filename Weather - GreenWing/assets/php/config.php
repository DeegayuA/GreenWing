<?php 
define('DB_HOST'    , 'fdb1033.awardspace.net'); 
define('DB_USERNAME', '4381468_grounddb'); 
define('DB_PASSWORD', 'Deegayu2001'); 
define('DB_NAME'    , '4381468_grounddb');

define('POST_DATA_URL', 'http://greenwing.scienceontheweb.net/assets/php/sensordata.php');

//PROJECT_API_KEY is the exact duplicate of, PROJECT_API_KEY in NodeMCU sketch file
//Both values must be same
define('PROJECT_API_KEY', 'GreenWing');


//set time zone for your country
date_default_timezone_set('Asia/Colombo');

// Connect with the database 
$db = new mysqli(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME); 
 
// Display error if failed to connect 
if ($db->connect_errno) { 
    echo "Connection to database is failed: ".$db->connect_error;
    exit();
}
