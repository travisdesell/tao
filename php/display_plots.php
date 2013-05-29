<?php
echo "
<head>
<title>TAO Parameter Optimization Progress Plots</title>
</head>
";
require_once("db.inc");

$host = '127.0.0.1';

$con = mysql_connect($host, $user, $pass);
mysql_select_db($db, $con);


//echo "GET: " . json_encode($_GET) . "\n";
//echo "POST: " . json_encode($_POST) . "\n";


$de = array();
$pso = array();


$data = array();

if (!empty($_GET)) {
    $query_limits = "";
    if ($_GET['first_evaluation'] != null) {
        $query_limits .= " AND evaluation >= " . intval($_GET['first_evaluation']);
    }
    if ($_GET['last_evaluation'] != null) {
        $query_limits .= " AND evaluation <= " . intval($_GET['last_evaluation']);
    }

    $y_min = null;
    $y_max = null;
    if ($_GET['y_min'] != null) {
        $y_min = doubleval($_GET['y_min']);
    }
    if ($_GET['y_max'] != null) {
        $y_max = doubleval($_GET['y_max']);
    }

//    echo "y_min: $y_min<br>\n";
//    echo "y_max: $y_max<br>\n";

    $limit = 1000;
    if ($_GET['limit'] != null) {
        $limit = min($limit, intval($_GET['limit']));
    }

    $nth_value = 1;
    if ($_GET['nth_value'] != null) {
        $nth_value = intval($_GET['nth_value']);
    }

    foreach (array_keys($_GET) as $value) {
        $vals = explode("_", $value);
        if (count($vals) == 3) {
            $current_id = $vals[1];

            if ($vals[0] == "DE") {
                $de_queries[$current_id][] = $vals[2];
            } else if ($vals[0] == "PSO") {
                $pso_queries[$current_id][] = $vals[2];
            } else if ($vals[0] == "ANM") {
                $anm_queries[$current_id][] = $vals[2];
            }
        }
    }

//    echo json_encode($de_queries) . "<br><br>";
//    echo json_encode($pso_queries) . "<br><br>";
//    echo json_encode($anm_queries) . "<br><br>";

    if ($de_queries != null)  $queries['differential_evolution'] = $de_queries;
    if ($pso_queries != null) $queries['particle_swarm'] = $pso_queries;
    if ($anm_queries != null) $queries['asynchronous_newton_method'] = $anm_queries;

//    echo json_encode($queries) . "<br><br>";

    foreach (array_keys($queries) as $search_type_name) {
        foreach (array_keys($queries[$search_type_name]) as $current_id) {
            $name_query = "SELECT name FROM " . $search_type_name . " WHERE id = " . $current_id;
            $row = mysql_fetch_assoc( mysql_query($name_query, $con) );
            $search_name = $row['name'];

            $fitness_query = "SELECT * FROM (";
            $fitness_query .= "SELECT (@x:=@x+1) AS x, evaluation";
            foreach ($queries[$search_type_name][$current_id] as $fitness_type) {
                $fitness_query .= ", " . $fitness_type;
            }
            $fitness_query .= " FROM (SELECT @x := 0) t, " . $search_type_name . "_log WHERE search_id = " . $current_id;
            if ($vals[0] == "PSO") $fitness_query .= " AND global = true";
            $fitness_query .= $query_limits;
            $fitness_query .= ") t WHERE x MOD " . $nth_value . " = 0";
            $fitness_query .= " LIMIT " . $limit;

//            echo $fitness_query . "<br><br>";

            $result = mysql_query($fitness_query, $con);

            $fitnesses = array();
            while ($row = mysql_fetch_assoc($result)) {
                foreach ($queries[$search_type_name][$current_id] as $fitness_type) {
                    $f = array( intval($row['evaluation']), doubleval($row[$fitness_type]) );
                    if ( $f[1] < -99999 ) $f[1] = -99999;
                    $fitnesses[$fitness_type][] = $f;
                }
            }

//            echo json_encode($fitnesses) . "<br><br>";

            foreach ($queries[$search_type_name][$current_id] as $fitness_type) {
                $current['name'] = $search_name . " " . $fitness_type;
                $current['data'] = $fitnesses[$fitness_type];

                $data[] = $current;
            }

//            echo json_encode($current) . "<br><br>";
        }
    }

    /*
    foreach (array_keys($_GET) as $value) {
//        echo $value . "<br>";

        $vals = explode("_", $value);
        if (count($vals) == 3) {
            if ($vals[0] == "DE") {
                $search_type_name = "differential_evolution";

            } else if ($vals[0] == "PSO") {
                $search_type_name = "particle_swarm";
            }

            $search_id = intval($vals[1]);
            $fitness_type = $vals[2];

            $name_query = "SELECT name FROM " . $search_type_name . " WHERE id = " . $search_id;
            $row = mysql_fetch_assoc( mysql_query($name_query, $con) );
            $search_name = $row['name'];

            $fitness_query = "SELECT evaluation, " . $fitness_type . " FROM " . $search_type_name . "_log WHERE search_id = " . $search_id;
            if ($vals[0] == "PSO") $fitness_query .= " AND global = true";
            $fitness_query .= $query_limits;
            $fitness_query .= " LIMIT " . $limit;

            echo $fitness_query . "<br><br>";

            $result = mysql_query($fitness_query, $con);

            $fitnesses = array();
            while ($row = mysql_fetch_assoc($result)) {
                $f = array( intval($row['evaluation']), doubleval($row[$fitness_type]) );
                if ( $f[1] < -99999 ) $f[1] = -99999;
                $fitnesses[] = $f;
            }
//            echo $fitness_query . "<br>";
//            echo "fitnesses: " . json_encode($fitnesses) . "<br>";

//            echo "SEARCH NAME: " . $search_name . " <br>";

            $current['name'] = $search_name . " " . $fitness_type;
            $current['data'] = $fitnesses;

            $data[] = $current;
        }
    }
     */
}

