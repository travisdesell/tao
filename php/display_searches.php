<?php

require_once("tao_php_common.inc");
require_once("asynchronous_newton_method.inc");
require_once("differential_evolution.inc");
require_once("particle_swarm.inc");

require_once("/boinc/src/milkyway_server/tao/db.inc");

$host = '127.0.0.1';

$con = mysql_connect($host, $user, $pass);
mysql_select_db($db, $con);

echo "
<head>
<title>TAO Parameter Optimization Progress</title>
<link rel='stylesheet' type='text/css' href='style.css' />
</head>
";

echo "<html>";

echo "<form action='display_plots.php' method='get'>\n";

echo "<h3>Active Differential Evolution Optimization</h3>\n";
$active_de = get_active_de($con);
print_de_table($active_de);

echo "<h3>Inactive Differential Evolution Optimization</h3>\n";
$inactive_de = get_inactive_de($con);
print_de_table($inactive_de);

echo "<hr/>";

echo "<h3>Active Particle Swarm Optimization</h3>\n";
$active_pso = get_active_pso($con);
print_pso_table($active_pso);

echo "<h3>Inactive Particle Swarm Optimization</h3>\n";
$inactive_pso = get_inactive_pso($con);
print_pso_table($inactive_pso);

echo "<hr/>\n";

echo "<h3>Active Asynchronous Newton Method Optimization</h3>\n";
$active_anm = get_active_anm($con);
print_anm_table($active_anm);

echo "<h3>Inactive Asynchronous Newton Method Optimization</h3>\n";
$inactive_anm = get_inactive_anm($con);
print_anm_table($inactive_anm);

echo "<hr/>\n";


echo "<table>\n";
echo "<tr>";
echo "<td></td>";
echo "<td align='center'>first evaluation</td>";
echo "<td align='center'>last evaluation</td>";
echo "<td align='center'>limit (default 1000)</td>";
echo "<td align='center'>y-axis minimum</td>";
echo "<td align='center'>y-axis maximum</td>";
echo "<td align='center'>select every nth value</td>";
echo "</tr>";

echo "<tr>";
echo "<td><input type='submit' name='generate_plots' value='Generate Plots' id='generate_plots'></td>";
echo "<td><input type='text' size=20 maxlength=20 name='first_evaluation' value = ''></td>";
echo "<td><input type='text' size=20 maxlength=20 name='last_evaluation' value = ''></td>";
echo "<td><input type='text' size=20 maxlength=20 name='limit' value = ''></td>";
echo "<td><input type='text' size=20 maxlength=20 name='y_min' value = ''></td>";
echo "<td><input type='text' size=20 maxlength=20 name='y_max' value = ''></td>";
echo "<td><input type='text' size=20 maxlength=20 name='nth_value' value = ''></td>";
echo "</tr>";

echo "</table>\n";
echo "</form>\n";

echo "</html>";
?>
