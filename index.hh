<?hh

require "config.hh";
require "GenerativeApiArt.hh";

if ($_GET['q'] === "bitmap"){
  $mode = GenerativeApiArt::MODE_BITMAP;
} else {
  $mode = GenerativeApiArt::MODE_COLOR;
}

$test = new GenerativeApiArt(Config::KEY, 500);
$test->call()->extract()->create($mode);

$test->output();