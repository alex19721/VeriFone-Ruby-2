<?php
//echo('PHP Here!');
$host='localhost';
$username='admin';
$password='123';
$databasename='dvr_db';
$sql = "use dvr_db;";
$sql1= "SELECT * FROM Data WHERE Date='";

if($_SERVER['REQUEST_METHOD'] == "POST" and isset($_POST['date']))
{

$dt=date("m/d/y", strtotime($_POST['date']));
print_r('<h2><u>RUBI DVR logs on ');
print_r($dt);
print_r('</u></h2></p>');
$conn = new mysqli($host, $username, $password, $databasename);
$conn->query($sql);
$sql1=$sql1.$dt."';";
$res=$conn->query($sql1);

//    echo '<table>';
//    foreach ($res as $row) {
//	echo '<tr>';
//	echo "<td><pre>{$row['Date']}</pre></td>";
//	echo "<td><pre>{$row['Time']}</pre></td>";
//	echo "<td><pre>{$row['Transaction']}</pre></td>";
//	echo '</tr>';
//    }
//    echo '</table>';



foreach ($res as $row) {
    echo('<pre>'.$row['Date']);
    echo(' ');
    echo($row['Time']);
    echo(' ');
    echo($row['Transaction'].'</pre>');
//    echo('<br>');

    if (strpos($row['Transaction'],'CSH: ',0)){
	print_r('------------------------------------------------------------------------------------------');
        print_r('<br>');
    }

}
$conn->close();
}
?>
