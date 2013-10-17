<?php

$search_name = $_GET['name'];

echo "
<head>
<title>Error Statistics for '$search_name'</title>
<link rel='stylesheet' type='text/css' href='style.css' />
</head>
<html>
";
require_once("/boinc/src/milkyway_server/tao/php/db.inc");

$host = '127.0.0.1';

$con = mysql_connect($host, $user, $pass);
mysql_select_db($db, $con);

//echo "GET: " . json_encode($_GET) . "\n";
//echo "<br>";
//echo "POST: " . json_encode($_POST) . "\n";
//echo "<br>";

echo "<h3>Search Information</h3>\n";
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

echo "<hr/>";
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
    $query = "SELECT id, workunitid, userid, hostid, app_version_id FROM result WHERE " . $where_clause;
    $result = mysql_query($query, $con);

    echo "<table class>\n";
    echo "<tr>\n";
    echo "<td><div class='error_stats_header'>host id<div></td> <td><div class='error_stats_header'>user id</div></td> <td><div class='error_stats_header'>application</div></td> ";
    echo "<td><div class='error_stats_header'><table><tr><td style='width:70px'>result id</td>  <td style='width:75px'>workunit id</td> <td>command line</td> </tr></table> </div></td>\n";
    echo "</tr>\n";

    $info_by_hostid = array();

    $app_version_names = array();

    while ($row = mysql_fetch_array($result)) {
        if (!array_key_exists($row['hostid'], $info_by_hostid)) {
            $info_by_hostid[$row['hostid']]->hostid = $row['hostid'];
            $info_by_hostid[$row['hostid']]->userids = array();
            $info_by_hostid[$row['hostid']]->app_version_ids = array();
            $info_by_hostid[$row['hostid']]->ids = array();
            $info_by_hostid[$row['hostid']]->workunitids = array();
        }

        if (!in_array($row['userid'], $info_by_hostid[$row['hostid']]->userids)) $info_by_hostid[$row['hostid']]->userids[] = $row['userid'];
        if (!in_array($row['app_version_id'], $info_by_hostid[$row['hostid']]->app_version_ids)) $info_by_hostid[$row['hostid']]->app_version_ids[] = $row['app_version_id'];
        if (!in_array($row['id'], $info_by_hostid[$row['hostid']]->ids)) $info_by_hostid[$row['hostid']]->ids[] = $row['id'];
        if (!in_array($row['workunitid'], $info_by_hostid[$row['hostid']]->workunitids)) $info_by_hostid[$row['hostid']]->workunitids[] = $row['workunitid'];

        if (!array_key_exists($row['app_version_id'], $app_version_names)) {
            if ($row['app_version_id'] <= 0) {
                $app_version_names[ $row['app_version_id'] ] = "app_version_id: " . $row['app_version_id'] . " (anonymous?)";
            } else {
                $query2 = "SELECT xml_doc FROM app_version WHERE id = " . $row['app_version_id'];
                $result2 = mysql_query($query2, $con);

                $row2 = mysql_fetch_array($result2);
                $doc = $row2['xml_doc'];

                $first_pos = strpos($doc, "<name>") + 6;
                $str_length = strpos($doc, "</name>") - $first_pos;
                $app_name = substr($doc, $first_pos, $str_length);

                $app_version_names[ $row['app_version_id'] ] = $app_name;
            }
        }
        //    echo json_encode($info_by_hostid[$row['hostid']]) . "<br>\n";
    }
        

    $workunit_cmd_line = array();

    $row_type = 0;
    foreach ($info_by_hostid as $info) {
        echo "<tr class='d" . ($row_type & 1) . "'> ";
        $row_type++;

        echo "<td><a href='http://milkyway.cs.rpi.edu/milkyway/show_host_detail.php?hostid=" . $info->hostid . "'>" . $info->hostid . "</a></td> ";
        echo "<td><a href='http://milkyway.cs.rpi.edu/milkyway/show_user.php?userid=" . $info->userids[0] . "'>" . $info->userids[0] . "</a></td> ";

        echo "<td>";

        echo "<table>";
        $n = count($info->app_version_ids);
        for ($i = 0; $i < $n; $i++) {
            echo "<tr><td>" . $app_version_names[$info->app_version_ids[$i]] . "</td></tr>";
        }
        echo "</table>";
        echo "</td>";

        echo "<td>";
        echo "<table>";

        $c = 0;
        $n = count($info->ids);
        for ($i = 0; $i < $n; $i++) {
            echo "<tr class='d" . ($i & 1) . "'><td>";
            echo "<a href='http://milkyway.cs.rpi.edu/milkyway/result.php?resultid=" . $info->ids[$i] . "'>" . $info->ids[$i] . "</a>";
            echo "</td><td>";
            echo "<a href='http://milkyway.cs.rpi.edu/milkyway/workunit.php?wuid=" . $info->workunitids[$i] . "'>" . $info->workunitids[$i] . "</a>";

            echo "</td><td>";

            if (!array_key_exists($info->workunitids[$i], $workunit_cmd_line)) {
                $query = "SELECT xml_doc FROM workunit WHERE id = " . $info->workunitids[$i];
                $result = mysql_query($query);
                $row = mysql_fetch_array($result);

                $xml_doc = $row['xml_doc'];
                $first_pos = strpos($xml_doc, "<command_line>") + 14;
                $str_length = strrpos($xml_doc, "</command_line>") - $first_pos;

                $workunit_cmd_line[$info->workunitids[$i]] = substr($xml_doc, $first_pos, $str_length);
            } 
            echo $workunit_cmd_line[$info->workunitids[$i]];
//            die();

            echo "</td></tr>";
        }
        echo "</table>";

        echo "</td>";
        echo "</tr>\n";
    }

    echo "</table>\n";

}

echo "<hr/>";
echo "<h3>Client errors</h3>\n";
show_error_results("name like '$search_name%' and outcome = 3", $con);

echo "<hr/>";
echo "<h3>Validate errors</h3>";
show_error_results("name like '$search_name%' and outcome = 6", $con);

echo "</html>";
?>

