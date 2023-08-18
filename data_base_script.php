<?php
  
  

  function test_input($data) {
   $data = trim($data);
   $data = stripslashes($data);
   $data = htmlspecialchars($data);
   return $data;
}
  
  class MyDB extends SQLite3 {
      function __construct() {
         $this->open('data.db');
      }
   }
   $db = new MyDB();
   $chosen_activity  = $activity_date_weekday ="" ;
   $seconds_passed = $minutes_passed = $hours_passed = $activity_date_month = $activity_date_day_number = $activity_date_hour = $activity_date_minute = $activity_date_year = 0 ;
   if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $chosen_activity = $_POST["chosen_activity"];
    $hours_passed = $_POST["hours_passed"];
    $minutes_passed = $_POST["minutes_passed"];
    $seconds_passed = $_POST["seconds_passed"];
    $activity_date_month = $_POST["activity_date_month"];
    $activity_date_day_number = $_POST["activity_date_day_number"];
    $activity_date_weekday = $_POST["activity_date_weekday"];
    $activity_date_hour = $_POST["activity_date_hour"];
    $activity_date_minute = $_POST["activity_date_minute"];
    $activity_date_year = $_POST["activity_date_year"];
    if(!$db) {
      echo $db->lastErrorMsg();
   } else {
      echo "Opened database successfully\n";
   }
   $sql =<<<EOF
   INSERT INTO activities (chosen_activity,hours_passed,minutes_passed,seconds_passed,activity_date_month,activity_date_day_number,activity_date_weekday,activity_date_hour,activity_date_minute,activity_date_year)
   VALUES ("$chosen_activity","$hours_passed","$minutes_passed","$seconds_passed","$activity_date_month","$activity_date_day_number","$activity_date_weekday","$activity_date_hour","$activity_date_minute","$activity_date_year");
  
EOF;

   $ret = $db->exec($sql);
   if(!$ret) {
      echo $db->lastErrorMsg();
   } else {
      echo "Records created successfully\n";
      echo $chosen_activity;
      
   }
   $db->close();

   }
if($_SERVER["REQUEST_METHOD"] == "GET"){
   $sql =<<<EOF
   select * from activities ORDER BY id DESC LIMIT 1
  
EOF;
$ret = $db->query($sql);
if(!$ret) {
   echo $db->lastErrorMsg();
} else {
   while ($row = $ret->fetchArray()) {
      echo $row[5] ;
      echo ",";
      echo  $row[6];
  }

}
} 












