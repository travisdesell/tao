<?php

function print_div_td($dc, $value) {
    echo "<td><div class='" . $dc . "'>" . $value . "</div></td>";
}

function mini_table($names, $values) {
    $bound_table = "";

    $bound_table = "<table class='bounds_table'>";

    $n = count($names);
    for ($i = 0; $i < $n; $i++) {
        $bound_table .= "<tr>";
        $bound_table .= "<td text-align:left>" . $names[$i] . ":</td>";

        $current = $values[$i];
        $nj = count($current);

        $v = explode(",", substr($current, 1, -1));
        foreach ($v as $vi) {
            if (($vi - floor($vi)) > 0) {
                $bound_table .= "<td>" . number_format((float)$vi, 3, '.', '') . "</td>";
            } else {
                $bound_table .= "<td>" . $vi . "</td>";
            }
        }
        $bound_table .= "</tr>";
    }
    $bound_table .= "</tr></table>";

//    echo "\n\nnames: " . json_encode($names) . "\n";
//    echo "\n\nvalues; " . json_encode($values) . "\n";
//    echo "\n\nBOUNDS TABLE:\n" . $bound_table . "\n\n";

    return $bound_table;
}

?>