/*
$query = "SELECT evaluation, fitness FROM particle_swarm_log WHERE swarm_id = 2 and global = true LIMIT 100";

$result = mysql_query($query, $con);
$php_evaluations = array();
$php_fitnesses = array();

while ($row = mysql_fetch_assoc($result)) {
        $php_evaluations[] = intval($row['evaluation']);
        $php_fitnesses[] = doubleval($row['fitness']);
}
 */
//print_r($php_evaluations);
//print_r($php_fitnesses);

//$test['name'] = 'ps2';
//$test['data'] = array(0, 2, 3, 4, 5, 10, -3);

?>

<script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.7.1/jquery.min.js"></script>
<script type="text/javascript">
jQuery.noConflict();
</script>

<script src="http://code.highcharts.com/highcharts.js"></script>
<script src="http://code.highcharts.com/modules/exporting.js"></script>

<div id="container" style="min-width: 400px; height: 400px; margin: 0 auto"></div>


<script type="text/javascript">
 (function($){ // encapsulate jQuery

$(function () {
    var chart;
    $(document).ready(function() {
        chart = new Highcharts.Chart({
            chart: {
                renderTo: 'container',
                type: 'line',
                height: 650,
                marginRight: 130,
                marginBottom: 25,

                events: {
                    load: function(event) {
//                        if (this.yAxis[0].getExtremes().dataMin < -10000) {
//                            this.yAxis[0].setExtremes(-10000, this.yAxis[0].getExtremes().dataMax); 
//                        }
                        var y_min = <?php if ($y_min == null) { echo "null"; } else { echo $y_min; } ?>;
                        var y_max = <?php if ($y_max == null) { echo "null"; } else { echo $y_max; } ?>;
                        if (y_min == null) y_min = this.yAxis[0].getExtremes().dataMin;
                        if (y_max == null) y_max = this.yAxis[0].getExtremes().dataMax;

                        this.yAxis[0].setExtremes( y_min, y_max );
                    }
                }
            },

            title: {
                text: 'Parameter Optimization Progress',
                style: {
                    font: '32px "Trebuchet MS", Verdana, sans-serif'
                },
                x: -20 //center
            },

/*            subtitle: {
                text: 'Source: TAO',
                x: -20
            },
 */

            xAxis: {
                labels: {
                    style: {
                        font: '18px "Trebuchet MS", Verdana, sans-serif'
                    }
                }
/*                title: {
                    text: 'Evaluation',
                    style: {
                        font: '32px "Trebuchet MS", Verdana, sans-serif'
                    }
            }*/
            },

            yAxis: {
                title: {
                    text: 'Fitness',
                    style: {
                        font: '32px "Trebuchet MS", Verdana, sans-serif'
                    }
                },
                plotLines: [{
                    value: 0,
                    width: 1,
                    color: '#808080'
                }],
                labels: {
                    style: {
                        font: '18px "Trebuchet MS", Verdana, sans-serif'
                    }
                }
            },

            tooltip: {
                formatter: function() {
                        return '<b>'+ this.series.name +'</b><br/>'+
                        'evaluation: ' + this.x +' : fitness: ' + this.y;
                }
            },

            legend: {
                layout: 'vertical',
                align: 'right',
                verticalAlign: 'top',
                x: -10,
                y: 100,
                borderWidth: 0,
                itemStyle: {
                    font: '24px "Trebuchet MS", Verdana, sans-serif'
                }
            },

            series: <?php echo json_encode($data); ?>
        });
    });
});

})(jQuery);
</script>

