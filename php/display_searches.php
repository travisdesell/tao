<?php

require_once("tao_php_common.inc");
require_once("asynchronous_newton_method.inc");
require_once("differential_evolution.inc");
require_once("particle_swarm.inc");

require_once("db.inc");

$host = '127.0.0.1';

$con = mysql_connect($host, $user, $pass);
mysql_select_db($db, $con);

echo "
<!DOCTYPE html PUBLIC '-//W3C//DTD HTML 4.01 Transitional//EN' 'http://www.w3.org/TR/html4/loose.dtd'>
<html>
    <head>
        <meta charset='utf-8'>
        <title>TAO Parameter Optimization Progress</title>
        <link rel='stylesheet' type='text/css' href='style.css' />

        <!-- Le styles -->
        <link href='bootstrap/docs/assets/css/bootstrap.css' rel='stylesheet'>
        <link href='bootstrap/docs/assets/css/bootstrap-responsive.css' rel='stylesheet'>

        <!-- HTML5 shim, for IE6-8 support of HTML5 elements -->
        <!--[if lt IE 9]>
        <script src='http://html5shim.googlecode.com/svn/trunk/html5.js'></script>
        <![endif]-->

        <!-- Fav and touch icons -->
        <link rel='apple-touch-icon-precomposed' sizes='144x144' href='bootstrap/docs/assets/ico/apple-touch-icon-144-precomposed.png'>
        <link rel='apple-touch-icon-precomposed' sizes='114x114' href='bootstrap/docs/assets/ico/apple-touch-icon-114-precomposed.png'>
        <link rel='apple-touch-icon-precomposed' sizes='72x72' href='bootstrap/docs/assets/ico/apple-touch-icon-72-precomposed.png'>
        <link rel='apple-touch-icon-precomposed' href='bootstrap/docs/assets/ico/apple-touch-icon-57-precomposed.png'>

        <!-- Le javascript
        ================================================== -->
        <!-- Placed at the end of the document so the pages load faster -->
        <script src='bootstrap/docs/assets/js/jquery.js'></script>
        <script src='bootstrap/docs/assets/js/bootstrap-transition.js'></script>
        <script src='bootstrap/docs/assets/js/bootstrap-alert.js'></script>
        <script src='bootstrap/docs/assets/js/bootstrap-modal.js'></script>
        <script src='bootstrap/docs/assets/js/bootstrap-dropdown.js'></script>
        <script src='bootstrap/docs/assets/js/bootstrap-scrollspy.js'></script>
        <script src='bootstrap/docs/assets/js/bootstrap-tab.js'></script>
        <script src='bootstrap/docs/assets/js/bootstrap-tooltip.js'></script>
        <script src='bootstrap/docs/assets/js/bootstrap-popover.js'></script>
        <script src='bootstrap/docs/assets/js/bootstrap-button.js'></script>
        <script src='bootstrap/docs/assets/js/bootstrap-collapse.js'></script>
        <script src='bootstrap/docs/assets/js/bootstrap-carousel.js'></script>
        <script src='bootstrap/docs/assets/js/bootstrap-typeahead.js'></script>

    </head>
";

echo "<body>";
echo "<form action='display_plots.php' method='get'>\n";



echo "
<div class='container'>
<ul class='nav nav-pills'>
    <li><a class='accordion-toggle' data-toggle='collapse' data-parent='#accordion2' href='#active_de'>Active DE</a></li>
    <li><a class='accordion-toggle' data-toggle='collapse' data-parent='#accordion2' href='#inactive_de'>Inactive DE</a></li>
    <li><a class='accordion-toggle' data-toggle='collapse' data-parent='#accordion2' href='#active_pso'>Active PSO</a></li>
    <li><a class='accordion-toggle' data-toggle='collapse' data-parent='#accordion2' href='#inactive_pso'>Inactive PSO</a></li>
    <li><a class='accordion-toggle' data-toggle='collapse' data-parent='#accordion2' href='#active_anm'>Active ANM</a></li>
    <li><a class='accordion-toggle' data-toggle='collapse' data-parent='#accordion2' href='#inactive_anm'>Inactive ANM</a></li>
    </ul>
</div>";


echo "<div class='container'>";
echo "<div class='accordion-group'>";

function print_search_row($href, $name, $print_de_table, $active_de) {

    echo "
            <div class='accordion-heading'>
                <a class='accordion-toggle' data-toggle='collapse' data-parent='#accordion2' href='#$href'>
                    <h3>$name</h3>
                </a>
            </div>

            <div id='$href' class='accordion-body collapse'>
                <div class='accordion-inner'>";
            
$print_de_table($active_de);

    echo"
                </div>
            </div>";

}

$active_de = get_active_de($con);
print_search_row("active_de", "Active Asynchronous Differential Evolution", print_de_table, $active_de);

$inactive_de = get_inactive_de($con);
print_search_row("inactive_de", "Inactive Asynchronous Differential Evolution", print_de_table, $inactive_de);

$active_pso = get_active_pso($con);
print_search_row("active_pso", "Active Particle Swarm Optimization", print_pso_table, $active_pso);

$inactive_pso = get_inactive_pso($con);
print_search_row("inactive_pso", "Inactive Particle Swarm Optimization", print_pso_table, $inactive_pso);

$active_anm = get_active_anm($con);
print_search_row("active_anm", "Active Asnychronous Newton Method Searches", print_anm_table, $active_anm);

$inactive_anm = get_inactive_anm($con);
print_search_row("inactive_anm", "Inactive Asnychronous Newton Method Searches", print_anm_table, $inactive_anm);

echo "<div class='accordion-group'>";
echo "</div>";  //container well


echo "
<div class='container'>
  <div class='row'>
      <div class='span2'>
          <input type='submit' name='generate_plots' value='Generate Plots' id='generate_plots'>
      </div>

      <div class='span10'>
          <div class='rowfluid'>
              <div class='span3'>
                  <p align='center'>first evaluation</p>
                  <input type='text' size=20 maxlength=20 name='first_evaluation' value = ''>
              </div>
              <div class='span3'>
                  <p align='center'>last evaluation</p>
                  <input type='text' size=20 maxlength=20 name='last_evaluation' value = ''>
              </div>
              <div class='span3'>
                  <p align='center'>limit (default 1000)</p>
                  <input type='text' size=20 maxlength=20 name='limit' value = ''>
              </div>
          </div>

          <div class='rowfluid'>
              <div class='span3'>
                  <p align='center'>y-axis minimum</p>
                  <input type='text' size=20 maxlength=20 name='y_min' value = ''>
              </div>
              <div class='span3'>
                  <p align='center'>y-axis maximum</p>
                  <input type='text' size=20 maxlength=20 name='y_max' value = ''>
              </div>
              <div class='span3'>
                  <p align='center'>select every nth value</p>
                  <input type='text' size=20 maxlength=20 name='nth_value' value = ''>
              </div>
          </div>
      </div>
</div>
</div>
</table>\n";

echo "</div>";  //container

echo "</form>\n";

echo "</body>\n";

echo "</html>";
?>
