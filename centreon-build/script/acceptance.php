<?php

// Load Centreon Build library.
$centreon_build_dir = dirname(__FILE__) . DIRECTORY_SEPARATOR . '..';
require_once($centreon_build_dir . DIRECTORY_SEPARATOR . 'script' . DIRECTORY_SEPARATOR . 'common.php');

// Replace all the elements of a file
function replace_in_file($in, $out, $to_replace) {
  $str = file_get_contents($in);
  foreach ($to_replace as $content) {
    $str = str_replace($content["from"], $content["to"], $str);
  }
  file_put_contents($out, $str);

  return (true);
}

function get_project_files($project_name) {
  global $distrib;
  global $centreon_build_dir;

  $project_files["web"]["dev"] = xpath("$centreon_build_dir/jobs/containers/mon-containers-web-dev.php");
  $project_files["web"]["compose-in"] = xpath("$centreon_build_dir/containers/web/docker-compose.yml.in");
  $project_files["web"]["compose-out"] = "mon-web-dev.yml";
  $project_files["web"]["compose-replace"][0]["from"] = "@WEB_IMAGE@";
  $project_files["web"]["compose-replace"][0]["to"] = "mon-web-dev:$distrib";

  $project_files["lm"]["dev"] = xpath("$centreon_build_dir/jobs/containers/mon-containers-lm-dev.php");
  $project_files["lm"]["compose-in"] = xpath("$centreon_build_dir/containers/middleware/docker-compose-web.yml.in");
  $project_files["lm"]["compose-out"] = "mon-lm-dev.yml";
  $project_files["lm"]["compose-replace"][0]["from"] = "@WEB_IMAGE@";
  $project_files["lm"]["compose-replace"][0]["to"] = "mon-lm-dev:$distrib";
  $project_files["lm"]["compose-replace"][1]["from"] = "@MIDDLEWARE_IMAGE@";
  $project_files["lm"]["compose-replace"][1]["to"] = "ci.int.centreon.com:5000/mon-middleware:$distrib";

  $project_files["ppm"]["dev"] = xpath("$centreon_build_dir/jobs/containers/mon-containers-ppm-dev.php");
  $project_files["ppm"]["compose-in"] = xpath("$centreon_build_dir/containers/middleware/docker-compose-web.yml.in");
  $project_files["ppm"]["compose-out"] = "mon-ppm-dev.yml";
  $project_files["ppm"]["compose-replace"][0]["from"] = "@WEB_IMAGE@";
  $project_files["ppm"]["compose-replace"][0]["to"] = "mon-ppm-dev:$distrib";
  $project_files["ppm"]["compose-replace"][1]["from"] = "@MIDDLEWARE_IMAGE@";
  $project_files["ppm"]["compose-replace"][1]["to"] = "ci.int.centreon.com:5000/mon-middleware:$distrib";

  $project_files["middleware"]["dev"] = xpath("$centreon_build_dir/jobs/containers/mon-containers-middleware-dev.php");
  $project_files["middleware"]["compose-in"] = xpath("$centreon_build_dir/containers/middleware/docker-compose-standalone.yml.in");
  $project_files["middleware"]["compose-out"] = "mon-middleware-dev.yml";
  $project_files["middleware"]["compose-replace"][0]["from"] = "@MIDDLEWARE_IMAGE@";
  $project_files["middleware"]["compose-replace"][0]["to"] = "mon-middleware-dev:$distrib";

  $project_files["ppe"]["dev"] = xpath("$centreon_build_dir/jobs/containers/mon-containers-ppe-dev.php");
  $project_files["ppe"]["compose-in"] = xpath("$centreon_build_dir/containers/web/docker-compose.yml.in");
  $project_files["ppe"]["compose-out"] = "mon-ppe-dev.yml";
  $project_files["ppe"]["compose-replace"][0]["from"] = "@WEB_IMAGE@";
  $project_files["ppe"]["compose-replace"][0]["to"] = "mon-ppe-dev:$distrib";
  $project_files["ppe"]["compose-replace"][1]["from"] = "@MIDDLEWARE_IMAGE@";
  $project_files["ppe"]["compose-replace"][1]["to"] = "ci.int.centreon.com:5000/mon-middleware:$distrib";

  return ($project_files[$project_name]);
}

