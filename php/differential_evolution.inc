<?php

function get_de_row($row) {
    $de->id = $row['id'];
    $de->name = $row['name'];
    $de->population_size = $row['population_size'];
    $de->parent_selection = $row['parent_selection'];
    $de->number_pairs = $row['number_pairs'];
    $de->recombination_selection = $row['recombination_selection'];
    $de->parent_scaling_factor = $row['parent_scaling_factor'];
    $de->differential_scaling_factor = $row['differential_scaling_factor'];
    $de->crossover_rate = $row['crossover_rate'];
    $de->directional = $row['directional'];
    $de->individuals_reported = $row['individuals_reported'];
    $de->individuals_created = $row['individuals_created'];
    $de->maximum_reported = $row['maximum_reported'];
    $de->maximum_created = $row['maximum_created'];
    $de->min_bound = $row['min_bound'];
    $de->max_bound = $row['max_bound'];

    return $de;
}

function get_active_de($con) {
    $query = "SELECT * FROM differential_evolution WHERE (maximum_created <= 0 || maximum_created > individuals_created) && (maximum_reported <= 0 || maximum_reported > individuals_reported)";
    $result = mysql_query($query, $con);

    $return = array();
    while ($row = mysql_fetch_array($result)) {
        $de = get_de_row($row);
        $return[] = $de;
    }
    return $return;
}

function get_inactive_de($con) {
    $query = "SELECT * FROM differential_evolution WHERE (maximum_created > 0 && maximum_created < individuals_created) || (maximum_reported > 0 && maximum_reported < individuals_reported)";

    $result = mysql_query($query, $con);

    $return = array();
    while ($row = mysql_fetch_array($result)) {
        $de = get_de_row($row);
        $return[] = $de;
    }
    return $return;
}

function print_de_header() {
    echo "<tr>";
    print_div_td("de_header_checks", "b a m w");
    print_div_td("de_header_id", "id");
    print_div_td("de_header_name", "name");
    print_div_td("de_header", "population size");
    print_div_td("de_header_free", "max / individuals reported");
    print_div_td("de_header_free", "max / individuals created");
    print_div_td("de_header_selection", "parent / pairs / recombination");
    print_div_td("de_header", "parent scaling factor");
    print_div_td("de_header", "differential scaling factor");
    print_div_td("de_header", "crossover rate");
    print_div_td("de_header", "directional");
    print_div_td("de_header", "");
    echo "</tr>";
}

function print_de_table($des) {
    if (!empty($des)) {
        echo "<table>\n";
        print_de_header();

        $i = 0;
        foreach ($des as $de) {
            echo "<tr class='d" . ($i & 1) . "'>";

            $checkboxes =  "<input type ='checkbox' name='DE_" . $de->id . "_best' value=1 />";
            $checkboxes .= "<input type ='checkbox' name='DE_" . $de->id . "_average' value=1 />";
            $checkboxes .= "<input type ='checkbox' name='DE_" . $de->id . "_median' value=1 />";
            $checkboxes .= "<input type ='checkbox' name='DE_" . $de->id . "_worst' value=1 />";

            print_div_td("checks", $checkboxes);

            print_div_td("id", $de->id);
            print_div_td("name", $de->name);
            print_div_td("population_size", $de->population_size);
            print_div_td("individuals_reported", $de->maximum_reported . " / " . $de->individuals_reported);
            print_div_td("individuals_created", $de->maximum_created . " / " . $de->individuals_created);

            $parent_n_recombination = "";

            //This is bad, should have these defined in some common location/method -- if these get changed in differential_evolution.hxx they wont be changed here
            if ($de->parent_selection == 0) {
                //print_div_td("parent_selection", "best");
                $parent_n_recombination .= "best/";
            } else if ($de->parent_selection == 1) {
                //print_div_td("parent_selection", "random");
                $parent_n_recombination .= "random/";
            } else if ($de->parent_selection == 2) {
                //print_div_td("parent_selection", "current-to-best");
                $parent_n_recombination .= "current-to-best/";
            } else if ($de->parent_selection == 3) {
                //print_div_td("parent_selection", "current-to-random");
                $parent_n_recombination .= "current-to-random/";
            } else {
                //print_div_td("parent_selection", "unknown");
                $parent_n_recombination .= "unknown/";
            }

            //print_div_td("number_pairs", $de->number_pairs);
            $parent_n_recombination .= $de->number_pairs . "/";

            //This is bad, should have these defined in some common location/method -- if these get changed in differential_evolution.hxx they wont be changed here
            if ($de->recombination_selection == 0) {
                //print_div_td("recombination_selection", "binary");
                $parent_n_recombination .= "binary";
            } else if ($de->recombination_selection == 1) {
                //print_div_td("recombination_selection", "exponential");
                $parent_n_recombination .= "exponential";
            } else if ($de->recombination_selection == 2) {
                //print_div_td("recombination_selection", "sum");
                $parent_n_recombination .= "sum";
            } else if ($de->recombination_selection == 3) {
                //print_div_td("recombination_selection", "none");
                $parent_n_recombination .= "none";
            } else {
                //print_div_td("recombination_selection", "unknown");
                $parent_n_recombination .= "unknown";
            }

            print_div_td("selection", $parent_n_recombination);

            print_div_td("parent_scaling_factor", $de->parent_scaling_factor);
            print_div_td("differential_scaling_factor", $de->differential_scaling_factor);
            print_div_td("crossover_rate", $de->crossover_rate);

            if ($de->directional == 0) {
                print_div_td("directional", "false");
            } else if ($de->directional == 1) {
                print_div_td("directional", "true");
            } else {
                print_div_td("directional", "unknown");
            }

//            print_div_td("bound", mini_table(array("min", "max"), array($de->min_bound, $de->max_bound)));
            print_div_td("name", "<a href='./search_errors.php?name=$de->name'>error statistics</a>");

            echo "</tr>\n";

            $i++;
        }
        echo "</table>\n";
    }
}

?>
