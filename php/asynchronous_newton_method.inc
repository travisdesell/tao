<?php

function get_anm_row($row) {
    $anm->id = $row['id'];
    $anm->name = $row['name'];
    $anm->minimum_line_search_individuals = $row['minimum_line_search_individuals'];
    $anm->line_search_individuals_reported = $row['minimum_line_search_individuals'];
    $anm->minimum_regression_individuals = $row['minimum_regression_individuals'];
    $anm->regression_individuals_reported = $row['regression_individuals_reported'];
    $anm->extra_workunits = $row['extra_workunits'];
    $anm->line_search_min = $row['line_search_min'];
    $anm->line_search_max = $row['line_search_max'];
    $anm->current_iteration = $row['current_iteration'];
    $anm->maximum_iterations = $row['maximum_iterations'];
    $anm->center_fitness = $row['center_fitness'];
    $anm->center = $row['center'];
    $anm->regression_radius = $row['regression_radius'];
    $anm->min_bound = $row['min_bound'];
    $anm->max_bound = $row['max_bound'];

    return $anm;
}

function get_active_anm($con) {
    $query = "SELECT * FROM asynchronous_newton_method WHERE (maximum_iterations <= 0 || maximum_iterations > current_iteration)";
    $result = mysql_query($query, $con);

    $return = array();
    while ($row = mysql_fetch_array($result)) {
        $anm = get_anm_row($row);
        $return[] = $anm;
    }
    return $return;
}

function get_inactive_anm($con) {
    $query = "SELECT * FROM asynchronous_newton_method WHERE (maximum_iterations > 0 && maximum_iterations <= current_iteration)";

    $result = mysql_query($query, $con);

    $return = array();
    while ($row = mysql_fetch_array($result)) {
        $anm = get_anm_row($row);
        $return[] = $anm;
    }
    return $return;
}

function print_anm_header() {
    echo "<tr>";
    print_div_td("anm_header_checks", "b a m w");
    print_div_td("anm_header_id", "id");
    print_div_td("anm_header_name", "name");
    print_div_td("anm_header_fitness", "center fitness");
    print_div_td("anm_header_free", "line search individuals reported / required");
    print_div_td("anm_header_free", "regression individuals reported / required");
    print_div_td("anm_header", "extra workunits");
    print_div_td("anm_header", "line search min / max");
    print_div_td("anm_header", "iterations current / max");
    print_div_td("anm_header_bound", "bounds / center / radius");
    echo "</tr>";
}

function print_anm_table($anms) {
    if (!empty($anms)) {
        echo "<table>\n";
        print_anm_header();

        $i = 0;
        foreach ($anms as $anm) {
            echo "<tr class='d" . ($i & 1) . "'>";

            $checkboxes =  "<input type ='checkbox' name='PSO_" . $anm->id . "_best' value=1 />";
            $checkboxes .= "<input type ='checkbox' name='PSO_" . $anm->id . "_average' value=1 />";
            $checkboxes .= "<input type ='checkbox' name='PSO_" . $anm->id . "_median' value=1 />";
            $checkboxes .= "<input type ='checkbox' name='PSO_" . $anm->id . "_worst' value=1 />";

            print_div_td("checks", $checkboxes);

            print_div_td("id", $anm->id);
            print_div_td("name", $anm->name);
            print_div_td("center_fitness", number_format($anm->center_fitness, 5, '.', ''));
            print_div_td("ls_reported_required", intval($anm->line_search_individuals_reported) . " / " . intval($anm->minimum_line_search_individuals));
            print_div_td("rg_reported_required", intval($anm->regression_individuals_reported). " / " . intval($anm->minimum_regression_individuals));
            print_div_td("extra_workunits", $anm->extra_workunits);
            print_div_td("ls_min_max", $anm->line_search_min . " / " . $anm->line_search_max);
            print_div_td("iterations_min_max", $anm->current_iteration . " / " . $anm->maximum_iterations);
            print_div_td("bound", mini_table(array("min", "max", "center", "radius"), array($anm->min_bound, $anm->max_bound, $anm->center, $anm->regression_radius)));

            echo "</tr>\n";

            $i++;
        }
        echo "</table>\n";
    }
}

?>
