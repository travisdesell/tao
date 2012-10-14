<?php

$search_name = $_GET['name'];

echo "
<head>
<title>Error Statistics for '$search_name'</title>
<link rel='stylesheet' type='text/css' href='style.css' />
</head>
<html>
";
require_once("/boinc/src/milkyway_server/tao/db.inc");

$host = '127.0.0.1';

$con = mysql_connect($host, $user, $pass);
mysql_select_db($db, $con);

//echo "GET: " . json_encode($_GET) . "\n";
//echo "<br>";
//echo "POST: " . json_encode($_POST) . "\n";
//echo "<br>";

echo "<table>\n";
echo "<tr class='error_stats_header'>\n";
echo "<td><div class='error_stats_header'>parameter file</div></td> <td><div class ='error_stats_header'>star file</div></td>";
echo "</tr>\n";

echo "<tr>";
$query = "SELECT input_filenames FROM tao_workunit_information WHERE search_name like '$search_name'";
$result = mysql_query($query);
$row = mysql_fetch_array($result);

$input_filenames = explode(",", substr($row['input_filenames'], 1, -1));

echo "<td>" . $input_filenames[0] . "</td> <td>" . $input_filenames[1] . "</td>";
echo "</tr>";
echo "</table>";

echo "<h3>Result statistics for $search_name</h3>\n";
echo "<table>\n";

echo "<tr>";
echo "<td><div class='error_stats_header'>count</div></td> <td><div class='error_stats_header'>success</div></td> <td><div class='error_stats_header'>client error</div></td> <td><div class='error_stats_header'>no reply<div></td> ";
echo "<td><div class='error_stats_header'>didn't need</div></td> <td><div class='error_stats_header'>validate error</div></td> <td><div class='error_stats_header'>client detached</div></td>";
echo "</tr>\n";

function get_count($table_name, $where_clause, $con) {
    $query = "SELECT count(*) FROM $table_name WHERE $where_clause";
    $result = mysql_query($query, $con);

//    echo "$query <br>\n";

    if ($result) {
        $row = mysql_fetch_array($result);
        $count = $row['count(*)'];
//        echo "$count <br>\n";
    } else {
        $count = 0;
    }
    return $count;
}

$result_count = get_count("result", "name like '$search_name%'", $con);
$success_count = get_count("result", "name like '$search_name%' and outcome = 1", $con);
$error_count = get_count("result", "name like '$search_name%' and outcome = 3", $con);
$no_reply_count = get_count("result", "name like '$search_name%' and outcome = 4", $con);
$didnt_need_count = get_count("result", "name like '$search_name%' and outcome = 5", $con);
$validate_error_count = get_count("result", "name like '$search_name%' and outcome = 6", $con);
$client_detached_count = get_count("result", "name like '$search_name%' and outcome = 7", $con);

echo "<tr class='d0'>";
echo "<td>$result_count</td> <td>$success_count</td> <td>$error_count</td> <td>$no_reply_count</td> <td>$didnt_need_count</td> <td>$validate_error_count</td> <td>$client_detached_count</td>";
echo "</tr>\n";

echo "</table>";

function show_error_results($where_clause, $con) {
    $query = "SELECT id, userid, hostid, app_version_id FROM result WHERE " . $where_clause;
    $result = mysql_query($query, $con);

    echo "<table class>\n";
    echo "<tr>\n";
    echo "<td><div class='error_stats_header'>host id<div></td> <td><div class='error_stats_header'>user id</div></td> <td><div class='error_stats_header'>application</div></td> <td><div class='error_stats_header'>result ids</div></td>\n";
    echo "</tr>\n";

    $info_by_hostid = array();

    $app_version_names = array();

    while ($row = mysql_fetch_array($result)) {
        if ($info_by_hostid[$row['hostid']] == null) {
            $info_by_hostid[$row['hostid']]->hostid = $row['hostid'];
            $info_by_hostid[$row['hostid']]->userids = array();
            $info_by_hostid[$row['hostid']]->app_version_ids = array();
            $info_by_hostid[$row['hostid']]->ids = array();
        }

        if (!in_array($row['userid'], $info_by_hostid[$row['hostid']]->userids)) $info_by_hostid[$row['hostid']]->userids[] = $row['userid'];
        if (!in_array($row['app_version_id'], $info_by_hostid[$row['hostid']]->app_version_ids)) $info_by_hostid[$row['hostid']]->app_version_ids[] = $row['app_version_id'];
        if (!in_array($row['id'], $info_by_hostid[$row['hostid']]->ids)) $info_by_hostid[$row['hostid']]->ids[] = $row['id'];

        if (!array_key_exists($row['app_version_id'], $app_version_names)) {
            $query2 = "SELECT xml_doc FROM app_version WHERE id = " . $row['app_version_id'];
            $result2 = mysql_query($query2, $con);

            $row2 = mysql_fetch_array($result2);
            $doc = $row2['xml_doc'];

            $first_pos = strpos($doc, "<name>") + 6;
            $str_length = strrpos($doc, "</name>") - $first_pos;
            $app_name = substr($doc, $first_pos, $str_length);

            $app_version_names[ $row['app_version_id'] ] = $app_name;
        }
        //    echo json_encode($info_by_hostid[$row['hostid']]) . "<br>\n";
    }

    $row_type = 0;
    foreach ($info_by_hostid as $info) {
        echo "<tr class='d" . ($row_type & 1) . "'> ";
        $row_type++;

        echo "<td>" . $info->hostid . "</td> ";
        echo "<td>" . $info->userids[0] . "</td> ";

        echo "<td>";
        $c = 0;
        $n = count($info->app_version_ids);
        for ($i = 0; $i < $n; $i++) {
            if ($i > 0) echo ", ";
            echo $app_version_names[$info->app_version_ids[$i]];
        }
        echo "</td>";

        echo "<td>";
        $c = 0;
        $n = count($info->ids);
        for ($i = 0; $i < $n; $i++) {
            if ($i > 0) echo ", ";

            echo "<a href='http://milkyway.cs.rpi.edu/milkyway/result.php?resultid=" . $info->ids[$i] . "'>" . $info->ids[$i] . "</a>";
        }
        echo "</td>";
        echo "</tr>\n";
    }

    echo "</table>\n";

}

echo "<h3>Client errors</h3>\n";
show_error_results("name like '$search_name%' and outcome = 3", $con);

echo "<h3>Validate errors</h3>";
show_error_results("name like '$search_name%' and outcome = 6", $con);

echo "</html>";
?>