function call_exit(int $signo)
{
    exit(1);
}

// Set signal handlers.
if (function_exists('pcntl_signal')) {
    pcntl_signal(SIGTERM, 'call_exit');
    pcntl_signal(SIGINT, 'call_exit');
}

// Parse the options.
$opts = getopt("d:gh");
array_shift($argv);
if (isset($opts['h'])) {
    echo "USAGE: acceptance.php [-h] [-g] [-d distrib] [feature1 [feature2 [...] ] ]\n";
    echo "\n";
    echo "  Description:\n";
    echo "    Feature files are optional. By default all of them will be run.\n";
    echo "    Log files and screenshots will be saved in system temporary\n";
    echo "    directory in case of an error in a scenario.\n";
    echo "\n";
    echo "  Arguments:\n";
    echo "    -h  Print this help.\n";
    echo "    -d  Distribution used to run tests. Can be one of centos6 (default) or centos7.\n";
    echo "    -g  Only generate files and images. Do not run tests.\n";
    echo "\n";
    echo "  Prerequisites:\n";
    echo "    - *Docker* (connected to Docker Machine on Windows or MacOS)\n";
    echo "    - *Docker Compose*\n";
    echo "    - *PHP*\n";
    echo "    - *PDO* extension for PHP\n";
    echo "    - *PDO MySQL* extension for PHP\n";
    echo "    - *composer*\n";
    echo "    - the following hosts must be resolved to the corresponding IP addresses:\n";
    echo "        crm.int.centreon.com 10.24.11.73\n";
    echo "        support.centreon.com 10.30.2.62\n";
    return (0);
}
if (isset($opts['d'])) {
    $distrib = $opts['d'];
    array_shift($argv);
    array_shift($argv);
}
else {
    $distrib = 'centos6';
}
if (isset($opts['g'])) {
    $only_generate = TRUE;
    array_shift($argv);
}
else {
    $only_generate = FALSE;
}
$source_dir = realpath('.');

// Load configuration file.
echo "[1/5] Loading configuration file...\n";
require_once(xpath($centreon_build_dir . '/conf/acceptance.conf.php'));
if (!defined('_GITHUB_TOKEN_') || _GITHUB_TOKEN_ == "") {
    echo "Please fill your GitHub token in acceptance.conf.php file.\n";
    return (1);
}

// Load project settings.
echo "[2/5] Loading project settings...\n";
$project = basename($source_dir);
switch ($project) {
case 'centreon-license-manager':
case 'centreon-lm':
    $project = 'lm';
    break ;
case 'centreon-imp-portal-api':
case 'centreon-middleware':
    $project = 'middleware';
    break ;
case 'centreon-export':
case 'centreon-ppe':
    $project = 'ppe';
    break ;
case 'centreon-import':
case 'centreon-ppm':
    $project = 'ppm';
    break ;
case 'centreon':
case 'centreon-web':
    $project = 'web';
    break ;
};
$project_files = get_project_files($project);
if (!isset($project_files)) {
  echo "Project $project not supported: supported projects are 'web', 'lm', 'ppe', 'ppm', 'middleware'.\n";
  return (1);
}

// Replace the compose .yml.in.
echo "[3/5] Preparing for execution...\n";
if (!replace_in_file($project_files["compose-in"], xpath($source_dir . "/" . $project_files["compose-out"]), $project_files["compose-replace"])) {
    echo "Couldn't replace in the file " . $project_files["compose"] . "\n";
    return (1);
}

// Execute the dev container script.
echo "[4/5] Building development container from current sources...\n";
passthru('php ' . $project_files["dev"] . " " . $distrib, $return_var);
if ($return_var != 0) {
    echo 'Could not build development container of ' . $project . "\n";
    return (1);
}

// Start acceptance tests.
echo "[5/5] Finally running acceptance tests...\n";
if (!$only_generate) {
    $cmd = xpath("./vendor/bin/behat --strict");
    if (empty($argv)) {
        $argv[] = '';
    }
    foreach ($argv as $feature) {
        passthru($cmd . ' ' . $feature, $return_var);
    }
}

?>
