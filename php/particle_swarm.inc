<?php

function get_pso_row($row) {
    $pso->id = $row['id'];
    $pso->name = $row['name'];
    $pso->population_size = $row['population_size'];
    $pso->inertia = $row['inertia'];
    $pso->global_best_weight = $row['global_best_weight'];
    $pso->local_best_weight = $row['local_best_weight'];
    $pso->individuals_reported = $row['individuals_reported'];
    $pso->individuals_created = $row['individuals_created'];
    $pso->maximum_reported = $row['maximum_reported'];
    $pso->maximum_created = $row['maximum_created'];
    $pso->min_bound = $row['min_bound'];
    $pso->max_bound = $row['max_bound'];

    return $pso;
}

function get_active_pso($con) {
    $query = "SELECT * FROM particle_swarm WHERE (maximum_created <= 0 || maximum_created > individuals_created) && (maximum_reported <= 0 || maximum_reported > individuals_reported)";
    $result = mysql_query($query, $con);

    $return = array();
    while ($row = mysql_fetch_array($result)) {
        $pso = get_pso_row($row);
        $return[] = $pso;
    }
    return $return;
}

function get_inactive_pso($con) {
    $query = "SELECT * FROM particle_swarm WHERE (maximum_created > 0 && maximum_created < individuals_created) || (maximum_reported > 0 && maximum_reported < individuals_reported)";

    $result = mysql_query($query, $con);

    $return = array();
    while ($row = mysql_fetch_array($result)) {
        $pso = get_pso_row($row);
        $return[] = $pso;
    }
    return $return;
}

function print_pso_header() {
    echo "<tr>";
    print_div_td("pso_header_checks", "b a m w");
    print_div_td("pso_header_id", "id");
    print_div_td("pso_header_name", "name");
    print_div_td("pso_header", "population size");
    print_div_td("pso_header_free", "max / individuals reported ");
    print_div_td("pso_header_free", "max / individuals created");
    print_div_td("pso_header", "inertia");
    print_div_td("pso_header", "local best weight");
    print_div_td("pso_header", "global best weight");
    print_div_td("pso_header", "");
//    print_div_td("pso_header_bound", "bounds");
    echo "</tr>";
}

function print_pso_table($psos) {
    if (!empty($psos)) {
        echo "<table>\n";
        print_pso_header();

        $i = 0;
        foreach ($psos as $pso) {
            echo "<tr class='d" . ($i & 1) . "'>";

            $checkboxes =  "<input type ='checkbox' name='PSO_" . $pso->id . "_best' value=1 />";
            $checkboxes .= "<input type ='checkbox' name='PSO_" . $pso->id . "_average' value=1 />";
            $checkboxes .= "<input type ='checkbox' name='PSO_" . $pso->id . "_median' value=1 />";
            $checkboxes .= "<input type ='checkbox' name='PSO_" . $pso->id . "_worst' value=1 />";

            print_div_td("checks", $checkboxes);

            print_div_td("id", $pso->id);
            print_div_td("name", $pso->name);
            print_div_td("population_size", $pso->population_size);
            print_div_td("individuals_reported", intval($pso->maximum_created) . " / " . intval($pso->individuals_reported));
            print_div_td("individuals_created", intval($pso->maximum_created). " / " . intval($pso->individuals_created));
            print_div_td("inertia", $pso->inertia);
            print_div_td("global_best_weight", $pso->global_best_weight);
            print_div_td("local_best_weight", $pso->local_best_weight);
            print_div_td("name", "<a href='./search_errors.php?name=$pso->name'>error statistics</a>");
//            print_div_td("bound", mini_table(array("min", "max"), array($pso->min_bound, $pso->max_bound)));

            echo "</tr>\n";

            $i++;
        }
        echo "</table>\n";
    }
}

?>
